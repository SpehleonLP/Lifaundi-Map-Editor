#include "arrowshader.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>
#include <glm/gtc/type_precision.hpp>

static const char * kVert();
static const char * kGeometry();
static const char * kFrag();

ArrowShader ArrowShader::Shader;



void ArrowShader::Destroy(QOpenGLFunctions * gl)
{
	ShaderBase::Destroy(gl);

	gl->glDeleteVertexArrays(1, &_vao);
	gl->glDeleteBuffers(3, _vbo);
}

void ArrowShader::operator()(QOpenGLFunctions* gl, std::vector<Arrow> const& arrows, glm::vec4 color)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);
	gl->glDisable(GL_CULL_FACE);

	gl->glEnable(GL_BLEND);

    gl->glPointSize(10.f);
	gl->glUniform4fv(u_color, 1, &color[0]);

	gl->glBindVertexArray(_vao);
	gl->glNamedBufferData(_vbo[2], arrows.size() * sizeof(arrows[0]), arrows.data(), GL_STREAM_DRAW);
	gl->glDrawElementsInstanced(GL_TRIANGLES, _count, GL_UNSIGNED_BYTE, nullptr, arrows.size());
}

const char ArrowShader::Vertex[] = SHADER(330,
	layout(std140) uniform Matrices
	{
		mat4 u_projection;
		mat4 u_modelview;
		ivec4 u_screenSize;
		vec4 u_misc;
	};

	uniform vec4 u_color;

	in vec2  a_vertex;

// instanced
	in ivec2 a_position;
	in vec2 a_rotation;

	out vec4 g_color;

	void main()
	{


		mat2 rotation = mat2(1);
		rotation[0] = vec2(a_rotation[0].x, -a_rotation[0].y);
		rotation[1] = vec2(a_rotation[0].y,  a_rotation[0].x);

		vec4 position = u_modelview * vec4(a_position, 0, 1);
		position   += vec4(rotation * (a_vertex * (a_scale / 2.0 * u_misc.y)), 0, 0) + position;

		gl_Position = u_projection * (pos);
		g_color     = mix(vec4(1), u_color, a_color);
	});



const char ArrowShader::Fragment[] = SHADER(330,
	in vec4  g_color;
	out vec4 frag_color;

	void main()
	{
		frag_color = g_color;
	}
);

void ArrowShader::Initialize(QOpenGLFunctions* gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "ArrowShader", vertex, nullptr, fragment))
		return;

	attribute(gl, 0, "a_vertex");
	attribute(gl, 1, "a_position");
	attribute(gl, 2, "a_rotation");

	OpenGL_LinkProgram(gl, "ArrowShader", program());

	uniform(gl, u_color, "u_color");

	const glm::i8vec2 vertices[] ={
		{-10,  4}, //0
		{- 8,  0}, //1
		{-10, -4}, //2
		{  2, -6}, //3
		{ 10,  0}, //4
		{  2,  6}, //5
		{  5,  0}, //6

		{3,  0}, //7
		{0, -6}, //8
		{0,  6},  //9
		{0, 0},
};

const int indices[] = {
0,
0, 4, 1, 2, 2,
3, 3, 4, 6, 5,
0, 4, 1, 2, 2,
8, 8, 4, 7, 9, 9, 9
};
// DrawArrow(position, rotation, 2* vec2(1.25, 1.5) * u_misc.y, vec4(1), 11);
// DrawArrow(position, rotation, 2* vec2(1) * u_misc.y, u_color, 0);
	gl->glGenVertexArrays(1, &_vao);
	gl->glGenBuffers(1, _vbo);

	gl->glBindVertexArray(_vao);

	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[1]);
	gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	gl->glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	gl->glVertexAttribPointer(0, 2, GL_BYTE, GL_FALSE, 4, (void*)0);
	gl->glVertexAttribPointer(1, 1, GL_BYTE, GL_FALSE, 4, (void*)2);
	gl->glVertexAttribPointer(2, 1, GL_BYTE, GL_FALSE, 4, (void*)3);

	gl->glEnableVertexAttribArray(0);
	gl->glEnableVertexAttribArray(1);
	gl->glEnableVertexAttribArray(2);

	gl->glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);

	gl->glVertexAttribDivisor(1, 2);
	gl->glVertexAttribDivisor(2, 2);

	gl->glVertexAttribPointer(3, 2, GL_INT, GL_FALSE, sizeof(Arrow), (void*)offsetof(Arrow,position));
	gl->glVertexAttribPointer(4, 2, GL_SHORT, GL_FALSE, sizeof(Arrow), (void*)offsetof(Arrow,rotation));

	gl->glEnableVertexAttribArray(3);
	gl->glEnableVertexAttribArray(4);

	gl->glVertexAttribDivisor(3, 2);
	gl->glVertexAttribDivisor(4, 2);
}
