#include "transparencyshader.h"
#include "defaultvaos.h"
#include "gl/compressedshadersource.h"
#include "src/Shaders/shaders.h"
#include <QOpenGLFunctions_4_5_Core>

void TransparencyShader::Bind(QOpenGLFunctions* gl)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);
	gl->glDisable(GL_BLEND);
	gl->glDisable(GL_CULL_FACE);
}

void TransparencyShader::operator()(Shaders * shaders)
{
	auto gl = shaders->gl;

	Bind(gl);
	shaders->defaultVaos.Bind(gl);
	shaders->defaultVaos.RenderSquare(gl);
}

const char TransparencyShader::Vertex[] = SHADER(150,
	layout(std140) uniform Matrices
	{
		mat4  u_projection;
		mat4  u_modelview;
		ivec4 u_screenSize;
		float u_ctime;
	};

	in vec3 a_vertex;
	in vec2 a_uv;

	out vec2 v_uv;

	void main()
	{
		gl_Position = u_projection * (u_modelview * vec4(a_vertex, 1.0));
		v_uv        = a_uv;
	});

const char TransparencyShader::Fragment[] = SHADER(150,
	in vec2 v_uv;
	in vec4 gl_FragCoord;

	out vec4 frag_color;

	void main()
	{
		ivec2 coords = ivec2(gl_FragCoord);
		int tile = (int(coords.x / 10) & 0x01) ^ (int(coords.y / 10) & 0x01);
		float color = (125 + 100 * tile) / 256.f;
		frag_color = vec4(vec3(color), 1.0);
	});

void TransparencyShader::Initialize(QOpenGLFunctions* gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "TransparencyShader", vertex, nullptr, fragment))
		return;

	attribute(gl, 0, "a_position");
	attribute(gl, 1, "a_selected");

	OpenGL_LinkProgram(gl, "TransparencyShader", program());
}
