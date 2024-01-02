#include "metaroom.h"
#include "lf_math.h"
#include "quadtree.h"
#include "document.h"
#include "glviewwidget.h"
#include "roomrange.h"
#include "edgerange.h"
#include "color_rooms.h"
#include "clipboard.h"
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <loguru.hpp>

template<typename T>
void * memcpy_s(T * dst, T const* src, size_t items = 1)
{
	return memcpy(dst, src, sizeof(*src)*items);
}

Metaroom::Metaroom(Document * document) :
	document(document)
{
}

Metaroom::~Metaroom()
{
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

	if(_entitySystem.used() == 0)
		return;

	assert(_entitySystem.used() == no_faces);

	std::vector<glm::i16vec2> vec(no_faces*4);

	fp.read((char*)&vec[0], sizeof(vec[0]) * vec.size());

	if(version <= 2)
	{
		for(uint32_t i = 0; i <	vec.size(); ++i)
		{
			vec[i] = glm::ivec2((glm::u16vec2&)vec[i]) - glm::ivec2(width, height)/2;
		}
	}


	fp.read((char*)&_gravity[0],  4* no_faces);
	fp.read((char*)&_roomType[0], 1* no_faces);

	if(version < 5)
	{
		fp.seekg(4 * no_faces, std::ios_base::cur);
	}

	fp.read((char*)&_music[0], 1* no_faces);

	if(version >= 5)
	{
		fp.read((char*)&_directionalShade[0], 4 * no_faces);
		fp.read((char*)&_ambientShade[0], no_faces);
		fp.read((char*)&_audio[0], 4 * no_faces);
	}
	else
	{
		memset(&_directionalShade[0], 0, 4 * no_faces);
		memset(&_ambientShade[0], 0, 4 * no_faces);
		memset(&_audio[0], 0, 4 * no_faces);
	}

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
	_selection.clear();

	std::vector<QuadTree::DoorList> door_indices;
	std::vector<QuadTree::Door>     door_list;
//read permeability info
	door_indices.resize(no_faces*4);
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
			if(0 <= i->perm && i->perm < 100 && index < i->face && i->face < (int)no_faces)
			{
				_permeabilities[GetDoorKey(index, i->face)] = i->perm;
			}
		}
	}

	if(version >= 4)
		_tree.ReadTree(fp);

	PruneDegenerate();
}



uint32_t Metaroom::Write(MainWindow * window, std::ofstream & fp)
{
	std::vector<QuadTree::DoorList> door_indices;
	std::vector<QuadTree::Door>     door_list;

	Metaroom mta{document};

	{
		auto new_list = mta.Insert(Clipboard::Extract(this, _entitySystem.GetList()));

		for(auto i = 0u; i < new_list.size(); ++i)
		{
			if(new_list[i] != i)
				LOG_F(FATAL, "Problem writing metaroom: %s", "Room reindexing failed");
		}
	}

	mta.AddPermeabilities(*this);
	auto no_faces = _entitySystem.used();

	mta._tree.GetWriteDoors(door_list, door_indices);
	assert(door_indices.size() == no_faces*4);
	
	uint32_t offset = fp.tellp();

	auto tracks = window->GetTrackList(&_music[0], range());

	const char * title = "lfmp";

	short version = VERSION;
	short no_tracks = tracks.size();

	fp.write(title, 4);
	fp.write((char*)&version, 2);
	fp.write((char*)&no_tracks, 2);
	fp.write((char*)&no_faces, 4);

	glm::i16vec4 bounds = GetBoundingBox();

	fp.write((char*)&bounds, sizeof(bounds));

	auto verts = [&mta]()
	{
		std::vector<glm::i16vec2> r(mta.noEdges());

		for(auto i : mta.edgeRange())
		{
			r[i] = mta.verts(i);
		}

		return r;
	}();

	fp.write((char*)&verts[0], sizeof(verts[0]) * verts.size());
	fp.write((char*)&mta._gravity[0],  4 * no_faces);
	fp.write((char*)&mta._roomType[0], 1 * no_faces);
	fp.write((char*)&mta._music[0],    1 * no_faces);

	fp.write((char*)&mta._directionalShade[0], 4 * no_faces);
	fp.write((char*)&mta._ambientShade[0], no_faces);
	fp.write((char*)&mta._audio[0], 4 * no_faces);


	for(uint32_t i = 0; i < tracks.size();++i)
	{
		uint8_t length = tracks[i].size();
		fp.write((char*)&length, 1);
		fp.write((char*)&tracks[i][0], length);
	}

	fp.write((char*)&door_indices[0], sizeof(QuadTree::DoorList) * door_indices.size());
	fp.write((char*)&door_list[0], sizeof(QuadTree::Door) * door_list.size());

	mta._tree.WriteTree(fp);

	ColorRooms color(door_list, door_indices, mta._tree.GetEdgeFlags());
	color.DoColoring();

	auto const& coloring = color.GetColoring();
	int8_t noColors = color.MaxColorUsed();

	fp.write((char*)&noColors, sizeof(noColors));
	fp.write((char*)&coloring[0], sizeof(coloring[0]) * coloring.size());

	return offset;
}

std::vector<uint32_t> Metaroom::Insert(std::vector<Room> const& list)
{
	auto faces = AddFaces(list.size());

	for(auto i : faces)
		SetRoom(i, list[i]);

	_selection.clear();

	for(auto i : faces)
	{
		_selection.select_face(i, Bitwise::SET);
	}

	CommitMove();

	return faces;
}


void Metaroom::Insert(std::vector<Room> const& list, std::vector<uint32_t> const& indices)
{
	for(auto i : indices)
		_entitySystem.GetEntity(i);

    for(uint32_t i = 0; i < list.size(); ++i)
		SetRoom(indices[i], list[i]);

	_selection.clear();

	for(int i : indices)
		_selection.select_face(i, Bitwise::SET);

	CommitMove();
}

std::vector<uint32_t> Metaroom::Slice(std::vector<SliceInfo> & slice)
{
	std::vector<uint32_t> face_list;
	face_list.reserve(slice.size()/2);

	for(size_t i = 0; i < slice.size(); i += 2)
		face_list.push_back(slice[i].edge / 4);

	auto result = Duplicate(face_list);

	for(auto  i = 0u; i < slice.size(); i += 2)
	{
		int j = result[i];
		int e = slice[i].edge % 4;

		_verts[j][e]         = slice[i  ].vertex;
		_verts[j][(e+3) % 4] = slice[i+1].vertex;
	}

	for(size_t i = 0; i < slice.size(); i += 2)
	{
		std::swap(raw()[NextInEdge(slice[i].edge)], slice[i  ].vertex);
		std::swap(raw()[slice[i+1].edge], slice[i+1].vertex);
	}


	return result;
}

int Metaroom::GetPermeability() const
{
	auto doors = GetSelectedDoors();

	if(doors.empty())
		return -1;

	bool   match = false;
	int   w_perm = 100;

	uint64_t door = GetDoorKey(doors[0].first, doors[0].second);

	for(auto pair : doors)
	{
		auto key = GetDoorKey(pair.first, pair.second);

		auto itr = _permeabilities.find(key);

		if(itr != _permeabilities.end())
		{
			if(match == true && w_perm != itr->second)
				return -1;

			w_perm = itr->second;
			match  = true;
		}
	}

	return w_perm;
}


std::vector<std::pair<int, int>> Metaroom::GetSelectedDoors() const
{
	std::vector<std::pair<int, int>> r;

	for(auto i : edgeRange())
	{
		if(!_selection.IsEdgeSelected(i))
			continue;

		auto edges = _tree.GetOverlappingEdges(GetVertex(i), GetNextVertex(i));

		std::sort(edges.begin(), edges.end());

		for(auto j : edges)
		{
			if(_selection.IsEdgeSelected(j) &&	i < j)
				r.push_back({i/4, j/4});
		}
	}

	return r;
}

std::vector<int> Metaroom::GetExtrudableEdges() const
{
	std::vector<int> r;

	for(auto i : edgeRange())
	{
		if( _selection.IsEdgeSelected(i)
		&& !_tree.HasOverlappingEdges(i))
			r.push_back(i);
	}

	return r;
}



bool Metaroom::CanAddFace(glm::ivec2 * verts)
{
	glm::ivec2 min = glm::min(glm::min(verts[0], verts[1]), glm::min(verts[2], verts[3]));
	glm::ivec2 max = glm::max(glm::max(verts[0], verts[1]), glm::max(verts[2], verts[3]));

	RoomRange range(&_tree, min, max);

	while(range.popFront())
	{
		int N = (range.face()+1)*4;

//if the first point is not contained then if we intersect there must be a line crossing boundary...
	/*	if(math::IsPointContained(&raw()[range.face()*4], verts[0])
		|| math::IsPointContained(verts, raw()[range.face()*4]))
			return false;*/

		for(int i = range.face()*4; i < N; ++i)
		{
			for(int j = 0; j < 4; ++j)
			{
				if(math::DoLinesIntersect(GetVertex(i), GetNextVertex(i), verts[j], verts[(j+1)%4]))
					return false;
			}
		}
	}

	return true;
}

void Metaroom::AddFace(glm::ivec2 min, glm::ivec2 max)
{
	glm::ivec2 verts[4]{ max, {max.x, min.y}, min, {min.y, max.y} };

    if(CanAddFace(verts))
		return;

	int face = AddFaces(1)[0];

	memcpy(&raw()[face*4], verts, sizeof(verts));

	_gravity[face]   = glm::packHalf2x16(glm::vec2(0, 9.81));
	_music[face]     = -1;
	_roomType[face]  = 0;

	_directionalShade[0]	= 0;
	_ambientShade[0]		= 0;
	_audio[0]				= glm::vec4(0);

	memcpy_s(&_prev[face], &_verts[face]);

	_selection.select_face(face, Bitwise::SET);
	gl.SetDirty();
}

std::vector<uint32_t> Metaroom::AddFaces(uint32_t no_faces)
{
	gl.SetDirty();
	_tree.SetDirty();

	std::vector<uint32_t> x;
	x.resize(no_faces);

	for(auto i = 0u; i < no_faces; ++i)
	{
		x[i] = _entitySystem.GetEntity();
	}

	return x;

	AddFaces();
}

void Metaroom::RemoveFace(int id)
{
	gl.SetDirty();

	_entitySystem.ReleaseEntity(id);

	_selection.erase(id);
	_tree.SetDirty();
}

void Metaroom::RemoveFaces(std::vector<uint32_t> const& vec)
{
	gl.SetDirty();
	_selection.clear();
	_tree.SetDirty();

	for(auto item : vec)
	{
		_entitySystem.ReleaseEntity(item);
	}
}

void Metaroom::CancelMove()
{
	_verts = _prev.clone();
	gl.SetDirty();
}

void Metaroom::CommitMove(bool update_mvsf)
{
	_prev = _verts.clone();
	_tree.SetDirty(update_mvsf);
	gl.SetDirty();
}

void Metaroom::Translate(glm::ivec2 translation, glm::ivec2 half_dimensions)
{
	gl.SetDirty();

	if(_prev.size() < _verts.size())
		_prev = _verts.clone();

	if(_scratch.size() < _verts.size())
		_scratch = _verts.clone();

	for(auto i : edgeRange())
	{
		if(_selection.IsVertSelected(i))
		{
			scratch(i) = prev(i) + translation;
			scratch(i) = glm::max(-half_dimensions, glm::min(scratch(i), half_dimensions));
		}
	}

	solve_constraints();
	std::swap(_scratch, _verts);
}

void Metaroom::Rotate(glm::ivec2 center, glm::vec2 complex)
{
	gl.SetDirty();

	_scratch = _prev.clone();

	for(auto i : edgeRange())
	{
		if(_selection.IsVertSelected(i))
		{
			glm::ivec2 p = scratch(i)  - center;

			p = glm::ivec2(
					p.x * complex.x - p.y * complex.y,
					p.x * complex.y + p.y * complex.x);

			scratch(i) = p + center;
		}
	}

	solve_constraints();
	std::swap(_scratch, _verts);
}

void Metaroom::Scale(glm::ivec2 center, glm::vec2 scale)
{
	gl.SetDirty();

	_scratch = _prev.clone();

	for(auto i : edgeRange())
	{
		if(_selection.IsVertSelected(i))
			scratch(i) = glm::ivec2(glm::vec2(scratch(i) - center) * scale) + center;
	}

	solve_constraints();
	std::swap(_scratch, _verts);
}

void Metaroom::RestoreVerts(const std::vector<glm::i16vec2> & vec)
{
	gl.SetDirty();

	for(auto i : edgeRange())
	{
		raw()[i] = vec[i];
	}

	_prev = _verts.clone();
}

glm::ivec2 Metaroom::GetSelectionCenter() const
{
	glm::ivec2 accumulator(0, 0);
	int32_t   size{};

	for(auto i : edgeRange())
	{
		int is_selected = _selection.IsVertSelected(i);

		accumulator += raw()[i] * is_selected;
		size        += is_selected;
	}

	return accumulator / (size + (int) (size == 0));
}

void Metaroom::BoxSelect(glm::ivec2 tl, glm::ivec2 br, Bitwise flags)
{
	if(flags == Bitwise::SET)
		_selection.clear();
	if(flags == Bitwise::AND)
		_selection.begin_and();

	for(auto i : edgeRange())
	{
		if(tl.x < raw()[i].x && raw()[i].x < br.x
		&& tl.y < raw()[i].y && raw()[i].y < br.y)
			_selection.select_vertex(i, flags);
	}

	if(flags == Bitwise::AND)
		_selection.end_and();

	update_selections();
	return;
}

void Metaroom::ClickSelect(glm::ivec2 pos, Bitwise flags, bool alt, float zoom)
{
	if(flags == Bitwise::SET)
		_selection.clear();
	if(flags == Bitwise::AND)
		_selection.begin_and();

	float radius = 36 / zoom;

	bool selected = false;

	for(auto i : edgeRange())
	{
		if(math::length2(raw()[i] - pos) < radius)
		{
			_selection.select_vertex(i, flags);
			selected = true;
		}
	}

	if(selected == false)
	{
		for(auto i : edgeRange())
		{
			int j = NextInEdge(i);

			float distance2 = math::SemgentDistanceSquared(raw()[i], raw()[j], pos);

			if(0 <= distance2 && distance2 < radius)
			{
				_selection.select_edge(i, flags);
				selected = true;

				if(alt)
				{
					RingSelectEdge(i, flags);
				}
			}
		}
	}

	if(selected == false)
	{
		int face = GetFace(pos);

		if(face != -1)
		{
			_selection.select_face(face, flags);
			if(alt)	RingSelectFace(face, pos, flags);
		}
	}

	if(flags == Bitwise::AND)
		_selection.end_and();

	update_selections();
}

int        Metaroom::GetSliceEdge(glm::ivec2 p) const
{
	int face = GetFace(p);

	if(face == -1)
		return -1;

	int best_edge      = -1;
	float min_distance = FLT_MAX;

	for(int i = 0; i < 4; ++i)
	{
		float distance2 = math::LineDistanceSquared(_verts[face][i], _verts[face][(i+1)%4], p);

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

void Metaroom::RingSelectFace(int face, glm::ivec2 mouse, Bitwise flags)
{
	int best_edge		= -1;
	float best_distance = FLT_MAX;

	for(int i = face*4; i < (face+1)*4; ++i)
	{
		int j = NextInEdge(i);

		float distance2 = math::SemgentDistanceSquared(raw()[i], raw()[j], mouse);

		if(0 <= distance2 && distance2 < best_distance)
		{
			best_edge = i;
			best_distance = distance2;
		}
	}

	float percentage = ProjectOntoEdge(best_edge, mouse);

	if(flags == Bitwise::SET)
		flags = Bitwise::OR;

	_selection.MarkFace(face);
	RingSelectFaceInternal(best_edge, GetPointOnEdge(best_edge, percentage), flags);
	best_edge = Metaroom::GetOppositeEdge(best_edge);
	RingSelectFaceInternal(best_edge, GetPointOnEdge(best_edge, percentage), flags);
	_selection.ClearMarks();
}

void Metaroom::RingSelectFaceInternal(int edge, glm::ivec2 position, Bitwise flags)
{
	float mid;

	while(_tree.GetSliceFace(
		GetVertex(edge),
		GetVertex(Metaroom::NextInEdge(edge)),
		position,
		edge,
		mid))
	{
		if(_selection.MarkFace(edge/4) == false)
			return;

		_selection.select_face(edge/4, flags);

		edge     = Metaroom::GetOppositeEdge(edge);
		position = GetPointOnEdge(edge, mid);
	}
}

void Metaroom::RingSelectEdge(int edge, Bitwise flags)
{


}


std::string Metaroom::TestTreeSymmetry()
{
	std::string r;

	for(auto i : range())
	{
		glm::i16vec2 tl, br;
		GetFaceAABB(i, tl, br);
		RoomRange range(&_tree, tl, br);

		while(range.popFront())
		{
			if(range.face() < (int)i)
				continue;

			bool match_found = false;

			RoomRange r2(&_tree, range.face());
			while(r2.popFront())
			{
				if(r2.face() == (int)i)
				{
					match_found = true;
					break;
				}
			}

			if(match_found == false)
			{
				char buffer[256];
				snprintf(buffer, sizeof(buffer), "Tree asymmetrical for pair %d, %d\n", i, range.face());
				r += buffer;
			}
		}
	}

	return {};
}

std::string Metaroom::TestDoorSymmetry()
{
	std::string r;

	for(auto i : edgeRange())
	{
		EdgeRange range(&_tree, i);

		while(range.popFront())
		{
			if(range.face() < (int)i/4)
				continue;

			bool match_found = false;

			EdgeRange r2(&_tree, range.edge());
			while(r2.popFront())
			{
				if(r2.edge() == (int)i)
				{
					match_found = true;
					break;
				}
			}

			if(match_found == false)
			{
				char buffer[256];
				snprintf(buffer, sizeof(buffer), "Tree asymmetrical for pair %d, %d\n", i, range.edge());
				r += buffer;
			}
		}
	}

	return {};
}

/*
#include "edgerange.h"

void Metaroom::RingSelectEdgeInternal(int edge, Bitwise flags)
{
	EdgeRange range(&_tree, edge);

	int bestEdge = -1;
	float highestDot = FLT_MIN;

	auto v0 = GetVertex(edge);
	auto v1 = GetNextVertex(edge);
	auto vec = v1 - v0;

	while(range.popFront(EdgeRange::Flags::None))
	{
		for(int i = 0; i < 4; ++i)
		{
			auto a0 = GetVertex(range.face()*4+i);
			auto a1 = GetNextVertex(range.face()*4+i);

			if(!(v0 == a0 || v0 == a1 || v1 == a0 || v1 == a1))
				continue;

			float dot = std::fabs(glm::dot(vec, a1 - a0));

			if(dot > highestDot)
			{
				highestDot = dot;
				bestEdge = range.face()*4+i;
			}
		}
	}



}
*/

std::vector<uint32_t> Metaroom::Duplicate(std::vector<uint32_t> const& faces)
{
	auto indices = AddFaces(faces.size());
	auto first = indices.data();

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy_s(&raw()[indices[i]*4],      &raw()[faces[i]*4], 4);

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy_s(&_gravity[*(first+i)],      &_gravity[faces[i]]);

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy_s(&_music[*(first+i)],        &_music[faces[i]]);

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy_s(&_directionalShade[*(first+i)], &_directionalShade[faces[i]]);

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy_s(&_ambientShade[*(first+i)], &_ambientShade[faces[i]]);

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy_s(&_audio[*(first+i)], &_audio[faces[i]]);

	for(size_t i = 0; i < faces.size(); ++i)
		memcpy_s(&_roomType[*(first+i)],     &_roomType[faces[i]]);


	return indices;
}

void Metaroom::PruneDegenerate()
{
	std::vector<uint32_t> degenerate_faces;

	for(auto i : range())
	{
		int sum = 0;

		for(int j = 0; j < 4; ++j)
		{
			for(int k = j+1; k < 4; ++k)
			{
				sum += (GetVertex(i*4 + j) == GetVertex(i*4 + k));
			}
		}

		if(sum > 1)
			degenerate_faces.push_back(i);
	}

	RemoveFaces(degenerate_faces);
	LOG_F(INFO, "pruned %i degenerate faces\n", (int)degenerate_faces.size());
}
