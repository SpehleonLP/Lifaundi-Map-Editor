#include "selectedroomshader.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>

void SelectedRoomShader::operator()(QOpenGLFunctions * gl, uint32_t faces)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);

	gl->glEnable(GL_BLEND);

    gl->glDrawElements(GL_TRIANGLES, faces*6, GL_UNSIGNED_INT, 0L);
}

const char SelectedRoomShader::Vertex[] = SHADER(150,
	layout(std140) uniform Matrices
	{
		mat4 u_projection;
		mat4 u_modelview;
	};

	in ivec2 a_position;

	out float texCoord;

	void main()
	{
		gl_Position = u_projection * (u_modelview * vec4(a_position, 0, 1.0));
	});

const char SelectedRoomShader::Fragment[] = SHADER(150,
	out vec4 frag_color;

	void main()
	{
		int x = int(gl_FragCoord.x) % 16;
		int y = int(gl_FragCoord.y) % 16;

		frag_color = vec4(int(x == y || x == 16 - y));
	});


void SelectedRoomShader::Initialize(QOpenGLFunctions* gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "SelectedRoomShader", vertex, nullptr, fragment))
		return;

	attribute(gl, 0, "a_position");

	OpenGL_LinkProgram(gl, "SelectedRoomShader", program());
}


