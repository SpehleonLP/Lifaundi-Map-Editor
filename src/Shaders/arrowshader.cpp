#include "arrowshader.h"
#include "gl/compressedshadersource.h"
#include "gl/renderdoc.h"
#include <QOpenGLFunctions_4_5_Core>
#include <glm/gtc/type_precision.hpp>
#include <QtLogging>

void debugVAOState(QOpenGLFunctions_4_5_Core * gl, std::string baseMessage = "")
{
	baseMessage.append( " ... querying VAO state:\n" );
	int vab, eabb, eabbs, mva, isOn( 1 ), vaabb;
	glGetIntegerv( GL_VERTEX_ARRAY_BINDING, &vab );
	glGetIntegerv( GL_ELEMENT_ARRAY_BUFFER_BINDING, &eabb );
	gl->glGetBufferParameteriv( GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eabbs );

	baseMessage.append( "  VAO: " + std::to_string( vab ) + "\n" );
	baseMessage.append( "  IBO: " + std::to_string( eabb ) + ", size=" + std::to_string( eabbs )  + "\n" );

	gl->glGetIntegerv( GL_MAX_VERTEX_ATTRIBS, &mva );
	for ( int i = 0; i < mva; ++i )
	{
		gl->glGetVertexAttribiv( i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isOn );
		if ( isOn )
		{
			gl->glGetVertexAttribiv( i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vaabb );
			baseMessage.append( "  attrib #" + std::to_string( i ) + ": VBO=" + std::to_string( vaabb ) + "\n" );
		}
	}

	qDebug( "%s", baseMessage.c_str() );
}

void ArrowShader::Destroy(QOpenGLFunctions * gl)
{
	ShaderBase::Destroy(gl);

	gl->glDeleteVertexArrays(1, &_vao);
	gl->glDeleteBuffers(3, _vbo);
}

void ArrowShader::operator()(QOpenGLFunctions* gl, std::vector<Arrow> const& arrows, glm::vec4 color)
{
	RenderDocCaptureRAII raii("Arrow Shader", true);

	assert(createdWith == gl);

	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);
	gl->glDisable(GL_CULL_FACE);

	gl->glEnable(GL_BLEND);

    gl->glPointSize(10.f);
	gl->glUniform4fv(u_color, 1, &color[0]);

	gl->glBindVertexArray(_vao);
	gl->glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
	gl->glBufferData(GL_ARRAY_BUFFER, arrows.size() * sizeof(arrows[0]), arrows.data(), GL_STREAM_DRAW);

//	debugVAOState(gl);

	gl->glDrawElementsInstanced(GL_TRIANGLES, _count, GL_UNSIGNED_BYTE, nullptr, arrows.size());
	gl->glBindVertexArray(0);
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

	in vec3  a_vertex;

// instanced
	in vec2 a_position;
	in vec2 a_rotation;

	out vec4 g_color;

	void main()
	{


		mat2 rotation = mat2(1);
		rotation[0] = vec2(a_rotation.x, -a_rotation.y);
		rotation[1] = vec2(a_rotation.y,  a_rotation.x);

		vec4 position = u_modelview * vec4(a_position, 0, 1);
		position   += vec4(rotation * (a_vertex.xy * u_misc.y), 0, 0) + position;

		gl_Position = u_projection * position;
		g_color     = mix(vec4(1), u_color, a_vertex.z);
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
	attribute(gl, 3, "a_position");
	attribute(gl, 4, "a_rotation");

	OpenGL_LinkProgram(gl, "ArrowShader", program());

	uniform(gl, u_color, "u_color");

	const glm::i8vec4 vertices[] ={
		{-10,  4, 0, 0}, //0
		{- 8,  0, 0, 0}, //1
		{-10, -4, 0, 0}, //2
		{  2, -6, 0, 0}, //3
		{ 10,  0, 0, 0}, //4
		{  2,  6, 0, 0}, //5
		{  5,  0, 0, 0}, //6

		{3,  0, 0, 0}, //7
		{0, -6, 0, 0}, //8
		{0,  6, 0, 0},  //9
		{0, 0, 0, 0},
};
	createdWith = gl;

const uint8_t indices[] = {
0, 4, 1, 2, 2,
3, 3, 4, 6, 5,
0, 4, 1, 2, 2,
8, 8, 4, 7, 9, 9, 9
};
// DrawArrow(position, rotation, 2* vec2(1.25, 1.5) * u_misc.y, vec4(1), 11);
// DrawArrow(position, rotation, 2* vec2(1) * u_misc.y, u_color, 0);
	gl->glGenVertexArrays(1, &_vao);
	gl->glGenBuffers(3, _vbo);

	gl->glBindVertexArray(_vao);

	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[1]);
	gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	_count = sizeof(indices)/sizeof(indices[0]);

	gl->glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
	gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	gl->glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, 4, (void*)0);
	gl->glEnableVertexAttribArray(0);

	gl->glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);

	gl->glVertexAttribPointer(3, 2, GL_INT, GL_FALSE, sizeof(Arrow), (void*)offsetof(Arrow,position));
	gl->glVertexAttribPointer(4, 2, GL_SHORT, GL_TRUE, sizeof(Arrow), (void*)offsetof(Arrow,rotation));

	gl->glEnableVertexAttribArray(3);
	gl->glEnableVertexAttribArray(4);

	gl->glVertexAttribDivisor(3, 1);
	gl->glVertexAttribDivisor(4, 1);

//	debugVAOState(gl);
	gl->glBindVertexArray(0);

}
