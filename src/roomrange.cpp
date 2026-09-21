#include "roomrange.h"
#include "metaroom.h"
#include "lf_math.h"
#include "glm_iostream.h"
#include <iostream>
#include <cfloat>

RoomRange::RoomRange(const QuadTree * tree, int edge, bool minimum) :
	RoomRange(tree, tree->m_metaroom->GetVertex(edge), tree->m_metaroom->GetNextVertex(edge), minimum? edge/4 : -1)
{
}

// Every QuadTree query rebuilds a dirty tree first; a range must too, or it
// walks nodes from before the last edit. The tree is a mutable cache
// (Metaroom::_tree), so rebuilding through a const pointer is sound.
const QuadTree::Node * RoomRange::BuiltNodes(const QuadTree * tree)
{
	if(tree->IsDirty())
		const_cast<QuadTree*>(tree)->Rebuild();

	return tree->m_nodes.get();
}

RoomRange::RoomRange(const QuadTree * tree, glm::ivec2 v0, glm::ivec2 v1, int minimum) :
	min(glm::min(v0, v1)),
	max(glm::max(v0, v1)),
	m_nodes(BuiltNodes(tree)),
	m_metaroom(tree->m_metaroom),
	minimum(minimum)
{

	if(m_nodes != nullptr)
		stack.push(0);
}


bool RoomRange::popFront()
{
	while(stack.size())
	{
		auto & node = m_nodes[stack.top()];
		stack.pop();

		if(math::boxIntersects(min, max, node.min, node.max) == false)
			continue;

		if(node.leaf == false)
		{
			stack.push(node.child+0);
			stack.push(node.child+1);
			continue;
		}

		if(minimum < 0 || minimum < node.child)
		{
			_face = node.child;
			return true;
		}
	}

	return false;
}

