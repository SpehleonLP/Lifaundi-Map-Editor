#include "metaroomdoors.h"
#include "metaroomselection.h"
#include "metaroom.h"
#include "Shaders/doorshader.h"
#include <algorithm>

MetaroomDoors::MetaroomDoors(GLViewWidget *gl)
{
	DoorShader::Shader.AddRef();
}

MetaroomDoors::~MetaroomDoors()
{
}

void MetaroomDoors::Release(GLViewWidget * gl)
{
	DoorShader::Shader.Release(gl);

    gl->glDeleteVertexArrays(1, &m_vao);
    gl->glDeleteBuffers(1, &m_vbo);

}

size_t MetaroomDoors::RemoveFaces(std::vector<int> const& vec)
{
	m_dirty = true;

	size_t accumulator = 0;

	for(int i : vec)
		accumulator += RemoveFace(i);

	return accumulator;
}

size_t MetaroomDoors::RemoveFace(int i)
{
	m_dirty = true;

	uint32_t read  = 0;
	uint32_t write = 0;

	for(; read < m_edges.size(); ++read)
	{
		if(m_edges[read].m[0].face != i && m_edges[read].m[1].face != i)
		{
			m_edges[read].m[0].face -= (m_edges[read].m[0].face > i);
			m_edges[read].m[1].face -= (m_edges[read].m[1].face > i);
			m_edges[write++] = m_edges[read];
		}
	}

	m_edges.resize(write);
	return read - write;
}


size_t MetaroomDoors::RemoveEdges(std::vector<int> const& vec)
{
	m_dirty = true;

	uint32_t read = 0;
	uint32_t write = 0;

	for(int i : vec)
	{
		while(read < m_edges.size() && m_edges[read].m[0].face < i)
		{
			int face = m_edges[read].m[0].face;

			uint32_t j = 0;

			for(; read < m_edges.size() && m_edges[read].m[0].face == face; )
			{
				while(j != vec.size() && m_edges[read].m[1].face > vec[j])
				{
					++j;
				}

				if(j == vec.size() || m_edges[read].m[1].face < vec[j])
				{
					m_edges[write] = m_edges[read];
					++read;
					++write;
				}
			}
		}

		while(read < m_edges.size() && m_edges[read].m[0].face == i) ++read;
	}

	while(read < m_edges.size() && m_edges[read].m[0].face)
	{
		int face = m_edges[read].m[0].face;

		uint32_t j = 0;

		for(; read < m_edges.size() && m_edges[read].m[0].face == face; )
		{
			while(j != vec.size() && m_edges[read].m[1].face > vec[j])
			{
				++j;
			}

			if(j == vec.size() || m_edges[read].m[1].face < vec[j])
			{
				m_edges[write] = m_edges[read];
				++read;
				++write;
			}
		}
	}

	m_edges.resize(write);
	return read - write;
}

void MetaroomDoors::UpdateDoors(Metaroom * metaroom)
{
	m_dirty = true;

	auto selection = metaroom->m_selection.GetVertSelection();
	std::vector<Edge> edges;

//	for(int i : selection)
//		metaroom->m_tree.GetAdjacent(i, edges);

	std::sort(edges.begin(), edges.end());

	auto i = edges.begin();
	auto j = m_edges.begin();

	while(i != edges.end() && j != m_edges.end())
	{
		if     (i < j) ++i;
		else if(j < i) ++j;
		else
		{
			i->m[0].type = j->m[0].type;
			i->m[0].perm = j->m[0].perm;
			i->m[1].type = j->m[1].type;
			i->m[1].perm = j->m[1].perm;

			++i;
			++j;
		}
	}

	RemoveEdges(selection);
	AddEdges(edges);
}

void MetaroomDoors::AddEdges(std::vector<Edge> & e)
{
	m_dirty = true;

	m_edges.resize(m_edges.size() + e.size());

	auto i = m_edges.rbegin()+e.size();
	auto j = e.rbegin();

	auto insertion = m_edges.rbegin();

	for(; i > m_edges.rend() && j > e.rend(); ++insertion)
	{
		if(*i < *j)	*insertion = *(j++);
		else        *insertion = *(i++);
	}

	for(;i > m_edges.rend(); ++i, ++insertion)
		*insertion = *i;

	for(;i > m_edges.rend(); ++i, ++insertion)
		*insertion = *i;
}

void MetaroomDoors::Prerender(GLViewWidget *gl)
{
	if(m_dirty == false)
		return;

	m_dirty = false;

	if(m_vao == 0)
	{
        gl->glGenVertexArrays(1, &m_vao);
        gl->glGenBuffers(1, &m_vbo);

        gl->glBindVertexArray(m_vao);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        gl->glVertexAttribIPointer(0, 1, GL_INT, sizeof(Edge::Vertex), (void*) offsetof(Edge::Vertex, id));
        gl->glVertexAttribIPointer(1, 1, GL_INT, sizeof(Edge::Vertex), (void*) offsetof(Edge::Vertex, face));
        gl->glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(Edge::Vertex), (void*) offsetof(Edge::Vertex, type));
        gl->glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(Edge::Vertex), (void*) offsetof(Edge::Vertex, perm));

        gl->glEnableVertexAttribArray(0);
        gl->glEnableVertexAttribArray(1);
	}

    gl->glBindVertexArray(0);
    gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	if(m_alloc < m_edges.capacity())
	{
		m_alloc = m_edges.capacity();
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(m_edges[0]) * m_edges.size(), &m_edges[0], GL_DYNAMIC_DRAW);
	}
	else if(m_edges.size() > 0)
	{
        gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_edges[0]) * m_edges.size(), &m_edges[0]);
	}
}
