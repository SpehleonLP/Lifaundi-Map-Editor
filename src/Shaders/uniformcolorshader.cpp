#include "uniformcolorshader.h"
#include "src/Shaders/shaders.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>

void UniformColorShader::Bind(QOpenGLFunctions* gl, float red, float green, float blue, float alpha)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);
	gl->glDisable(GL_CULL_FACE);

	gl->glEnable(GL_BLEND);
	gl->glBlendEquation(GL_FUNC_ADD);
	gl->glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);

    gl->glUniform4f(u_color, red, green, blue, alpha);
}

const char UniformColorShader::Vertex[] = SHADER(150,
		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
		};

		in ivec2 a_vertex;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_vertex, 0, 1.0));
		});

const char UniformColorShader::Fragment[] = SHADER(150,
		uniform vec4 u_color;

		out vec4 frag_color;

		void main()
		{
			frag_color = u_color;
		});

void UniformColorShader::Initialize(QOpenGLFunctions* gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "UniformColorShader", vertex, nullptr, fragment))
		return;

	attribute(gl, 0, "a_vertex");

	OpenGL_LinkProgram(gl, "UniformColorShader", program());

	uniform(gl, u_color, "u_color");
}
