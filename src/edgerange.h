#ifndef EDGERANGE_H
#define EDGERANGE_H
#include "quadtree.h"
#include "roomrange.h"

class EdgeRange : protected RoomRange
{
public:
	EdgeRange(const QuadTree * tree, int edge, bool minimum = false);
	EdgeRange(const QuadTree * tree, glm::ivec2 v0, glm::ivec2 v1, int minimum = -1) :
		EdgeRange(tree, v0, v1, v1, minimum) {};
	EdgeRange(const QuadTree * tree, glm::ivec2 v0, glm::ivec2 v1, glm::ivec2 v2, int minimum = -1);

	int  face() const { return child; }
	int  edge() const { return child*4 + i; }
	int  nextEdge() const { return child*4 + (i+1)%4; }
	int  index() const { return i; }
	glm::ivec2 vec() const { return v1 - v0; }
	bool popFront();

	const glm::ivec2  v0, v1;
	glm::ivec2 a0, a1;

private:
	int i{-1};
};

#endif // EDGERANGE_H
