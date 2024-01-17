#include "quadtree.h"
#include "metaroom.h"
#include "lf_math.h"
#include "edgerange.h"
#include "mvsf_sampler.h"
#include "qopenglfunctions_4_5_core.h"
#include "src/Shaders/shaders.h"
#include "Shaders/doorshader.h"
#include <climits>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <stack>

QuadTree::QuadTree(Metaroom * room) :
    m_metaroom(room)
{
}

QuadTree::~QuadTree()
{
}

void QuadTree::Release(Shaders * shaders)
{
	auto gl = shaders->gl;

    gl->glDeleteVertexArrays(1, &m_vao);
    gl->glDeleteBuffers(1, &m_vbo);

    m_vao = 0;
    m_vbo = 0;
}

bool QuadTree::empty() const
{
	return m_metaroom->noFaces() == 0;
}

void QuadTree::Rebuild()
{
	m_dirty     = false;
	m_vbo_dirty = true;

	glm::i16vec2 min, max;
	uint32_t     size = m_metaroom->noFaces();
	auto bounds = m_metaroom->GetBoundingBox();

	bounds.x *= -2 * (bounds.x < 0);
	bounds.y *= -2 * (bounds.y < 0);

	if(size == 0) return;

	std::unique_ptr<Leaf[]> leaves(new Leaf[m_metaroom->noFaces()]);

	for(auto i : m_metaroom->range())
	{
		m_metaroom->GetFaceAABB(i, min, max);

		leaves[i].face_id = i;
		leaves[i].z_order = math::GetZOrder(glm::ivec2(min.x + max.x + bounds.x, min.y + max.y + bounds.y)/2);
		leaves[i].min     = min;
		leaves[i].max     = max;
	}

	std::sort(&leaves[0], &leaves[0] + size,
		[](const Leaf & a, const Leaf & b)
			{ return a.z_order < b.z_order; });

	uint32_t layer_size = size;
	uint32_t alloc = 2;

	while(layer_size > 1)
	{
		alloc     += layer_size;
		layer_size = (layer_size+1)/2;
	}

	if(alloced < alloc)
	{
		alloced = alloc;
		m_nodes = std::unique_ptr<Node[]>(new Node[alloc]);
	}

	uint32_t length = BuildTree(&m_nodes[0], &leaves[0], 0, size, 0, 0, 1, alloc);
//	fprintf(stderr, "final length: %ui\nallocated: %ui", length, alloced);
}

void QuadTree::ReadTree(std::ifstream & fp)
{
	uint32_t length;
	fp.read((char*)&length, 4);

	if(length)
	{
		m_mvsf.resize(length);
		fp.read((char*)&m_mvsf[0], length * sizeof(m_mvsf[0]));
	}
}

void QuadTree::WriteTree(std::ofstream & fp)
{
	if(m_mvsf.empty())
	{
		std::vector<glm::i16vec4> boxes = MVSF_sampler::GetBoundingBoxes(m_metaroom);
		m_mvsf = CreateNodes(m_metaroom->GetBoundingBox(), &boxes[0], m_metaroom->noFaces());
	}

	uint32_t length = m_mvsf.size();
	fp.write((char*)&length, 4);

	if(length)
		fp.write((char*)&m_mvsf[0], length * sizeof(m_mvsf[0]));
}

/// this works as proved by the quad tree algorithm being reused for all the interaction
/// the only difference is the source being from the mvsf sampler
std::vector<QuadTree::Node> QuadTree::CreateNodes(glm::i16vec4 bounds, glm::i16vec4 const* boxes, uint32_t size)
{
	bounds.x *= -2 * (bounds.x < 0);
	bounds.y *= -2 * (bounds.y < 0);

	if(size == 0) return std::vector<Node>();

	std::unique_ptr<Leaf[]> leaves(new Leaf[size]);

	for(uint32_t i = 0; i < size; ++i)
	{
		auto min =  glm::i16vec2(boxes[i].x, boxes[i].y);
		auto max =  glm::i16vec2(boxes[i].z, boxes[i].w);

		leaves[i].face_id = i;
		leaves[i].z_order = math::GetZOrder(glm::ivec2(min.x + max.x + bounds.x, min.y + max.y + bounds.y)/2);
		leaves[i].min     = min;
		leaves[i].max     = max;
	}

	std::sort(&leaves[0], &leaves[0] + size,
		[](const Leaf & a, const Leaf & b)
			{ return a.z_order < b.z_order; });

	uint32_t layer_size = size;
	uint32_t alloc = 2;

	while(layer_size > 1)
	{
		alloc     += layer_size;
		layer_size = (layer_size+1)/2;
	}

	std::vector<Node> nodes(alloc);

	BuildTree(&nodes[0], &leaves[0], 0, size, 0, 0, 1, alloc);
//	fprintf(stderr, "final length: %ui\nallocated: %ui", length, alloced);

	return nodes;
}

void QuadTree::PrintTree(int i, int tabs) const
{
	fprintf(stderr, "%*.smin { %d, %d} max { %d, %d }\n", tabs*2, "",
		m_nodes[i].min.x, m_nodes[i].min.y,
		m_nodes[i].max.x, m_nodes[i].max.y);

	if(m_nodes[i].leaf)
		fprintf(stderr, "%*.s-> %d\n", (tabs+1)*2, "", m_nodes[i].child);
	else
	{
		PrintTree(m_nodes[i].child+0, tabs+1);
		PrintTree(m_nodes[i].child+1, tabs+1);
	}
}

uint32_t QuadTree::BuildTree(Node * nodes, const Leaf * leaves, uint32_t min, uint32_t max, uint32_t mask, uint32_t node, uint32_t base, uint32_t alloc, uint32_t depth)
{
	assert(alloc > base);

	if(min+1 >= max)
	{
		nodes[node].leaf  = true;
		nodes[node].child = leaves[min].face_id;
		nodes[node].min   = leaves[min].min;
		nodes[node].max   = leaves[min].max;
		return base;
	}

	uint32_t search  = leaves[max-1].z_order;

	search = search & ~mask;
	search = search & (search-1);
	search = search | mask;

	uint32_t r_min = min+1;
	uint32_t r_max = max-2;

	while(r_min <= r_max)
	{
		uint32_t mid = (r_min + r_max) / 2;

		if(leaves[mid].z_order < search)
		{
			r_min = mid+1;

			if(leaves[mid+1].z_order >= search)
				break;
		}
		else
		{
			if(mid > min && leaves[mid-1].z_order < search)
			{
				r_min = mid;
				break;
			}

			r_max = mid-1;
		}
	}

	nodes[node].leaf  = false;
	nodes[node].child = base;

	base = BuildTree(nodes, leaves,   min,  r_min, mask,          nodes[node].child+0, base+2, alloc, depth+1);
	base = BuildTree(nodes, leaves, r_min,    max, mask | search, nodes[node].child+1, base  , alloc, depth+1);

	nodes[node].min   = glm::min(nodes[nodes[node].child+0].min, nodes[nodes[node].child+1].min);
	nodes[node].max   = glm::max(nodes[nodes[node].child+0].max, nodes[nodes[node].child+1].max);

	return base;
}

bool QuadTree::HasOverlappingEdges(int v)
{
	if(IsDirty()) Rebuild();

	EdgeRange range(this, v);

	while(range.popFront())
	{
		if(math::DoesOverlap(range.v0, range.v1-range.v0, range.a0, range.a1))
			return true;
	}

	return false;
}

bool QuadTree::HasFullOverlap(int v)
{
	if(IsDirty()) Rebuild();

	EdgeRange range(this, v);

	float coverage = 0.f;

	std::pair<float, int>  begin, end;

	while(range.popFront())
	{
		if(math::GetOverlap(range.v0, range.v1, range.a0, range.a1, &begin, &end))
		{
			coverage += (end.first - begin.first);
		}
	}

	return coverage >= .98f;
}

std::vector<uint32_t> QuadTree::GetOverlappingEdges(glm::ivec2 v0, glm::ivec2 v1)
{
	if(IsDirty()) Rebuild();

	std::vector<uint32_t>  r;
	EdgeRange range(this, v0, v1);

	while(range.popFront())
	{
		if(math::DoesOverlap(v0, v1-v0, range.a0, range.a1))
		{
			if(r.size())
				assert(r.back() != range.edge());

			r.push_back(range.edge());
		}
	}

	return r;
}


bool QuadTree::GetSliceFace(const glm::ivec2 v0, const glm::ivec2 v1, const glm::ivec2 v2, int & edge_id, float & mid)
{
	EdgeRange range(this, v0, v1, v2);

	while(range.popFront())
	{
		float t0;
		glm::ivec2 v = range.a1 - range.a0;

		if(std::abs(v.x) < std::abs(v.y))
			t0 = (v2.y - range.a0.y) / (float) v.y;
		else
			t0 = (v2.x - range.a0.x) / (float) v.x;

		if(0 < t0 && t0 < 1.f)
		{
			edge_id = range.edge();
			mid     = range.index() < 2? 1.f - t0 : t0;
			return true;
		}
	}

	return false;
}

int QuadTree::GetFace(glm::ivec2 position)
{
	if(IsDirty()) Rebuild();

	if(m_nodes == nullptr)
		return -1;


	std::stack<int> stack;
	stack.push(0);

	while(stack.size())
	{
		auto & node = m_nodes[stack.top()];
		stack.pop();

		if(math::boxContains(position, node.min, node.max))
		{
			if(node.leaf)
			{
				if(m_metaroom->IsPointContained(node.child, position))
				{
					return node.child;
				}
			}
			else
			{
				stack.push(node.child+0);
				stack.push(node.child+1);
			}
		}
	}

	return -1;
}

void QuadTree::GetWriteDoors(std::vector<Door> & doors, std::vector<DoorList> & indices, bool make_halls, int typeId)
{
	if(IsDirty()) Rebuild();

	if(m_nodes == nullptr)
		return;

	std::stack<int> s;
	std::vector<DoorInfo> info;

	indices.resize(m_metaroom->noVertices());
	doors.reserve(m_metaroom->noEdges()*2);

	for(auto i : m_metaroom->edgeRange())
	{
		//if(typeId && m_metaroom->m_doorType[i] != 0)	continue;

		info.clear();
		GetWriteDoors(i, s, info, typeId);

		if(info.size() == 0)
		{
			indices[i].min_perm = -1;
			indices[i].max_perm = -1;
			indices[i].length   = 0;
			indices[i].index    = doors.size();
			continue;
		}

		int8_t min_perm = 100;
		int8_t max_perm = -1;
		bool solid = true;
		float begin = 0.f;

		if(make_halls == false)
		{
			for(size_t i = 0; i < info.size(); ++i)
			{
				if(begin < info[i].begin)
				{
					info.insert(info.begin()+i, {begin, info[i].begin, -1, -1});
					min_perm = -1;
				}

				begin = info[i].end;
				min_perm = std::min<char>(min_perm, info[i].perm);
				max_perm = std::max<char>(max_perm, info[i].perm);
			}

			if(begin < 1.f)
			{
				info.insert(info.end(), {begin, 1.f, -1, -1});
				min_perm = -1;
				solid = false;
			}
		}
		
		indices[i].min_perm = min_perm;
		indices[i].max_perm = max_perm;
		indices[i].length = info.size();
		indices[i].index  = doors.size();

		if(info.size() > indices[i].length)
		{
			throw std::runtime_error("room has too many doors for save format.");
		}

		for(size_t j = 0; j < info.size(); ++j)
		{
			int perm = m_metaroom->MetaroomMemory::GetPermeability(i/4, info[j].face);
			doors.push_back({perm, info[j].face, info[j].end});
		}
	}
}

void QuadTree::GetHallDoors(std::vector<int> & doors, std::vector<std::pair<int, int>> & indices)
{
	if(IsDirty()) Rebuild();

	if(m_nodes == nullptr)
		return;

	std::vector<DoorInfo> info;

	indices.resize(m_metaroom->noFaces());

	for(auto i : m_metaroom->edgeRange())
	{
		indices[i].first = doors.size();

		for(int j = 0; j < 4; ++j)
			GetHallDoors(i*4+j, doors);

		indices[i].second = doors.size();
	}
}


void QuadTree::GetHallDoors(int edge, std::vector<int> & doors)
{
	if(IsDirty()) Rebuild();

//	if(m_metaroom->m_doorType[edge] != 0)	return;

	EdgeRange range(this, edge);

	while(range.popFront())
	{
		if(math::DoesOverlap(range.v0, range.v1 - range.v0, range.a0, range.a1))
		{
			doors.push_back(range.face());
		}
	}
}

std::vector<uint8_t> QuadTree::GetEdgeFlags()
{
	if(IsDirty()) Rebuild();

	if(m_nodes == nullptr)
		return {};

	std::vector<uint8_t> r(m_metaroom->noFaces(), 0);

	auto min = m_nodes[0].min;
	auto max = m_nodes[0].max;

	glm::ivec2 coords[4][2]
	{
		{{min.x, min.y}, {max.x, min.y}},
		{{min.x, max.y}, {max.x, max.y}},
		{{min.x, min.y}, {min.x, max.y}},
		{{max.x, min.y}, {max.x, max.y}}
	};

	std::vector<int> vec;
	for(int i = 0; i < 4; ++i)
	{
		vec.clear();
		GetOverlappingRooms(coords[i][0], coords[i][1], vec);
		for(int j : vec) { r[j] |= 1 << i; }
	}

	return r;
}

void QuadTree::GetWriteDoors(int edge, std::stack<int> & stack, std::vector<DoorInfo> & edges, int typeId)
{	
	if(IsDirty()) Rebuild();

	//if(typeId >= 0 && m_metaroom->m_doorType[edge] != typeId) return;

	stack.push(0);

	EdgeRange range(this, edge);

	while(range.popFront())
	{
//		if(typeId >= 0 && m_metaroom->m_doorType[range.edge()] != typeId)			continue;

//determine how far each thing is from an axis or whatever.
		std::pair<float, int> begin, end;
		
		if(math::GetOverlap(range.v0, range.v1, range.a0, range.a1, &begin, &end))
		{
			
//convert into an edge struct
			DoorInfo info;
			info.begin = begin.first;
			info.end   = end.first;
			info.perm  = m_metaroom->MetaroomMemory::GetPermeability(edge/4, range.face());
			info.face  = range.face();

			edges.push_back(info);
		}
	}
	
	if(edges.size() > 1)
	{
		std::sort(edges.begin(), edges.end(), [](DoorInfo const& b0, DoorInfo const& b1) { return b0.end < b1.end; } );
		
		for(auto i = 0u; i < edges.size(); ++i)
		{
			assert(edges[i].begin != edges[i].end);	
		}
	}
}

std::vector<QuadTree::EdgeVertex> QuadTree::GetRenderWalls()
{
	std::vector<EdgeVertex> r;

	if(IsDirty()) Rebuild();

	if(m_nodes)
	{
		for(auto i : m_metaroom->edgeRange())
			GetRenderWalls(i, r);
	}

	return r;
}

void QuadTree::GetRenderWalls(int edge, std::vector<EdgeVertex> & edges) const
{
	int next_edge = (edge & 0xFFFFFFFC) + (((edge & 0x03) + 1) & 0x03);

	EdgeVertex e;
	e.type    = 0;//m_metaroom->m_doorType[edge];

	EdgeRange range(this, edge, false);

	while(range.popFront())
	{
	//	if(m_metaroom->m_doorType[range.edge()] != e.type)	continue;

//determine how far each thing is from an axis or whatever.
		std::pair<float, int> begin, end;

		if(math::GetOverlap(range.v0, range.v1, range.a0, range.a1, &begin, &end))
		{
			e.permeability = m_metaroom->MetaroomMemory::GetPermeability(edge/4, range.face());
// first 2 have to flip because one is going -> other is going <-
			int vertices[4] = { next_edge, edge, range.edge(), range.nextEdge() };

			e.vertex  = m_metaroom->GetVertex(vertices[begin.second]);
			edges.push_back(e);
			e.vertex  = m_metaroom->GetVertex(vertices[end.second]);
			edges.push_back(e);
		}
	}
}

bool QuadTree::DoesOverlap(glm::ivec2 min, glm::ivec2 max)
{
	if(IsDirty()) Rebuild();

	if(m_nodes == nullptr)
		return false;

	std::stack<int> stack;
	stack.push(0);

	while(stack.size())
	{
		auto & node = m_nodes[stack.top()];
		stack.pop();

		if(math::boxIntersects(min, max, node.min, node.max))
		{
			if(node.leaf == false)
			{
				stack.push(node.child+0);
				stack.push(node.child+1);
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}

void QuadTree::GetOverlappingRooms(glm::ivec2 min, glm::ivec2 max, std::vector<int> & vec)
{
	if(IsDirty()) Rebuild();

	if(m_nodes == nullptr)
		return;

	std::stack<int> stack;
	stack.push(0);

	while(stack.size())
	{
		auto & node = m_nodes[stack.top()];
		stack.pop();

		if(math::boxIntersects(min, max, node.min, node.max))
		{
			if(node.leaf == false)
			{
				stack.push(node.child+0);
				stack.push(node.child+1);
			}
			else
			{
				vec.push_back(node.child);
			}
		}
	}
}

void QuadTree::Prepare(Shaders * shaders)
{
	auto gl = shaders->gl;

	if(IsDirty()) Rebuild();

	if(m_vbo_dirty == false)
		return;

	m_vbo_dirty = false;

	if(m_vao == 0)
	{
        gl->glGenVertexArrays(1, &m_vao);
        gl->glGenBuffers(1, &m_vbo);

        gl->glBindVertexArray(m_vao);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        gl->glVertexAttribIPointer(0, 2, GL_SHORT, sizeof(EdgeVertex), (void*) offsetof(EdgeVertex, vertex));
        gl->glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(EdgeVertex), (void*) offsetof(EdgeVertex, type));
		gl->glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(EdgeVertex), (void*) offsetof(EdgeVertex, permeability));

        gl->glEnableVertexAttribArray(0);
        gl->glEnableVertexAttribArray(1);
		gl->glEnableVertexAttribArray(2);
	}

	auto array = GetRenderWalls();
	gl_size = array.size();

    gl->glBindVertexArray(0);
    gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	if(gl_alloced < array.capacity())
	{
		gl_alloced = array.capacity();
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(array[0]) * array.capacity(), &array[0], GL_DYNAMIC_DRAW);
	}
	else if(array.size() > 0)
	{
        gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(array[0]) * array.size(), &array[0]);
	}
}

void QuadTree::Render(Shaders * shaders, int selected_door_type)
{
	auto gl = shaders->gl;

	Prepare(shaders);

	if(gl_size > 0)
	{
        gl->glBindVertexArray(m_vao);
		shaders->doorShader.Bind(gl, selected_door_type);
        gl->glDrawArrays(GL_LINES, 0, gl_size);
	}
}

