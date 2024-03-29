#include "metaroom.h"
#include "colorprogressindicator.h"
#include "lf_math.h"
#include "qapplication.h"
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
#include <thread>
#include <chrono>

using namespace std::literals::chrono_literals;

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

	if(version == VERSION_STILL_HAVE_SIZE)
	{
		fp.read((char*)&width, 2);
		fp.read((char*)&height, 2);
	}
	if(version >= VERSION_ADDED_MUSIC)
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


	fp.read((char*)&_gravity[0],  4*no_faces);
	fp.read((char*)&_roomType[0], no_faces);

	if(version < VERSION_ADDED_SHADE)
	{
		fp.seekg(4 * no_faces, std::ios_base::cur);
	}

	fp.read((char*)&_music[0], 1* no_faces);

	if(version >= VERSION_ADDED_SHADE)
	{
		fp.read((char*)&_directionalShade[0], 4*no_faces);
		fp.read((char*)&_ambientShade[0], no_faces);
		fp.read((char*)&_audio[0],  4*no_faces);
	}
	else
	{
		memset(&_directionalShade[0], 0, _directionalShade.byteLength());
		memset(&_ambientShade[0], 0, _ambientShade.byteLength());
		memset(&_audio[0], 0, _audio.byteLength());
	}

	if(version >= VERSION_ADDED_DEPTH)
	{
		fp.read((char*)&_depth[0], sizeof(_depth[0]) * no_faces);
	}
	else
	{
		for(auto & item : _depth)
			item = glm::u16vec2(0, 4096);
	}

	if(version <= VERSION_ADDED_MUSIC)
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
	fp.read((char*)&door_indices[0], sizeof(door_indices[0]) * door_indices.size());

	for(auto i = 1u; i < door_indices.size(); ++i)
	{
		if(door_indices[i].index != door_indices[i-1].end())
		{
			qWarning("Problem reading door indices");
			goto end;
		}
	}

	door_list.resize(door_indices.back().index + door_indices.back().length);
	fp.read((char*)&door_list[0], sizeof(door_list[0]) * door_list.size());

	for(auto & list : door_indices)
	{
		int  index = (&list - &door_indices[0]) / 4;
		auto begin = &door_list[list.index];
		auto end   = std::min(begin + list.length, door_list.data() + door_list.size());

		for(auto i = begin; i < end; ++i)
		{
			if(0 <= i->perm && i->perm < 100 && index < i->face && i->face < (int)no_faces)
			{
				_permeabilities[GetDoorKey(index, i->face)] = i->perm;
			}
		}
	}

	if(version >= VERSION_ADDED_MVSF)
		_tree.ReadTree(fp);
end:
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
	fp.write((char*)&_depth[0], sizeof(_depth[0]) * no_faces);


	for(uint32_t i = 0; i < tracks.size();++i)
	{
		uint8_t length = tracks[i].size();
		fp.write((char*)&length, 1);
		fp.write((char*)&tracks[i][0], length);
	}

	fp.write((char*)&door_indices[0], sizeof(QuadTree::DoorList) * door_indices.size());
	fp.write((char*)&door_list[0], sizeof(QuadTree::Door) * door_list.size());

	mta._tree.WriteTree(fp);

	std::shared_ptr<ColorProgress> progress = std::make_shared<ColorProgress>();
	std::thread coloring_thread([&door_list, &door_indices, &mta, &progress]()
	{
		ColorRooms color(door_list, door_indices, mta._tree.GetEdgeFlags());
		color.DoColoring(progress);
	});

	std::this_thread::sleep_for(50ms);
	if(progress->_complete == false)
	{
		std::unique_ptr<ColorProgressIndicator> indicator(new ColorProgressIndicator(window, progress));
		indicator->show();

		while(progress->_complete == false)
		{
			std::this_thread::sleep_for(20ms);
			indicator->Update();

			QCoreApplication::processEvents();
		}

		indicator->close();
	}

	coloring_thread.join();

	auto coloring = std::move(progress->coloring);
	int8_t noColors = progress->noColors;

	fp.write((char*)&noColors, sizeof(noColors));
	fp.write((char*)&coloring[0], sizeof(coloring[0]) * coloring.size());

	return offset;
}

std::vector<uint32_t> Metaroom::Insert(std::vector<Room> const& list)
{
	auto faces = AddFaces(list.size());

	for(auto i = 0u; i < faces.size(); ++i)
		SetRoom(faces[i], list[i]);

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

std::vector<uint32_t> Metaroom::Slice(SliceArray & slice)
{
	std::vector<uint32_t> face_list;
	face_list.reserve(slice.size()/2);

	for(size_t i = 0; i < slice.size(); ++i)
		face_list.push_back(slice[i][0].edge / 4);

	auto result = Duplicate(face_list);

	for(size_t i = 0, j = 0; i < slice.size(); i = j)
	{
		const int e = slice[i][0].edge % 4;

		int itr = result[i];

		_verts[itr][e]         = slice[i][0].vertex;
		_verts[itr][(e+3) % 4] = slice[i][1].vertex;

		for(j = i+1; j < slice.size() && slice[j][0].edge == slice[i][0].edge; ++j)
		{
			itr = result[j];

			_verts[itr][(e+1)%4] = slice[j-1][0].vertex;
			_verts[itr][(e+2)%4] = slice[j-1][1].vertex;

			_verts[itr][e]         = slice[j][0].vertex;
			_verts[itr][(e+3) % 4] = slice[j][1].vertex;
		}

		std::swap(raw()[NextInEdge(slice[j-1][0].edge)], slice[j-1][0].vertex);
		std::swap(raw()[slice[j-1][1].edge], slice[j-1][1].vertex);
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

	_directionalShade[face]	= 0;
	_ambientShade[face]		= 0;
	_audio[face]			= glm::vec4(0);
	_depth[face]			= glm::u16vec2(0, USHRT_MAX);

	memcpy_s(&_prev[face], &_verts[face]);

	_selection.select_face(face, Bitwise::SET);
	gl.SetIndicesDirty();
}

std::vector<uint32_t> Metaroom::AddFaces(uint32_t no_faces)
{
	gl.SetIndicesDirty();
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
	gl.SetIndicesDirty();

	_entitySystem.ReleaseEntity(id);

	_selection.erase(id);
	_tree.SetDirty();
}

void Metaroom::RemoveFaces(std::vector<uint32_t> const& vec)
{
	gl.SetIndicesDirty();
	_selection.clear();
	_tree.SetDirty();

	for(auto item : vec)
	{
		_entitySystem.ReleaseEntity(item);
	}
}

void Metaroom::BeginMove()
{
	_prev = _verts.clone();
	_scratch = _verts.clone();
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
	document->OnTransformed();
}


void Metaroom::Translate(glm::ivec2 translation, glm::ivec2 half_dimensions)
{
	gl.SetDirty();

	assert(_prev.size() == _verts.size());
	assert(_scratch.size() == _verts.size());


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

	assert(_prev.size() == _verts.size());
	assert(_scratch.size() == _verts.size());

	for(auto i : edgeRange())
	{
		if(_selection.IsVertSelected(i))
		{
			glm::ivec2 p = prev(i)  - center;

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

	assert(_prev.size() == _verts.size());
	assert(_scratch.size() == _verts.size());

	for(auto i : edgeRange())
	{
		if(_selection.IsVertSelected(i))
			scratch(i) = glm::ivec2(glm::vec2(prev(i) - center) * scale) + center;
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

		if(alt)
		{
			_selection.ClearMarks();
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
	float best_distance = -1.f;

	auto dirn = GetCenter(face)- glm::vec2(mouse);

	for(int i = 0; i < 4; ++i)
	{
		auto distance2 = glm::dot(GetNormal(face, i), dirn);

		if(distance2 > best_distance)
		{
			best_edge = face*4 + i;
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
	if(flags == Bitwise::SET)
		flags = Bitwise::OR;

	if(_selection.MarkFace(edge) == false)
		return;

	std::vector<QuadTree::VertexQuery> stack;
	std::vector<QuadTree::VertexQuery> query;
	stack.push_back({uint32_t(edge/4), uint32_t(edge&0x03)});

	while(stack.size())
	{
		auto top = stack.back();
		stack.pop_back();

		query.clear();
		_tree.GetRoomsWithVertex(top.face, top.edge, query);

		if(query.empty())
			continue;

		auto abs_normal = glm::abs(GetNormal(top.face, top.edge));

		for(auto i : query)
		{
			auto next_normal = glm::abs(GetNormal(i.face, i.edge));
			auto prev_normal = glm::abs(GetNormal(i.face, (i.edge+3) & 3));

			if(glm::dot(next_normal, abs_normal) > glm::dot(prev_normal, abs_normal))
			{
			//	i.edge = (i.edge+1u)&0x03u;
			}
			else
			{
				i.edge = (i.edge+3u)&0x03u;
			//	continue;
			}

			if(_selection.MarkFace(i.face*4 + i.edge))
			{
				assert(_selection.MarkFace(i.face*4 + i.edge) == false);
				_selection.select_edge(i.face*4 + i.edge, flags);
				assert(_selection.MarkFace(i.face*4 + i.edge) == false);
				stack.push_back(i);
			}
			else
			{
				int break_point{0};
				++break_point;
			}
		}


	}
}

void Metaroom::RingSelectEdgeInternal(int edge, Bitwise flags)
{
	float mid;

	while(_tree.GetSliceFace(
		GetVertex(edge),
		GetVertex(Metaroom::NextInEdge(edge)),
	{INT_MIN, INT_MIN},
		edge,
		mid))
	{
		if(_selection.MarkFace(edge/4) == false)
			return;

		_selection.select_edge(edge, flags);

		edge     = Metaroom::GetOppositeEdge(edge);
	}
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
		memcpy_s(&_depth[*(first+i)],    &_depth[faces[i]]);

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

std::vector<std::pair<glm::vec4, float>> Metaroom::GetPermTable() const
{
	std::vector<std::pair<glm::vec4, float>> map;

	for(auto & itr : _permeabilities )
	{
		glm::vec4 x;
		(glm::vec2&)x.x = GetCenter(((uint32_t*)(&itr.first))[0]);
		(glm::vec2&)x.z = GetCenter(((uint32_t*)(&itr.first))[1]);
		map.emplace_back(x, itr.second);
	}

	return map;
}

void Metaroom::SetPermTable(std::vector<std::pair<glm::vec4, float>> const& map)
{
	_permeabilities.clear();

	for(auto item : map)
	{
		_permeabilities.emplace(
					GetDoorKey(_tree.GetFace(item.first), _tree.GetFace((glm::vec2&)item.first.z)),
					item.second);
	}


}
