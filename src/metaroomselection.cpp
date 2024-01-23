#include "metaroomselection.h"
#include <algorithm>
#include <cstring>
#include <cassert>
#include "qopenglfunctions_4_5_core.h"
#include "src/Shaders/shaders.h"

#define GL_ASSERT gl->glAssert()

MetaroomSelection::~MetaroomSelection()
{
}

void MetaroomSelection::Release(Shaders * shaders)
{
	auto gl = shaders->gl;
    gl->glDeleteBuffers(2, m_vbo);
}

void MetaroomSelection::resize(size_t new_size)
{
	if(new_size <= _selection.size())
		return;

	auto tmp = shared_array<std::array<uint8_t, 4>>(new_size, std::array<uint8_t, 4>{0});
	memcpy(tmp.data(), _selection.data(), _selection.byteLength());
	_selection = tmp;
}

void MetaroomSelection::erase(size_t i)
{
	memset(&_selection[i], 0, sizeof(_selection[0]));
}

void MetaroomSelection::erase(std::vector<uint32_t> const& vec)
{
	for(int i : vec)
	{
		erase(i);
	}
}

void MetaroomSelection::clear()
{
	m_selectionChanged = true;
	memset(&_selection[0], 0, _selection.byteLength());
}

void MetaroomSelection::begin_and()
{
	for(uint32_t i = 0; i < _selection.size()*4; ++i)
		vertex(i) <<= 1;
}

void MetaroomSelection::end_and()
{
	for(uint32_t i = 0; i < _selection.size()*4; ++i)
		vertex(i) = ((vertex(i) >> 1) & vertex(i)) & 0x01;
}


bool MetaroomSelection::MarkFace(int i)
{
	bool mark = (vertex(i) & 0x02);
	vertex(i) |= 0x02;
	return !mark;
}

void MetaroomSelection::ClearMarks()
{
	for(uint32_t i = 0; i < _selection.size(); ++i)
	{
		vertex(i) &= 0x01;
	}
}

std::vector<uint32_t> MetaroomSelection::GetVertSelection() const
{
	std::vector<uint32_t> r;

	for(uint32_t i = 0; i < _selection.size()*4; ++i)
	{
		if(IsVertSelected(i))
			r.push_back(i);
	}

	return r;
}

std::vector<uint32_t> MetaroomSelection::GetFaceSelection() const
{
	std::vector<uint32_t> r;

	for(uint32_t i = 0; i < _selection.size(); ++i)
	{
		if(IsFaceSelected(i))
			r.push_back(i);
	}

	return r;
}

std::vector<uint32_t> MetaroomSelection::GetSelection() const
{
	std::vector<uint32_t> r;

	for(uint32_t i = 0; i < _selection.size(); ++i)
	{
		if(IsAnyCornerSelected(i))
			r.push_back(i);
	}

	return r;
}

std::vector<uint32_t> MetaroomSelection::GetEdgeSelection() const
{
	std::vector<uint32_t> r;

	for(uint32_t i = 0; i < _selection.size()*4; ++i)
	{
		if(IsEdgeSelected(i))
			r.push_back(i);
	}

	return r;
}

void MetaroomSelection::SetSelection(std::vector<uint32_t> const& vec)
{
	clear();

	for(int i : vec)
	{
		select_face(i, Bitwise::SET);
	}
}

void MetaroomSelection::SetVertexSelection(std::vector<uint32_t> const& vec)
{
	for(int i : vec)
	{
		select_vertex(i, Bitwise::SET);
	}
}

bool MetaroomSelection::ToggleSelectAll(Range range, Bitwise flags)
{
	m_selectionChanged = true;

	bool changed = false;

	for(auto i : range)
	{
		for(int corner = 0; corner < 4; ++corner)
		{
			auto j = i*4+corner;

			bool is_selected = IsVertSelected(j);
			select_vertex(j, flags);
			changed |= (IsVertSelected(j) != is_selected);
		}
	}

	if(changed == false)
		clear();

	m_selectedFaces = range.size();
	return changed;
}

void MetaroomSelection::select_vertex(int id, Bitwise flags)
{
	m_selectionChanged = true;

	switch(flags)
	{
	case Bitwise::SET:  vertex(id)  =  1;	break;
	case Bitwise::AND:	vertex(id) |=  1;	break;
	case Bitwise::OR:   vertex(id) |=  1;	break;
	case Bitwise::XOR:  vertex(id) ^=  1;	break;
	case Bitwise::NOT:	vertex(id) &=  0xFE;	break;
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
		case Bitwise::SET:  vertex(i)  =  it.vertex(i); break;
		case Bitwise::AND:	vertex(i) &=  it.vertex(i); break;
		case Bitwise::OR:   vertex(i) |=  it.vertex(i); break;
		case Bitwise::XOR:  vertex(i) ^=  it.vertex(i); break;
		case Bitwise::NOT:	vertex(i) &= ~it.vertex(i); break;
		}

	}
}

void MetaroomSelection::Prepare(Shaders * shaders)
{
	auto gl = shaders->gl;

	if(m_selectionChanged == false)
	{
		return;
	}

	m_selectionChanged = false;

	if(m_vbo[0] == 0)
	{
		gl->glGenBuffers(2, m_vbo);
	}

    gl->glBindVertexArray(0);
	gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[1]);

	if(m_glalloc < _selection.size())
	{
		m_glalloc = _selection.size();
		gl->glBufferData(GL_ARRAY_BUFFER, _selection.byteLength(), data(), GL_DYNAMIC_DRAW);
		gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, _selection.size() * 24, nullptr, GL_DYNAMIC_DRAW);
	}
	else if(_selection.size() > 0)
	{
		gl->glBufferSubData(GL_ARRAY_BUFFER, 0, _selection.byteLength(), data());
	}

	std::vector<uint32_t> m_indices;
	m_indices.reserve(_selection.size()*6);

	for(uint32_t i = 0; i < _selection.size(); ++i)
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
		gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_indices.size() * sizeof(m_indices[0]), &m_indices[0]);
	}

	m_selectedFaces = m_indices.size()/6;
	m_selectedEdges = 0;

	for(uint32_t i = 0; i < _selection.size()*4; ++i)
		m_selectedEdges += IsEdgeSelected(i);
}





