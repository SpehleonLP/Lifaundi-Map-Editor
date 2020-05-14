#include "metaroom.h"
#include "lf_math.h"
#include "quadtree.h"
#include "document.h"
#include "glviewwidget.h"
#include "Shaders/roomoutlineshader.h"
#include "Shaders/selectedroomshader.h"
#include "Shaders/arrowshader.h"
#include "clipboard.h"
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

template<typename T>
static std::unique_ptr<T[]> Realloc(std::unique_ptr<T[]> const& array, uint32_t size, uint32_t new_size, int mul)
{
	std::unique_ptr<T[]> r(new T[new_size*mul]);
	memcpy(&r[0], &array[0], mul * size * sizeof(T));
	return r;
}

template<typename T>
static void MemMove(std::unique_ptr<T[]> & array, uint32_t i, uint32_t size, int mul)
{
	memmove(&array[mul*i], &array[mul*(i+1)], mul * (size-(i+1)) * sizeof(T));
}

template<typename T>
static void RemoveStuff(std::unique_ptr<T[]> & array, uint32_t size, std::vector<int> const& vec, int mul)
{
	uint32_t read = 0, write = 0;

	for(int i : vec)
	{
		assert(read < size);

		uint32_t copy_length = i - read;

		if(copy_length == 0)
		{
			++read;
			continue;
		}

		memmove(&array[mul*write], &array[mul*read], mul * copy_length * sizeof(T));

		read  += copy_length+1;
		write += copy_length;
	}

	assert((read - write) == vec.size());

	uint32_t copy_length = size - read;

	if(copy_length > 0)
	{
		memmove(&array[mul*write], &array[mul*read], mul * copy_length * sizeof(T));
	}

}

template<typename T>
static void ExpandStuff(std::unique_ptr<T[]> & array, int size, std::vector<int> const& vec, int mul)
{
	for(int i = vec.size()-1; i >= 0; --i)
	{
		int src = vec[i] - i;
		int dst = vec[i] + 1;

		if(size - dst > 0)
		{
			memmove(&array[mul*dst], &array[mul*src], mul * (size - dst) * sizeof(T));
		}

		size = dst;
	}
}

Metaroom::Metaroom(Document * document) :
	document(document)
{
	RoomOutlineShader::Shader.AddRef();
	SelectedRoomShader::Shader.AddRef();
	ArrowShader::Shader.AddRef();
}

Metaroom::~Metaroom()
{
}

void Metaroom::Release(GLViewWidget *gl)
{
	gl->glAssert();
    m_indices.Release(gl);
    m_selection.Release(gl);
    RoomOutlineShader::Shader.Release(gl);
    SelectedRoomShader::Shader.Release(gl);
    ArrowShader::Shader.Release(gl);

    gl->glDeleteVertexArrays(3, m_vao);
    gl->glDeleteBuffers(3, m_buffers);
}
void Metaroom::Read(MainWindow * window, std::ifstream & fp, size_t offset)
{
	char buffer[4];
	short version;
	short no_tracks;
	uint16_t width{};
	uint16_t height{};
    uint32_t  no_faces;
	fp.seekg(offset, std::ios::beg);

	fp.read(&buffer[0], 4);

	fp.read((char*)&version, 2);
	fp.read((char*)&no_tracks, 2);
	fp.read((char*)&no_faces, 4);

	if(version == 2)
	{
		fp.read((char*)&width, 2);
		fp.read((char*)&height, 2);
	}
	if(version >= 3)
	{
		fp.seekg(8, std::ios::cur);
	}

	if(memcmp(buffer, "lfmp", 4) != 0)
		throw std::runtime_error("Bad File");

	AddFaces(no_faces);

	if(size() == 0)
		return;

	assert(faces == no_faces);

	std::vector<glm::i16vec2> vec(size()*4);

	fp.read((char*)&vec[0], sizeof(vec[0]) * vec.size());

	if(version <= 2)
	{
        for(uint32_t i = 0; i <	vec.size(); ++i)
		{
			vec[i] = glm::ivec2((glm::u16vec2&)vec[i]) - glm::ivec2(width, height)/2;
		}
	}


	fp.read((char*)&m_gravity[0],  4* size());
	fp.read((char*)&m_roomType[0], 1* size());
	fp.read((char*)&m_doorType[0], 4* size());
	fp.read((char*)&m_music[0],    1* size());

#if HAVE_UUID
	for(uint i = 0; i < size(); ++i)
		m_uuid[i] = m_uuid_counter + i;
	m_uuid_counter += size();
#endif

	if(version <= 3)
	{
		fp.seekg(8 * no_tracks, std::ios::cur);
	}
	else
	{
		std::vector<std::string> tracks;
		tracks.resize(no_tracks);

		for(uint32_t i = 0; i < tracks.size(); ++i)
		{
			uint8_t length;
			fp.read((char*)&length, 1);
			tracks[i].resize(length);
			fp.read(&tracks[i][0], length);
		}

		window->SetTrackList(tracks);
	}

	RestoreVerts(vec);
	m_selection.clear();

	std::vector<QuadTree::DoorList> door_indices;
	std::vector<QuadTree::Door>     door_list;
//read permeability info
	door_indices.resize(size()*4);
	fp.read((char*)&door_indices[0], sizeof(QuadTree::DoorList) * door_indices.size());

	door_list.resize(door_indices.back().index + door_indices.back().length);
	fp.read((char*)&door_list[0], sizeof(door_list[0]) * door_list.size());

	for(auto & list : door_indices)
	{
		int  index = (&list - &door_indices[0]) / 4;
		auto begin = &door_list[list.index];
		auto end   = begin + list.length;

		for(auto i = begin; i < end; ++i)
		{
			if(0 <= i->perm && i->perm < 100 && index < i->face)
			{
				m_permeabilities.push_back(std::make_pair<uint64_t, uint8_t>
					(GetDoorKey(index, i->face), i->perm));
			}
		}
	}

	if(version > 4)
		m_tree.ReadTree(fp);
}

uint32_t Metaroom::Write(MainWindow * window, std::ofstream & fp)
{
	uint32_t offset = fp.tellp();

	auto tracks = window->GetTrackList(&m_music[0], size());

	const char * title = "lfmp";

	short version = 4;
	short no_tracks = tracks.size();

	fp.write(title, 4);
	fp.write((char*)&version, 2);
	fp.write((char*)&no_tracks, 2);
	fp.write((char*)&faces, 4);

	auto bounds = GetBoundingBox();

	fp.write((char*)&bounds, 2 * 4);

	auto verts = SaveVerts();

	fp.write((char*)&verts[0], sizeof(verts[0]) * verts.size());
	fp.write((char*)&m_gravity[0],  4 * size());
	fp.write((char*)&m_roomType[0], 1 * size());
	fp.write((char*)&m_doorType[0], 4 * size());
	fp.write((char*)&m_music[0],    1 * size());

	for(uint32_t i = 0; i < tracks.size();++i)
	{
		uint8_t length = tracks[i].size();
		fp.write((char*)&length, 1);
		fp.write((char*)&tracks[i][0], length);
	}

	std::vector<QuadTree::DoorList> door_indices;
	std::vector<QuadTree::Door>     door_list;

    m_tree.GetWriteDoors(door_list, door_indices);

	assert(door_indices.size() == size()*4);

	fp.write((char*)&door_indices[0], sizeof(QuadTree::DoorList) * door_indices.size());
	fp.write((char*)&door_list[0], sizeof(QuadTree::Door) * door_list.size());

	m_tree.WriteTree(fp);

	return offset;
}

int Metaroom::Insert(std::vector<Room> const& list)
{
	int first = AddFaces(list.size());

	for(size_t i = 0; i < list.size(); ++i)
		SetRoom(first+i, list[i]);

	m_selection.clear();

	for(uint32_t i = first; i < faces; ++i)
	{
		m_selection.select_face(i, Bitwise::SET);
	}

	memcpy(&m_prev_verts[first*4], &m_verts[first*4], sizeof(glm::ivec2) * list.size()*4);
    m_tree.SetDirty();
	m_dirty = true;

	return first;
}

void Metaroom::SetRoom(int index, Room const& room)
{
	m_roomType[index] = room.type;
	m_gravity[index]  = room.gravity;
	m_music[index]    = room.music_track;

	for(int j = 0; j < 4; ++j)
		m_doorType[index*4+j] = room.wall_types[j];

	for(int j = 0; j < 4; ++j)
		m_verts[index*4+j] = room.verts[j];
}


void  Metaroom::CopyRoom(int dst, int src)
{
	if(dst == src) return;

	m_roomType[dst] = m_roomType[src];
	m_gravity[dst]  = m_gravity[src];
	m_music[dst]    = m_music[src];

	for(int j = 0; j < 4; ++j)
		m_doorType[dst*4 + j] = m_doorType[src*4 + j];

	for(int j = 0; j < 4; ++j)
		m_verts[dst*4 + j] = m_verts[src*4 + j];
}

void Metaroom::Insert(std::vector<Room> const& list, std::vector<int> const& indices)
{
	Expand(indices);

    for(uint32_t i = 0; i < list.size(); ++i)
		SetRoom(indices[i], list[i]);

	m_selection.clear();

	for(int i : indices)
		m_selection.select_face(i, Bitwise::SET);

	CommitMove();
}



glm::i16vec4 Metaroom::GetBoundingBox() const
{
	if(faces == 0)
		return glm::i16vec4(0, 0, 0, 0);

	glm::ivec2 min{m_verts[0]};
	glm::ivec2 max{m_verts[0]};

	for(uint32_t i = 1; i < faces*4; ++i)
	{
		min = glm::min(min, m_verts[i]);
		max = glm::max(max, m_verts[i]);
	}

	return glm::i16vec4(min, max);
}

int Metaroom::Slice(std::vector<SliceInfo> & slice)
{
	std::vector<int> face_list;
	face_list.reserve(slice.size()/2);

	for(size_t i = 0; i < slice.size(); i += 2)
		face_list.push_back(slice[i].edge / 4);

	int first = Duplicate(face_list);

	for(size_t i = 0; i < slice.size(); i += 2)
	{
		int j = first + i/2;

		int e = slice[i].edge % 4;
		m_verts[j*4 + e]         = slice[i  ].vertex;
		m_verts[j*4 + (e+3) % 4] = slice[i+1].vertex;

		m_doorType[j*4 + e]      = 0;

		if(slice[i].uuid < 0)
			slice[i].uuid = ++m_uuid_counter;

		m_uuid[j] = slice[i].uuid;
	}

	for(size_t i = 0; i < slice.size(); i += 2)
	{
		std::swap(m_verts[NextInEdge(slice[i].edge)], slice[i  ].vertex);
		std::swap(m_verts[slice[i+1].edge], slice[i+1].vertex);
		std::swap(m_doorType[Metaroom::NextInEdge(slice[i].edge)], slice[i].type);
	}


	return first;
}

uint32_t Metaroom::GetGravity() const
{
	uint32_t g_id = 0;
	bool     match = false;

	for(uint32_t i = 0; i < size(); ++i)
	{
		if(m_selection.IsFaceSelected(i))
		{
			if(match == false)
			{
				g_id = m_gravity[i];
				match = true;
			}
			else if(g_id != m_gravity[i])
			{
				g_id  = 0;
				match = false;
				break;
			}
		}
	}

	return g_id;
}


glm::vec2 Metaroom::GetGravity(int room) const
{
    if(room < 0 || (uint32_t)room >= faces)
		throw std::runtime_error("tried to get gravity of room which doesn't exist");

	return glm::unpackHalf2x16(m_gravity[room]);
}

glm::vec2 Metaroom::GetCenter(int room) const
{
    if(room < 0 || (uint32_t)room >= faces)
		throw std::runtime_error("tried to get center of room which doesn't exist");

	return glm::vec2(
			m_verts[room*4 + 0] +
			m_verts[room*4 + 1] +
			m_verts[room*4 + 2] +
			m_verts[room*4 + 3]) / 4.f;
}

int Metaroom::GetMusicTrack() const
{
	bool     match = false;
	int      track = 0;

	for(uint32_t i = 0; i < size(); ++i)
	{
		if(m_selection.IsFaceSelected(i))
		{
			if(match == true && track != m_music[i])
				return false;

			track = m_music[i];
			match = true;
		}
	}

	return track;
}

int Metaroom::GetRoomType() const
{
	bool match = false;
	int   r_type = -1;

	for(uint32_t i = 0; i < size(); ++i)
	{
		if(m_selection.IsFaceSelected(i))
		{
			if(match == true && r_type != m_roomType[i])
				return -1;

			r_type = m_roomType[i];
			match = true;
		}
	}

	return r_type;
}

int Metaroom::GetWallType() const
{
	bool match = false;
	int   r_type = -1;

	for(uint32_t i = 0; i < size()*4; ++i)
	{
		if(m_selection.IsEdgeSelected(i))
		{
			if(match == true && r_type != m_doorType[i])
				return -1;

			r_type = m_doorType[i];
			match = true;
		}
	}

	return r_type;
}


int Metaroom::GetDoorType()
{
	bool     match = false;
	int   w_type = 0;

	for(uint32_t i = 0; i < size()*4; ++i)
	{
		if(m_selection.IsEdgeSelected(i))
		{
			if(match == true && w_type != m_doorType[i])
				return -1;

			w_type = m_doorType[i];
			match = true;
		}
	}

	return w_type;
}

uint64_t Metaroom::GetDoorKey(uint32_t a, uint32_t b) const
{
	a = m_uuid[a];
	b = m_uuid[b];

	return a < b? ((uint64_t)a << 32) | b : ((uint64_t)b << 32) | a;
}

int Metaroom::GetPermeability(int a, int b) const
{
	if(a < 0 || b < 0)
		return -1;

	uint64_t key = GetDoorKey(a, b);

	for(uint32_t i = 0; i < m_permeabilities.size(); ++i)
	{
		if(key == m_permeabilities[i].first)
			return m_permeabilities[i].second;
		else if(key < m_permeabilities[i].first)
			break;
	}

	return 100;
}


int Metaroom::GetPermeability() const
{
	auto doors = GetSelectedDoors();

	if(doors.empty())
		return -1;

	bool   match = false;
	int   w_perm = 100;

	uint64_t door = GetDoorKey(doors[0].first, doors[0].second);

	uint32_t i{}, j{};
	while(i < doors.size() && j < m_permeabilities.size())
	{
		if(m_permeabilities[j].first > door)
		{
			if(++i < doors.size())
				door = GetDoorKey(doors[i].first, doors[i].second);
		}
		else if(m_permeabilities[j].first < door)
		{
			++j;
		}
		else
		{
			if(match == true && w_perm != m_permeabilities[j].second)
				return -1;

			w_perm = m_permeabilities[j].second;
			match  = true;
			++i, ++j;
		}
	}

	return w_perm;
}


std::vector<std::pair<int, int>> Metaroom::GetSelectedDoors() const
{
	std::vector<std::pair<int, int>> r;

	for(uint32_t i = 0; i < faces*4; ++i)
	{
		if(!m_selection.IsEdgeSelected(i))
			continue;

		auto edges = m_tree.GetOverlappingEdges(GetVertex(i), GetNextVertex(i));

		std::sort(edges.begin(), edges.end());

		for(auto j : edges)
		{
			if(m_selection.IsEdgeSelected(j) &&	i < j)
				r.push_back({i/4, j/4});
		}
	}

	return r;
}

std::vector<int> Metaroom::GetExtrudableEdges() const
{
	std::vector<int> r;

	for(uint32_t i = 0; i < faces*4; ++i)
	{
		if( m_selection.IsEdgeSelected(i)
		&& !m_tree.HasOverlappingEdges(i))
			r.push_back(i);
	}

	return r;
}

int Metaroom::Duplicate(std::vector<int> const& faces)
{
	int first = AddFaces(faces.size());

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy(&m_verts[(first+i)*4],      &m_verts[faces[i]*4],       sizeof(m_verts[0])*4);

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy(&m_gravity[(first+i)],      &m_gravity[faces[i]],       sizeof(m_gravity[0]));

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy(&m_music[(first+i)],        &m_music[faces[i]],         sizeof(m_music[0]));

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy(&m_roomType[(first+i)],     &m_roomType[faces[i]],      sizeof(m_roomType[0]));

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy(&m_doorType[(first+i)*4],   &m_doorType[faces[i]*4],    sizeof(m_doorType[0])*4);


	return first;
}

void Metaroom::Expand(std::vector<int> const& indices)
{
	AddFaces(indices.size());

	ExpandStuff(m_verts,    faces, indices, 4);
	ExpandStuff(m_gravity,  faces, indices, 1);
	ExpandStuff(m_music,    faces, indices, 1);
	ExpandStuff(m_roomType, faces, indices, 1);
	ExpandStuff(m_doorType, faces, indices, 4);
#if HAVE_UUID
	ExpandStuff(m_uuid    , faces, indices, 1);
#endif
}

int        Metaroom::GetSliceEdge(glm::ivec2 p) const
{
    int face = m_tree.GetFace(p);

	if(face == -1)
		return -1;

	int best_edge      = -1;
	float min_distance = FLT_MAX;

	for(int i = 0; i < 4; ++i)
	{
		float distance2 = math::LineDistanceSquared(m_verts[face*4+i], m_verts[face*4+(i+1)%4], p);

		if(distance2 < min_distance)
		{
			best_edge    = i;
			min_distance = distance2;
		}
	}

	if(best_edge >= 0)
		return face*4 + best_edge;

	return -1;
}

glm::ivec2 Metaroom::GetPointOnEdge(int v0, float p) const
{
	glm::vec2 vec(m_verts[NextInEdge(v0)] - m_verts[v0]);

	 glm::ivec2 offset = glm::vec2(
		vec.x < 0? ceil(vec.x * p) : floor(vec.x * p),
		vec.y < 0? ceil(vec.y * p) : floor(vec.y * p));

	if((v0 & 0x03) < 2)
		return m_verts[NextInEdge(v0)] - offset;

	return offset + m_verts[v0];
}


float     Metaroom::GetPercentOfEdge(int v0, glm::ivec2 pos) const
{
	glm::vec2 a(m_verts[NextInEdge(v0)] - m_verts[v0]);
	glm::vec2 b(pos - m_verts[v0]);

	float value = std::max(0.f, std::min(1.f, glm::length(b) / glm::length(a)));

	if((v0 & 0x03) < 2)
		return 1.f - value;

	return value;
}

float     Metaroom::ProjectOntoEdge(int v0, glm::ivec2 pos) const
{
	glm::vec2 a(pos - m_verts[v0]);
	glm::vec2 b(m_verts[NextInEdge(v0)] - m_verts[v0]);
	float length = math::length2(b);

	float value = glm::dot(a, b) / length;
	value = std::max(0.f, std::min(1.f, value));

	if((v0 & 0x03) < 2)
		return 1.f - value;

	return value;
}

void Metaroom::Prepare(GLViewWidget* gl)
{
    m_selection.Prepare(gl);

	if(m_dirty == false || size() == 0)
		return;

	m_dirty = false;
	bool initialized_vaos = (m_vao[0] != 0L);

    m_indices.Prepare(gl, allocated);

	if(initialized_vaos == false)
	{
        gl->glGenVertexArrays(3, m_vao);
        gl->glGenBuffers(3, m_buffers);

        gl->glBindVertexArray(m_vao[0]);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(glm::ivec2), 0);
        gl->glEnableVertexAttribArray(0);

        gl->glBindBuffer(GL_ARRAY_BUFFER, m_selection.GetBuffer());
        gl->glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), 0);
        gl->glEnableVertexAttribArray(1);

        gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices.EdgeVBO());

// create selected face shader
        gl->glBindVertexArray(m_vao[1]);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(glm::ivec2), 0);
        gl->glEnableVertexAttribArray(0);

        gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_selection.GetIndices());

        gl->glBindVertexArray(m_vao[2]);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
        gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(ArrowShader::Vertex), (void*) offsetof(ArrowShader::Vertex, position));
        gl->glVertexAttribPointer(1, 2, GL_SHORT, GL_TRUE, sizeof(ArrowShader::Vertex), (void*) offsetof(ArrowShader::Vertex, rotation));

        gl->glEnableVertexAttribArray(0);
        gl->glEnableVertexAttribArray(1);
	}

    gl->glBindVertexArray(0);
    gl->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);

    gl->glAssert();
	if(m_glAlloced < allocated)
	{
		m_glAlloced = allocated;
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::ivec2)*allocated*4, &m_verts[0], GL_DYNAMIC_DRAW);
	}
	else if(size() > 0)
	{
        gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::ivec2)*allocated*4, &m_verts[0]);
	}
    gl->glAssert();

//prepare hall transitions
	std::vector<ArrowShader::Vertex> halls;
	halls.reserve(faces);


	ArrowShader::Vertex buffer;
	for(uint32_t i = 1; i < faces; ++i)
	{
		for(int j = 0; j < 16; ++j)
		{
			const int a = (j % 4);
			const int b = (j / 4);

			const auto a0 = GetVertex(i-1, a);
			const auto a1 = GetVertex(i-1, a+1);
			const auto b0 = GetVertex(i, b);
			const auto b1 = GetVertex(i, b+1);

			if(a0 == a1
			|| b0 == b1
			|| GetDoorType(i-1, a) != 0
			|| GetDoorType(i  , b) != 0
			|| math::IsOpposite(a1 - a0, b1 - b0) == false
			|| math::IsColinear(a0, a1, b0, b1) == false)
				continue;

			std::pair<float, int> begin, end;

			if(math::GetOverlap(a0, a1-a0, b0, b1, &begin, &end))
			{
				float avg = (begin.first + end.first) * .5f;

				glm::vec2 vec = a1 - a0;

				buffer.position = a0 + glm::ivec2(vec * avg);

				vec = glm::normalize(vec);
				buffer.rotation.x = SHRT_MAX * vec.x;
				buffer.rotation.y = SHRT_MAX * vec.y;

				halls.push_back(buffer);
			}
		}
	}

    gl->glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
    gl->glAssert();

	if(m_hallAlloc < halls.capacity())
	{
		m_hallAlloc = halls.capacity();
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(halls[0])*halls.capacity(), &halls[0], GL_DYNAMIC_DRAW);
	}
	else if(halls.size() > 0)
	{
        gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(halls[0])*halls.size(), &halls[0]);
	}

    m_noHalls = halls.size();
    gl->glAssert();

}

void Metaroom::Render(GLViewWidget *gl, int selected_door_type)
{
	if(faces == 0)
		return;

    Prepare(gl);

	if(m_selection.NoSelectedFaces() > 0)
	{
        gl->glBindVertexArray(m_vao[1]);
        SelectedRoomShader::Shader.Render(gl, m_selection.NoSelectedFaces());
	}

    gl->glBindVertexArray(m_vao[0]);
    gl->glLineWidth(3);
    RoomOutlineShader::Shader.Render(gl, faces, true);
    gl->glLineWidth(1);
    RoomOutlineShader::Shader.Render(gl, faces, false);

    m_tree.Render(gl, selected_door_type);

	if(m_noHalls)
	{
        gl->glBindVertexArray(m_vao[2]);
        ArrowShader::Shader.Render(gl, m_noHalls);
	}
}

void Metaroom::AddFace(glm::ivec2 min, glm::ivec2 max)
{
    if(m_tree.DoesOverlap(min, max))
		return;

	int face = AddFaces();

	m_verts[face*4+0] = max;
	m_verts[face*4+1] = glm::ivec2(max.x, min.y);
	m_verts[face*4+2] = min;
	m_verts[face*4+3] = glm::ivec2(min.x, max.y);

	m_gravity[face]   = glm::packHalf2x16(glm::vec2(0, 9.81));
	m_music[face]     = -1;
	m_roomType[face]  = 0;
#if HAVE_UUID
	m_uuid[face]      = m_uuid_counter++;
#endif

	memset(&m_doorType[face*4], 0, 4);

	memcpy(&m_prev_verts[face*4], &m_verts[face*4], sizeof(m_verts[0]) * 4);

	m_selection.select_face(face, Bitwise::SET);

	m_dirty = true;
}

int Metaroom::AddFaces(int no_faces)
{
	m_dirty = true;
    m_tree.SetDirty();

	if(faces+no_faces >= allocated)
	{
		auto new_size = ((faces+no_faces) + 15) & 0xFFFFFFF0;

		m_verts          = Realloc(m_verts,          allocated, new_size, 4);
		m_prev_verts     = Realloc(m_prev_verts,     allocated, new_size, 4);
		m_scratch_verts  = std::unique_ptr<glm::ivec2[]>(new glm::ivec2[new_size*4]);

		m_gravity        = Realloc(m_gravity,        allocated, new_size, 1);
		m_music          = Realloc(m_music,          allocated, new_size, 1);
		m_roomType       = Realloc(m_roomType,       allocated, new_size, 1);
		m_doorType       = Realloc(m_doorType,       allocated, new_size, 4);
#if HAVE_UUID
		m_uuid           = Realloc(m_uuid    ,       allocated, new_size, 1);
#endif

		m_selection.resize(new_size);
		allocated = new_size;
	}

	int new_face = faces;
	faces += no_faces;
	return new_face;
}

void Metaroom::RemoveFace(int id)
{
	m_dirty = true;

	MemMove(m_verts,          id, allocated, 4);
	MemMove(m_prev_verts,     id, allocated, 4);
	MemMove(m_music,          id, allocated, 1);
	MemMove(m_gravity,        id, allocated, 1);
	MemMove(m_roomType,       id, allocated, 1);
	MemMove(m_doorType,       id, allocated, 4);

#if HAVE_UUID
	MemMove(m_uuid    ,       id, allocated, 1);
#endif

	m_selection.erase(id);

	--faces;

    m_tree.SetDirty();
}

void Metaroom::RemoveFaces(std::vector<int> const& vec)
{
	if(vec.empty())
		return;

	if(vec.size() == 1)
	{
		RemoveFace(vec[0]);
		return;
	}

	m_dirty = true;

	RemoveStuff(m_verts,          allocated, vec, 4);
	RemoveStuff(m_prev_verts,     allocated, vec, 4);
	RemoveStuff(m_music,          allocated, vec, 1);
	RemoveStuff(m_gravity,        allocated, vec, 1);
	RemoveStuff(m_roomType,       allocated, vec, 1);
	RemoveStuff(m_doorType,       allocated, vec, 4);
#if HAVE_UUID
	RemoveStuff(m_uuid    ,       allocated, vec, 1);
#endif

	m_selection.clear();

	faces -= vec.size();

    m_tree.SetDirty();
}

void Metaroom::CancelMove()
{
	memcpy(&m_verts[0], &m_prev_verts[0], sizeof(glm::ivec2) * faces*4);
	m_dirty = true;
}

void Metaroom::CommitMove(bool update_mvsf)
{
	memcpy(&m_prev_verts[0], &m_verts[0], sizeof(glm::ivec2) * faces*4);
    m_tree.SetDirty(update_mvsf);
	m_dirty = true;
}

void Metaroom::Translate(glm::ivec2 translation, glm::ivec2 half_dimensions)
{
	m_dirty = true;

	for(uint32_t i = 0; i < faces*4; ++i)
	{
		m_scratch_verts[i] = m_prev_verts[i] + translation * (int) m_selection.IsVertSelected(i);
		m_scratch_verts[i] = glm::max(-half_dimensions, glm::min(m_scratch_verts[i], half_dimensions));
	}

	solve_constraints();
	std::swap(m_scratch_verts, m_verts);
}

void Metaroom::Rotate(glm::ivec2 center, glm::vec2 complex)
{
	m_dirty = true;

	memcpy(&m_scratch_verts[0], &m_prev_verts[0], sizeof(glm::ivec2) * faces*4);

	for(uint32_t i = 0; i < faces*4; ++i)
	{
		if(m_selection.IsVertSelected(i))
		{
			glm::ivec2 p = m_scratch_verts[i] - center;

			p = glm::ivec2(
					p.x * complex.x - p.y * complex.y,
					p.x * complex.y + p.y * complex.x);

			m_scratch_verts[i] = p + center;
		}
	}

	solve_constraints();
	std::swap(m_scratch_verts, m_verts);
}

void Metaroom::Scale(glm::ivec2 center, glm::vec2 scale)
{
	m_dirty = true;

	memcpy(&m_scratch_verts[0], &m_prev_verts[0], sizeof(glm::ivec2) * faces*4);

	for(uint32_t i = 0; i < faces*4; ++i)
	{
		if(m_selection.IsVertSelected(i))
			m_scratch_verts[i] = glm::ivec2(glm::vec2(m_scratch_verts[i] - center) * scale) + center;
	}

	solve_constraints();
	std::swap(m_scratch_verts, m_verts);
}

std::vector<glm::i16vec2> Metaroom::SaveVerts()
{
	std::vector<glm::i16vec2> r(faces*4);

	for(uint32_t i = 0; i < faces*4; ++i)
	{
		r[i] = m_verts[i];
	}

	return r;
}

void Metaroom::RestoreVerts(const std::vector<glm::i16vec2> & vec)
{
	m_dirty = true;

	assert(faces*4 == vec.size());

	for(uint32_t i = 0; i < faces*4; ++i)
	{
		m_verts[i] = vec[i];
	}
}

void Metaroom::GetFaceAABB(int id, glm::i16vec2 & tl, glm::i16vec2 & br) const
{
	tl.x = std::min(
		std::min(m_verts[id*4+0].x, m_verts[id*4+1].x),
		std::min(m_verts[id*4+2].x, m_verts[id*4+3].x));

	tl.y = std::min(
		std::min(m_verts[id*4+0].y, m_verts[id*4+1].y),
		std::min(m_verts[id*4+2].y, m_verts[id*4+3].y));

	br.x = std::max(
		std::max(m_verts[id*4+0].x, m_verts[id*4+1].x),
		std::max(m_verts[id*4+2].x, m_verts[id*4+3].x));

	br.y = std::max(
		std::max(m_verts[id*4+0].y, m_verts[id*4+1].y),
		std::max(m_verts[id*4+2].y, m_verts[id*4+3].y));
}

glm::ivec2 Metaroom::GetSelectionCenter() const
{
	glm::ivec2 accumulator(0, 0);
	int32_t   size{};

	for(uint32_t i = 0; i < faces*4; ++i)
	{
		int is_selected = m_selection.IsVertSelected(i);

		accumulator += m_verts[i] * is_selected;
		size        += is_selected;
	}

	return accumulator / (size + (int) (size == 0));
}

void Metaroom::BoxSelect(glm::ivec2 tl, glm::ivec2 br, Bitwise flags)
{
	if(flags == Bitwise::SET)
		m_selection.clear();
	if(flags == Bitwise::AND)
		m_selection.begin_and();

	for(size_t i = 0; i < faces*4; ++i)
	{
		if(tl.x < m_verts[i].x && m_verts[i].x < br.x
		&& tl.y < m_verts[i].y && m_verts[i].y < br.y)
			m_selection.select_vertex(i, flags);
	}

	if(flags == Bitwise::AND)
		m_selection.end_and();

	update_selections();
	return;
}

void Metaroom::ClickSelect(glm::ivec2 pos, Bitwise flags)
{
	if(flags == Bitwise::SET)
		m_selection.clear();
	if(flags == Bitwise::AND)
		m_selection.begin_and();

	static constexpr float radius = 36;

	bool selected = false;

	for(uint32_t i = 0; i < 4*faces; ++i)
	{
		if(math::length2(m_verts[i] - pos) < radius)
		{
			m_selection.select_vertex(i, flags);
			selected = true;
		}
	}

	if(selected == false)
	{
		for(uint32_t i = 0; i < 4*faces; ++i)
		{
			int j = NextInEdge(i);

			float distance2 = math::SemgentDistanceSquared(m_verts[i], m_verts[j], pos);

			if(0 <= distance2 && distance2 < radius)
			{
				m_selection.select_edge(i, flags);
				selected = true;
			}
		}
	}

	if(selected == false)
	{
		for(uint32_t i = 0; i < faces; ++i)
		{
			if(IsPointContained(i, pos))
			{
				m_selection.select_face(i, flags);
			}
		}
	}

	if(flags == Bitwise::AND)
		m_selection.end_and();

	update_selections();
}

bool Metaroom::IsPointContained(int i, glm::ivec2 pos)
{
	return
	   (math::cross(m_verts[i*4+1] - m_verts[i*4+0], pos - m_verts[i*4+0]) <= 0)
	&& (math::cross(m_verts[i*4+2] - m_verts[i*4+1], pos - m_verts[i*4+1]) <= 0)
	&& (math::cross(m_verts[i*4+3] - m_verts[i*4+2], pos - m_verts[i*4+2]) <= 0)
	&& (math::cross(m_verts[i*4+0] - m_verts[i*4+3], pos - m_verts[i*4+3]) <= 0);
}
