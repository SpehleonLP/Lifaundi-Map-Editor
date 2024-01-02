#include "indexbuffers.h"
#include "src/glviewwidget.h"
#include <vector>

IndexBuffers::~IndexBuffers()
{
}

void IndexBuffers::Release(GLViewWidget*gl)
{
    gl->glDeleteBuffers(2, m_vbo);
}


void IndexBuffers::Prepare(GLViewWidget*gl, EntitySystem::Range range)
{
	m_allocated = std::max<uint32_t>((range.size()+32) & 0xFFFFFFE0, m_allocated);

	std::vector<uint32_t> data(m_allocated*8);
	auto * dst = data.data();
	//fill with edge data
	for(auto i : range)
	{
		*(dst++) = i*4+0;
		*(dst++) = i*4+1;
		*(dst++) = i*4+1;
		*(dst++) = i*4+2;
		*(dst++) = i*4+2;
		*(dst++) = i*4+3;
		*(dst++) = i*4+3;
		*(dst++) = i*4+0;
	}

	if(m_vbo[0] == 0)
	{
        gl->glGenBuffers(2, m_vbo);
        gl->glAssert();
	}

    gl->glBindVertexArray(0);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[0]);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * sizeof(uint32_t) * m_allocated, &data[0], GL_DYNAMIC_DRAW);
    gl->glAssert();

//fill with edge data
	for(auto i : range)
	{
		*(dst++) = i*4+0;
		*(dst++) = i*4+1;
		*(dst++) = i*4+2;
		*(dst++) = i*4+2;
		*(dst++) = i*4+1;
		*(dst++) = i*4+3;
	}

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[1]);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32_t) * m_allocated, &data[1], GL_DYNAMIC_DRAW);
    gl->glAssert();
}
