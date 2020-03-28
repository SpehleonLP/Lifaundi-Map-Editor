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


void IndexBuffers::Prepare(GLViewWidget*gl, uint32_t size)
{
	if(size < m_allocated)
		return;

	m_allocated = (size+32) & 0xFFFFFFE0;

	std::vector<uint32_t> data(m_allocated*8);

	//fill with edge data
	for(uint32_t i = 0; i < m_allocated; ++i)
	{
		data[i*8+0] = i*4+0;
		data[i*8+1] = i*4+1;
		data[i*8+2] = i*4+1;
		data[i*8+3] = i*4+2;
		data[i*8+4] = i*4+2;
		data[i*8+5] = i*4+3;
		data[i*8+6] = i*4+3;
		data[i*8+7] = i*4+0;
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
	for(uint32_t i = 0; i < m_allocated; ++i)
	{
		data[i*6+0] = i*4+0;
		data[i*6+1] = i*4+1;
		data[i*6+2] = i*4+2;
		data[i*6+3] = i*4+2;
		data[i*6+4] = i*4+1;
		data[i*6+5] = i*4+3;
	}

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[1]);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32_t) * m_allocated, &data[1], GL_DYNAMIC_DRAW);
    gl->glAssert();
}
