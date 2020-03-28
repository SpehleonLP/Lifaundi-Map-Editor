#ifndef MetaroomDoors_H
#define MetaroomDoors_H
#include "src/glviewwidget.h"
#include <vector>
#include <cstdint>

struct Edge
{
	struct Vertex
	{
		int id;
		int face;
		uint8_t type;
		uint8_t perm;
	} m[2];

	bool operator<(Edge const& it)
	{
		uint8_t f0 = m[0].face < it.m[0].face;
		uint8_t f1 = m[1].face < it.m[1].face;
		return f0 | (f1 & ~f0);
	}
};

class Metaroom;

class MetaroomDoors
{
public:
    MetaroomDoors(GLViewWidget * gl);
	~MetaroomDoors();

	void UpdateDoors(Metaroom * metaroom);
	void Release(GLViewWidget * gl);

	void AddEdges(std::vector<Edge> & e);

	size_t RemoveEdges(std::vector<int> const& vec);
	size_t RemoveFaces(std::vector<int> const& vec);
	size_t RemoveFace(int i);

    void   Prerender(GLViewWidget *gl);
	void   Render(uint32_t vertex_texture);

    std::vector<Edge> m_edges;

public:
	uint32_t m_vao{};
	uint32_t m_vbo{};
	uint32_t m_alloc{};

	bool     m_dirty{false};
};

#endif // LINKS_H
