#include "edgerange.h"
#include "metaroom.h"
#include "lf_math.h"
#include "glm_iostream.h"
#include <iostream>
#include <cfloat>

EdgeRange::EdgeRange(const QuadTree * tree, int edge, bool minimum) :
	RoomRange(tree, edge, minimum),
	v0(m_metaroom->GetVertex(edge)),
	v1(m_metaroom->GetNextVertex(edge))
{
}

EdgeRange::EdgeRange(const QuadTree * tree, glm::ivec2 v0, glm::ivec2 v1,  glm::ivec2 v2, int minimum) :
	RoomRange(tree, v0, v2, minimum),
	v0(v0),
	v1(v1)
{

	if(m_nodes != nullptr)
		stack.push(0);
}

bool EdgeRange::popFront()
{
	do {
		if(child > -1)
		{
			while(++i < 4)
			{
				a0 = m_metaroom->GetVertex(edge());
				a1 = m_metaroom->GetNextVertex(edge());

				if(a0 != a1
				&& math::IsOpposite(a1- a0, v1- v0)
				&& math::IsColinear(v0, v1, a0, a1))
					return true;
			}

			child = -1;
		}

		i     = -1;
	} while(RoomRange::popFront());

	return false;
}

