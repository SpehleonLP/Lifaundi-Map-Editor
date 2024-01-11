#include "roomoutlineshader.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>

void RoomOutlineShader::operator()(QOpenGLFunctions*gl, uint32_t faces, bool selected)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);

	gl->glEnable(GL_BLEND);

    gl->glUniform1i(u_renderSelected, selected);
    gl->glDrawElements(GL_LINES, faces*8, GL_UNSIGNED_INT, 0L);
}

const char RoomOutlineShader::Vertex[] = SHADER(150,
		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
		};

		uniform int u_renderSelected;

		in ivec2 a_position;
		in int  a_selected;

		out vec4 v_color;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_position, 0, 1.0));
			int s = a_selected * u_renderSelected;
			v_color = vec4(s, 1, 1-s, (1 - u_renderSelected) + s);
		});


const char RoomOutlineShader::Fragment[] = SHADER(150,
		in vec4 v_color;

		out vec4 frag_color;

		void main()
		{
			frag_color = v_color;
		});


void RoomOutlineShader::Initialize(QOpenGLFunctions* gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "RoomOutlineShader", vertex, nullptr, fragment))
		return;

	attribute(gl, 0, "a_position");
	attribute(gl, 1, "a_selected");

	OpenGL_LinkProgram(gl, "RoomOutlineShader", program());

	uniform(gl, u_renderSelected, "u_renderSelected");
}
