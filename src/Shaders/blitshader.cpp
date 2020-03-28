#include "blitshader.h"
#include "defaultvaos.h"
#include "src/glviewwidget.h"

#define SHADER(k) "#version 150\n" #k
static const char * kVert();
static const char * kFrag();

BlitShader BlitShader::Shader;

void BlitShader::construct(GLViewWidget * gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
    attribute(gl, 0, "a_vertex");
    attribute(gl, 3, "a_texCoord0");
    link(gl);

#define UNIFORM(x) uniform(gl, x, #x)
	UNIFORM(u_texture);
#undef UNIFORM

    uniformBlock(gl, 0, "Matrices");
}

void BlitShader::AddRef()
{
	glDefaultVAOs::AddRef();
	glProgram::AddRef();
}

void BlitShader::Release(GLViewWidget * gl)
{
	gl->glAssert();
    glDefaultVAOs::Release(gl);
    glProgram::Release(gl);
}

void BlitShader::bind(GLViewWidget * gl)
{
    if(bindShader(gl))
	{
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_FALSE);
        gl->glEnable(GL_BLEND);
        gl->glDisable(GL_CULL_FACE);
        glDefaultVAOs::BindVAO(gl);

        gl->glUniform1i(u_texture, 0);
	}

    gl->glAssert();
}

void BlitShader::render(GLViewWidget * gl)
{
    glDefaultVAOs::RenderSquare(gl);
}

static const char * kVert()
{
	return SHADER(
		layout(std140) uniform Matrices
		{
			mat4  u_projection;
			mat4  u_modelview;
			ivec2 u_screenSize;
		};

		in vec3 a_vertex;
		in vec2 a_texCoord0;

		out vec2 v_texCoord0;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_vertex, 1.0));
			v_texCoord0 = a_texCoord0;
		});
}

/*
static const char * kVert()
{
	return SHADER(
		in vec3 a_vertex;
		in vec2 a_uv;

		out vec2 v_uv;

		void main()
		{
			gl_Position = vec4(a_vertex, 1.0);
			v_uv        = a_uv;
		});
}
*/

static const char * kFrag()
{
	return SHADER(
		uniform sampler2D u_texture;

		in vec2 v_texCoord0;

		out vec4 frag_color;

		void main()
		{
			frag_color = texture(u_texture, v_texCoord0);
		});
}

/*
static const char * kFrag()
{
	return SHADER(
		uniform ivec2 u_screensize;

		in vec2 v_uv;
		out vec4 frag_color;

		void main()
		{
			ivec2 coords = ivec2(v_uv * u_screensize);
			int tile = (int(coords.x / 10) & 0x01) ^ (int(coords.y / 10) & 0x01);
			float color = (125 + 100 * tile) / 256.f;
			frag_color = vec4(vec3(color), 1.0);
		});
}*/

