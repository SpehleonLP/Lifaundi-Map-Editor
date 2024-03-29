#ifndef QUADTREE_H
#define QUADTREE_H
#include "metaroomdoors.h"
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>
#include <stack>
#include <vector>
#include <memory>

class Metaroom;
class Shaders;
class EdgeRange;

class QuadTree
{
public:
    QuadTree(Metaroom * room);
	~QuadTree();

    void Release(Shaders * shaders);

	struct Node
	{
		int         leaf : 1;
		int			child : 31;
		glm::i16vec2 min;
		glm::i16vec2 max;
	};

	struct Leaf
	{
		uint32_t    z_order;
		int         face_id;
		glm::i16vec2 min;
		glm::i16vec2 max;
	};

	struct EdgeVertex
	{
		glm::i16vec2 vertex;
		uint8_t      permeability;
		uint8_t      type;
		uint16_t     padding;
	};

//format....
//	list of indices
//  float end_point
//  int   face

	struct DoorList
	{
		uint32_t index;
		uint8_t  length;
		 int8_t  min_perm;
		 int8_t  max_perm;
		 uint8_t  padding{};

		 uint32_t end() const { return index+length; }
	};

	struct Door
	{
		int   perm : 8;
		int   face : 24;
		float end_point;
	};

	struct DoorInfo
	{
		float begin;
		float end;
		int   perm : 8;
		int   face : 24;
	};

	static std::vector<Node> CreateNodes(glm::i16vec4 bounds, glm::i16vec4 const* boxes, uint32_t size);

	glm::vec4 GetDimensions() const { return m_nodes? glm::vec4(m_nodes[0].min,m_nodes[0].max) : glm::vec4(0); }
	void ReadTree(std::ifstream & fp);
	void WriteTree(std::ofstream & fp);

    void Prepare(Shaders * shaders);
    void Render(Shaders * shaders, int selected_door_type);

	void GetWriteDoors(std::vector<Door> & doors, std::vector<DoorList> & indices, bool make_halls = false, int typeId = -1);
	void GetHallDoors(std::vector<int> & doors, std::vector<std::pair<int, int>> & indices);
	std::vector<uint8_t> GetEdgeFlags();

	int GetFace(glm::ivec2 position);

	bool IsDirty() const { return m_dirty; }
	void SetDirty(bool = true) { m_dirty = true; m_mvsf.clear(); }
	void SetVBODirty() { m_vbo_dirty = true; }
	bool empty() const;

	bool DoesOverlap(glm::ivec2 min, glm::ivec2 max);

	struct VertexQuery
	{
		uint32_t face : 30;
		uint32_t edge : 2;
	};

	void GetOverlappingRooms(glm::ivec2 min, glm::ivec2 max, std::vector<int> & vec);
	void GetRoomsWithVertex(int face, int edge, std::vector<VertexQuery> & vec);

	std::vector<uint32_t> GetOverlappingEdges(glm::ivec2 v0, glm::ivec2 v1);
	bool                  HasOverlappingEdges(int v);
	bool                  HasFullOverlap(int v);

	bool GetSliceFace(const glm::ivec2 v0, const glm::ivec2 v1, glm::ivec2 v2, int & edge_id, float & mid);

	void PrintTree(int node, int tabs) const;

private:
friend class RoomRange;
	std::vector<EdgeVertex> GetRenderWalls();

	void GetWriteDoors(int edge, std::stack<int> & stack, std::vector<DoorInfo> & edges, int typeId);
	void GetHallDoors(int edge, std::vector<int> & doors);
	void GetRenderWalls(int edge, std::vector<EdgeVertex> & edges) const;

	void     Rebuild();
	static uint32_t BuildTree(Node * nodes, Leaf const* leaves, uint32_t min, uint32_t max, uint32_t mask, uint32_t node, uint32_t base, uint32_t alloc, uint32_t depth = 0);


	Metaroom              * m_metaroom{nullptr};
	std::unique_ptr<Node[]> m_nodes;

	bool m_dirty{true};
	bool m_vbo_dirty{true};

	uint32_t m_vao{};
	uint32_t m_vbo{};
	uint32_t gl_alloced{};
	uint32_t gl_size{};
    uint32_t alloced{};

	std::vector<Node> m_mvsf;
};

#endif // QUADTREE_H
