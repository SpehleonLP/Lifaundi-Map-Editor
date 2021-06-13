#ifndef ROOMRANGE_H
#define ROOMRANGE_H
#include "quadtree.h"

class RoomRange
{
public:
	RoomRange(const QuadTree * tree, int edge, bool minimum = false);
	RoomRange(const QuadTree * tree, glm::ivec2 v0, glm::ivec2 v1, int minimum = -1);

	int  face() const { return child; }
	bool popFront();

	const glm::ivec2  min, max;

protected:
	std::stack<int> stack;

	const QuadTree::Node * m_nodes;
	const Metaroom       * m_metaroom;
	int child{-1};
	const int minimum{-1};
};

#endif // EDGERANGE_H
