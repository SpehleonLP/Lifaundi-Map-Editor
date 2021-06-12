#include "edgerange.h"
#include "metaroom.h"
#include "lf_math.h"
#include "glm_iostream.h"
#include <iostream>
#include <cfloat>

EdgeRange::EdgeRange(const QuadTree * tree, int edge, bool minimum) :
	EdgeRange(tree, tree->m_metaroom->GetVertex(edge), tree->m_metaroom->GetNextVertex(edge), minimum? edge/4 : -1)
{
}

EdgeRange::EdgeRange(const QuadTree * tree, glm::ivec2 v0, glm::ivec2 v1,  glm::ivec2 v2, int minimum) :
	v0(v0),
	v1(v1),
	min(glm::min(v0, v2)),
	max(glm::max(v0, v2)),
	m_nodes(&tree->m_nodes[0]),
	m_metaroom(tree->m_metaroom),
	minimum(minimum)
{

	if(m_nodes != nullptr)
		stack.push(0);
}


bool EdgeRange::popFront(Flags f)
{
top:
	if(child > -1)
	{
		if(f == Flags::None && i == -1)
		{
			i = 0;
			return true;
		}
		else if(f == Flags::Colinear)
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
		}

		child = -1;
	}

	while(stack.size())
	{
		auto & node = m_nodes[stack.top()];
		stack.pop();

		if(math::intersects(min, max, node.min, node.max) == false)
			continue;

		if(node.leaf == false)
		{
			stack.push(node.child+0);
			stack.push(node.child+1);
			continue;
		}

		if(minimum < 0 || (uint32_t)minimum < node.child)
		{
			child = node.child;
			i     = -1;
			goto top;
		}
	}

	return false;
}

