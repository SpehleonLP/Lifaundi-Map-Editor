#include "edgerange.h"
#include "metaroom.h"
#include "lf_math.h"

EdgeRange::EdgeRange(const QuadTree * tree, int edge, bool minimum) :
	EdgeRange(tree, tree->m_metaroom->GetVertex(edge), tree->m_metaroom->GetNextVertex(edge), minimum? edge : -1)
{
}

EdgeRange::EdgeRange(const QuadTree * tree, glm::ivec2 v0, glm::ivec2 v1, int minimum) :
	v0(v0),
	v1(v1),
	min(glm::min(v0, v1)),
	max(glm::max(v0, v1)),
	m_nodes(&tree->m_nodes[0]),
	m_metaroom(tree->m_metaroom),
	minimum(minimum)
{

	if(m_nodes != nullptr)
		stack.push(0);
}

bool EdgeRange::popFront()
{
top:
	if(child > -1)
	{
		while(++i < 4)
		{
			a0 = m_metaroom->GetVertex(child*4 + i);
			a1 = m_metaroom->GetVertex(child*4 + (i+1) % 4);

			if(a0 != a1
			&& math::IsOpposite(a1- a0, v1- v0) == false
			&& math::IsColinear(v0, v1, a0, a1) == false)
				return true;
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

		if(minimum > 0 && (uint32_t)minimum < node.child)
		{
			child = node.child;
			i     = -1;
			goto top;
		}
	}

	return false;
}

