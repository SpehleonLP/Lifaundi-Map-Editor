#include "metaroomselection.h"
#include <algorithm>
#include <cstring>
#include <cassert>
#include "src/glviewwidget.h"

#define GL_ASSERT gl->glAssert()

MetaroomSelection::~MetaroomSelection()
{
}

void MetaroomSelection::Release(GLViewWidget *gl)
{
    gl->glDeleteBuffers(2, m_vbo);
}

void MetaroomSelection::resize(size_t new_size)
{
	if(new_size <= m_alloced)
		return;

	std::unique_ptr<uint8_t[]> r(new uint8_t[new_size*4]);

	if(m_array != nullptr)
		memcpy(&r[0], &m_array[0], std::min<size_t>(m_alloced, new_size)*4);

	m_array    = std::move(r);
	m_alloced  = new_size;
}

void MetaroomSelection::erase(size_t i)
{
	memmove(&m_array[i*4], &m_array[(i+1)*4], 4 * (m_alloced - i));
	memset(&m_array[(m_alloced - (i+1))*4], 0, 4);
}

void MetaroomSelection::erase(std::vector<int> const& vec)
{
	uint32_t read = 0, write = 0;

	for(int i : vec)
	{
		assert(read < m_alloced);

		uint32_t copy_length = i - read;

		if(copy_length == 0)
		{
			++read;
			continue;
		}

		memmove(&m_array[write*4], &m_array[read*4], copy_length);

		read  += copy_length+1;
		write += copy_length;
	}

	assert((read - write) <= vec.size());
}

void MetaroomSelection::clear()
{
	m_selectionChanged = true;
	memset(&m_array[0], 0, m_alloced*4);
}

void MetaroomSelection::begin_and()
{
	for(uint32_t i = 0; i < m_alloced*4; ++i)
		m_array[i] <<= 1;
}

void MetaroomSelection::end_and()
{
	for(uint32_t i = 0; i < m_alloced*4; ++i)
		m_array[i] = ((m_array[i] >> 1) & m_array[i]) & 0x01;
}


bool MetaroomSelection::MarkFace(int id)
{
	if(m_array[id] & 0x02)
		return false;

	m_array[id] |= 0x02;
	return true;
}

void MetaroomSelection::ClearMarks()
{
	for(uint32_t i = 0; i < m_alloced; ++i)
	{
		m_array[i] &= 0x01;
	}
}

std::vector<int> MetaroomSelection::GetVertSelection() const
{
	std::vector<int> r;

	for(uint32_t i = 0; i < m_alloced*4; ++i)
	{
		if(IsVertSelected(i))
			r.push_back(i);
	}

	return r;
}

std::vector<int> MetaroomSelection::GetFaceSelection() const
{
	std::vector<int> r;

	for(uint32_t i = 0; i < m_alloced; ++i)
	{
		if(IsFaceSelected(i))
			r.push_back(i);
	}

	return r;
}

std::vector<int> MetaroomSelection::GetSelection() const
{
	std::vector<int> r;

	for(uint32_t i = 0; i < m_alloced; ++i)
	{
		if(IsSelected(i))
			r.push_back(i);
	}

	return r;
}

std::vector<int> MetaroomSelection::GetEdgeSelection() const
{
	std::vector<int> r;

	for(uint32_t i = 0; i < m_alloced*4; ++i)
	{
		if(IsEdgeSelected(i))
			r.push_back(i);
	}

	return r;
}

void MetaroomSelection::SetSelection(std::vector<int> const& vec)
{
	clear();

	for(int i : vec)
	{
		select_face(i, Bitwise::SET);
	}
}

void MetaroomSelection::SetVertexSelection(std::vector<int> const& vec)
{
	for(int i : vec)
	{
		select_vertex(i, Bitwise::SET);
	}
}

bool MetaroomSelection::ToggleSelectAll(uint32_t faces, Bitwise flags)
{
	m_selectionChanged = true;

	bool changed = false;

    for(uint32_t i = 0; i < faces*4; ++i)
	{
		bool is_selected = IsVertSelected(i);
		select_vertex(i, flags);
		changed |= (IsVertSelected(i) != is_selected);
	}

	if(changed == false)
		clear();

	m_selectedFaces = faces;
	return changed;
}

void MetaroomSelection::select_vertex(int id, Bitwise flags)
{
	m_selectionChanged = true;

	switch(flags)
	{
	case Bitwise::SET:  m_array[id]  =  1;	break;
	case Bitwise::AND:	m_array[id] |=  1;	break;
	case Bitwise::OR:   m_array[id]  =  1;	break;
	case Bitwise::XOR:  m_array[id] ^=  1;	break;
	case Bitwise::NOT:	m_array[id] &=  0xFE;	break;
	}
}


void MetaroomSelection::select_face  (int id, Bitwise flags)
{
	select_vertex(id*4+0, flags);
	select_vertex(id*4+1, flags);
	select_vertex(id*4+2, flags);
	select_vertex(id*4+3, flags);
}

void MetaroomSelection::select_edge  (int id, Bitwise flags)
{
	select_vertex(id, flags);
	select_vertex((id & 0xFFFFFFFC) + (id+1) % 4, flags);
}

void MetaroomSelection::MergeSelection(MetaroomSelection const& it, Bitwise flags)
{
	int N = std::min(it.size(), size())*4;

	for(int i = 0; i < N; ++i)
	{
		switch(flags)
		{
		case Bitwise::SET:  m_array[i]  =  it.m_array[i]; break;
		case Bitwise::AND:	m_array[i] &=  it.m_array[i]; break;
		case Bitwise::OR:   m_array[i] |=  it.m_array[i]; break;
		case Bitwise::XOR:  m_array[i] ^=  it.m_array[i]; break;
		case Bitwise::NOT:	m_array[i] &= ~it.m_array[i]; break;
		}

	}
}

void MetaroomSelection::Prepare(GLViewWidget*gl)
{

	if(m_selectionChanged == false)
	{
		return;
	}

	m_selectionChanged = false;

	if(m_vbo[0] == 0)
	{
        gl->glGenBuffers(2, m_vbo); GL_ASSERT;
	}

    gl->glBindVertexArray(0);
    gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]); GL_ASSERT;
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[1]); GL_ASSERT;

	if(m_glalloc < m_alloced)
	{
		m_glalloc = m_alloced;
        gl->glBufferData(GL_ARRAY_BUFFER, m_alloced*4, &m_array[0], GL_DYNAMIC_DRAW);  GL_ASSERT;
        gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_alloced * 24, nullptr, GL_DYNAMIC_DRAW);  GL_ASSERT;
	}
	else if(m_alloced > 0)
	{
        gl->glBufferSubData(GL_ARRAY_BUFFER, 0, m_alloced*4, &m_array[0]); GL_ASSERT;
	}

	std::vector<uint32_t> m_indices;
	m_indices.reserve(m_alloced*6);

	for(uint32_t i = 0; i < m_alloced; ++i)
	{
		if(IsFaceSelected(i))
		{
			m_indices.push_back(i*4+0);
			m_indices.push_back(i*4+1);
			m_indices.push_back(i*4+2);
			m_indices.push_back(i*4+0);
			m_indices.push_back(i*4+2);
			m_indices.push_back(i*4+3);
		}
	}

	if(m_indices.size() > 0)
	{
        gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_indices.size() * sizeof(m_indices[0]), &m_indices[0]); GL_ASSERT;
	}

	m_selectedFaces = m_indices.size()/6;
	m_selectedEdges = 0;

	for(uint32_t i = 0; i < m_alloced*4; ++i)
		m_selectedEdges += IsEdgeSelected(i);
}





