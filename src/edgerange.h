#ifndef EDGERANGE_H
#define EDGERANGE_H
#include "quadtree.h"

class EdgeRange
{
public:
	EdgeRange(const QuadTree * tree, int edge, bool minimum = -1);
	EdgeRange(const QuadTree * tree, glm::ivec2 v0, glm::ivec2 v1, int minimum = -1);

	int  face() const { return child; }
	int  edge() const { return child*4 + i; }
	int  nextEdge() const { return child*4 + (i+1)%4; }
	int  index() const { return i; }
	glm::ivec2 vec() const { return v1 - v0; }
	bool popFront();

	const glm::ivec2  v0, v1;
	const glm::ivec2  min, max;
	glm::ivec2 a0, a1;

private:
	std::stack<int> stack;

	const QuadTree::Node * m_nodes;
	const Metaroom       * m_metaroom;
	int i{};
	int child{-1};
	const int minimum{-1};
};

#endif // EDGERANGE_H
