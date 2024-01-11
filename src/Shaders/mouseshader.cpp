#include "mouseshader.h"
#include "defaultvaos.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>


void MouseShader::operator()(QOpenGLFunctions* gl, glm::ivec2 position, uint32_t size, glm::vec4 color)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);

	gl->glEnable(GL_BLEND);
	
	gl->glPointSize(size);
	gl->glUniform2iv(u_position, 1, &position.x);
	gl->glUniform4fv(u_color, 1, &color.x);
	
	_vao.Bind(gl);
	_vao.RenderPoint(gl);
}

const char MouseShader::Vertex[] = SHADER(150,
	layout(std140) uniform Matrices
	{
		mat4 u_projection;
		mat4 u_modelview;
	};

	uniform ivec2 u_position;

	void main()
	{
		gl_Position = u_projection * (u_modelview * vec4(u_position, 0, 1.0));
	});

const char MouseShader::Fragment[] = SHADER(150,
	uniform vec4 u_color;
	out vec4 frag_color;

	void main()
	{
		frag_color = u_color;
	});

void MouseShader::Initialize(QOpenGLFunctions* gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "MouseShader", vertex, nullptr, fragment))
		return;

	attribute(gl, 0, "a_position");
	attribute(gl, 1, "a_type");
	attribute(gl, 2, "a_permeability");

	OpenGL_LinkProgram(gl, "MouseShader", program());

	uniform(gl, u_position, "u_position");
	uniform(gl, u_color, "u_color");
}
