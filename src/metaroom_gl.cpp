#include "metaroom_gl.h"
#include "metaroom.h"
#include "Shaders/roomoutlineshader.h"
#include "Shaders/selectedroomshader.h"
#include "Shaders/arrowshader.h"
#include "src/Shaders/shaders.h"
#include "document.h"
#include "src/lf_math.h"
#include <QOpenGLFunctions_4_5_Core>
#include <glm/packing.hpp>

MetaroomGL::MetaroomGL(Metaroom & me) : me(me)
{
}

MetaroomGL::~MetaroomGL()
{
}

void MetaroomGL::Release(Shaders * shaders)
{
	auto gl = shaders->gl;

	_indices.Release(shaders);
	me._selection.Release(shaders);

	gl->glDeleteVertexArrays(2, _vao);
	gl->glDeleteBuffers(1, &_buffers);
}

void MetaroomGL::Render(Shaders * shaders, int selected_door_type)
{
	if(me.empty())
		return;

	auto gl = shaders->gl;

	Prepare(shaders);

	if(me._selection.NoSelectedFaces() > 0)
	{
		gl->glBindVertexArray(_vao[1]);
		shaders->selectedRoomShader(gl, me._selection.NoSelectedFaces());
	}

	gl->glBindVertexArray(_vao[0]);
	gl->glLineWidth(3);
	shaders->roomOutlineShader(gl, me.noFaces(), true);
	gl->glLineWidth(1);
	shaders->roomOutlineShader(gl, me.noFaces(), false);

	me._tree.Render(shaders, selected_door_type);

	if(_arrows.size() && (me.document->m_window->viewGravity() ||  me.document->m_window->viewHalls()))
	{
		shaders->arrowShader(gl, _arrows, glm::vec4(0, 0, 1, 1));
	}
}

void MetaroomGL::Prepare(Shaders * shaders)
{
	me._selection.Prepare(shaders);

	if(_dirty == false || me.noFaces() == 0)
		return;

	_dirty = false;
	bool initialized_vaos = (_vao[0] != 0L);

	if(_indicesDirty)
	{
		_indicesDirty = false;
		_indices.Prepare(shaders, me.range());
	}

	auto gl = shaders->gl;

	if(initialized_vaos == false)
	{
		gl->glGenVertexArrays(2, _vao);
		gl->glGenBuffers(1, &_buffers);

		gl->glBindVertexArray(_vao[0]);
		gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers);
		gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(glm::ivec2), 0);
		gl->glEnableVertexAttribArray(0);

		gl->glBindBuffer(GL_ARRAY_BUFFER, me._selection.GetBuffer());
		gl->glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), 0);
		gl->glEnableVertexAttribArray(1);

		gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices.EdgeVBO());

// create selected face shader
		gl->glBindVertexArray(_vao[1]);
		gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers);
		gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(glm::ivec2), 0);
		gl->glEnableVertexAttribArray(0);

		gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, me._selection.GetIndices());
	}

	gl->glBindVertexArray(0);
	gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers);
	gl->glBufferData(GL_ARRAY_BUFFER, me._verts.byteLength(), me._verts.data(), GL_DYNAMIC_DRAW);
	

	if(me.document->m_window->viewGravity())
	{
		_arrows.resize(me.noFaces());

		for(auto i : me.range())
		{
			_arrows[i].position = (me.GetVertex(i, 0) + me.GetVertex(i, 1) + me.GetVertex(i, 2) + me.GetVertex(i, 3))/4;
			_arrows[i].rotation = (float)SHRT_MAX * glm::normalize(glm::unpackHalf2x16(me._gravity[i]));
		}
	}
	else
	{
		_arrows.clear();
		Arrow buffer;
		auto range = me.range();

		for(auto i = range.begin(), next = i; i != range.end(); i = next)
		{
			if(++next == range.end())
				continue;

			for(int j = 0; j < 16; ++j)
			{
				const int a = (j % 4);
				const int b = (j / 4);

				const auto a0 = me.GetVertex(*i, a);
				const auto a1 = me.GetVertex(*i, a+1);
				const auto b0 = me.GetVertex(*next, b);
				const auto b1 = me.GetVertex(*next, b+1);

				if(a0 == a1
				|| b0 == b1
	//			|| GetDoorType(i-1, a) != 0
	//			|| GetDoorType(i  , b) != 0
				|| math::IsOpposite(a1 - a0, b1 - b0) == false
				|| math::IsColinear(a0, a1, b0, b1) == false)
					continue;

				std::pair<float, int> begin, end;

				if(math::GetOverlap(a0, a1, b0, b1, &begin, &end))
				{
					float avg = (begin.first + end.first) * .5f;

					glm::vec2 vec = a1 - a0;

					buffer.position = a0 + glm::ivec2(vec * avg);

					vec = glm::normalize(vec);
					buffer.rotation.x = -SHRT_MAX * vec.y;
					buffer.rotation.y = -SHRT_MAX * vec.x;

					_arrows.push_back(buffer);
				}
			}
		}
	}
}
