#include "doorshader.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>

void DoorShader::Bind(QOpenGLFunctions * gl, int selected_door_type)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);
	gl->glDisable(GL_CULL_FACE);

	gl->glEnable(GL_BLEND);
	gl->glBlendEquation(GL_FUNC_ADD);
	gl->glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);

    gl->glUniform1i(u_activeType, selected_door_type);
}

const char DoorShader::Vertex[] = SHADER(150,
		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
			ivec2 u_screenSize;
			float u_ctime;
		};

		uniform int       u_activeType;

		in ivec2 a_position;
		in int   a_type;
		in int   a_permeability;

		out vec4 v_color;
		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_position, 0, 1.0));

			float perm = a_permeability / 100.f;

			vec2 color = vec2(1.f - perm, perm);
			color = pow(color, vec2(1 / 2.8));

			v_color = vec4(color, 0, 1);
		});


const char DoorShader::Fragment[] = SHADER(150,
	in vec4 v_color;

	out vec4 frag_color;

	void main()
	{
		frag_color = v_color;
	});


void DoorShader::Initialize(QOpenGLFunctions* gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "DoorShader", vertex, nullptr, fragment))
		return;

	attribute(gl, 0, "a_position");
	attribute(gl, 1, "a_type");
	attribute(gl, 2, "a_permeability");

	OpenGL_LinkProgram(gl, "DoorShader", program());

	uniform(gl, u_activeType, "u_activeType");
}
