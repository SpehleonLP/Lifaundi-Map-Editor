#include "metaroom_gl.h"
#include "metaroom.h"
#include "Shaders/roomoutlineshader.h"
#include "Shaders/selectedroomshader.h"
#include "Shaders/arrowshader.h"
#include "src/glviewwidget.h"
#include "document.h"
#include "src/lf_math.h"
#include <glm/packing.hpp>

MetaroomGL::MetaroomGL(Metaroom & me) : me(me)
{
	RoomOutlineShader::Shader.AddRef();
	SelectedRoomShader::Shader.AddRef();
	ArrowShader::Shader.AddRef();
}

MetaroomGL::~MetaroomGL()
{
}

void MetaroomGL::Release(GLViewWidget *gl)
{
	gl->glAssert();
	_indices.Release(gl);
	me._selection.Release(gl);
	RoomOutlineShader::Shader.Release(gl);
	SelectedRoomShader::Shader.Release(gl);
	ArrowShader::Shader.Release(gl);

	gl->glDeleteVertexArrays(NoArrays, _vao);
	gl->glDeleteBuffers(NoBuffers, _buffers);
}

void MetaroomGL::Render(GLViewWidget *gl, int selected_door_type)
{
	if(me.empty())
		return;

	Prepare(gl);

	if(me._selection.NoSelectedFaces() > 0)
	{
		gl->glBindVertexArray(_vao[1]);
		SelectedRoomShader::Shader.Render(gl, me._selection.NoSelectedFaces());
	}

	gl->glBindVertexArray(_vao[0]);
	gl->glLineWidth(3);
	RoomOutlineShader::Shader.Render(gl, me.noFaces(), true);
	gl->glLineWidth(1);
	RoomOutlineShader::Shader.Render(gl, me.noFaces(), false);

	me._tree.Render(gl, selected_door_type);

	if(_noHalls &&  me.document->m_window->viewHalls())
	{
		gl->glBindVertexArray(_vao[aHalls]);
		ArrowShader::Shader.Render(gl, _noHalls, glm::vec4(1, 0, 0, 1));
	}

	if( me.noFaces() &&  me.document->m_window->viewGravity())
	{
		gl->glBindVertexArray(_vao[aGravity]);
		ArrowShader::Shader.Render(gl, me.noFaces(), glm::vec4(0, 0, 1, 1));
	}
}

void MetaroomGL::Prepare(GLViewWidget* gl)
{
	me._selection.Prepare(gl);

	if(_dirty == false || me.noFaces() == 0)
		return;

	_dirty = false;
	bool initialized_vaos = (_vao[0] != 0L);

	if(_indicesDirty)
	{
		_indicesDirty = false;
		_indices.Prepare(gl, me.range());
	}


	if(initialized_vaos == false)
	{
		gl->glGenVertexArrays(NoArrays, _vao);
		gl->glGenBuffers(NoBuffers, _buffers);

		gl->glBindVertexArray(_vao[0]);
		gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers[bVertices]);
		gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(glm::ivec2), 0);
		gl->glEnableVertexAttribArray(0);

		gl->glBindBuffer(GL_ARRAY_BUFFER, me._selection.GetBuffer());
		gl->glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), 0);
		gl->glEnableVertexAttribArray(1);

		gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices.EdgeVBO());

// create selected face shader
		gl->glBindVertexArray(_vao[1]);
		gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers[bVertices]);
		gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(glm::ivec2), 0);
		gl->glEnableVertexAttribArray(0);

		gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, me._selection.GetIndices());

		gl->glBindVertexArray(_vao[aHalls]);
		gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers[bHalls]);
		gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(ArrowShader::Vertex), (void*) offsetof(ArrowShader::Vertex, position));
		gl->glVertexAttribPointer(1, 2, GL_SHORT, GL_TRUE, sizeof(ArrowShader::Vertex), (void*) offsetof(ArrowShader::Vertex, rotation));

		gl->glEnableVertexAttribArray(0);
		gl->glEnableVertexAttribArray(1);

		gl->glBindVertexArray(_vao[aGravity]);
		gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers[bGravity]);
		gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(ArrowShader::Vertex), (void*) offsetof(ArrowShader::Vertex, position));
		gl->glVertexAttribPointer(1, 2, GL_SHORT, GL_TRUE, sizeof(ArrowShader::Vertex), (void*) offsetof(ArrowShader::Vertex, rotation));

		gl->glEnableVertexAttribArray(0);
		gl->glEnableVertexAttribArray(1);
	}

	gl->glBindVertexArray(0);
	gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers[bVertices]);
	gl->glBufferData(GL_ARRAY_BUFFER, me._verts.byteLength(), me._verts.data(), GL_DYNAMIC_DRAW);
	gl->glAssert();

//prepare hall transitions
	std::vector<ArrowShader::Vertex> halls;
	halls.reserve(me.noFaces());

	ArrowShader::Vertex buffer;
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

				halls.push_back(buffer);
			}
		}
	}

	gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers[bHalls]);
	gl->glAssert();

	gl->glBufferData(GL_ARRAY_BUFFER, sizeof(halls[0])*halls.capacity(), &halls[0], GL_DYNAMIC_DRAW);
	_noHalls = halls.size();
	gl->glAssert();

//prepare gravity arrows
	halls.resize(me.noFaces());

	for(auto i : me.range())
	{
		halls[i].position = (me.GetVertex(i, 0) + me.GetVertex(i, 1) + me.GetVertex(i, 2) + me.GetVertex(i, 3))/4;
		halls[i].rotation = (float)SHRT_MAX * glm::normalize(glm::unpackHalf2x16(me._gravity[i]));
	}

	gl->glBindBuffer(GL_ARRAY_BUFFER, _buffers[bGravity]);
	gl->glAssert();

	gl->glBufferData(GL_ARRAY_BUFFER, sizeof(halls[0])*halls.size(), &halls[0], GL_DYNAMIC_DRAW);
	gl->glAssert();
}
