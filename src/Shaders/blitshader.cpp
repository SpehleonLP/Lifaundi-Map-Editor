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
	UNIFORM(u_layer);
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

void BlitShader::bind(GLViewWidget * gl, BackgroundLayer layer)
{
    if(bindShader(gl))
	{
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_FALSE);
        gl->glEnable(GL_BLEND);
        gl->glDisable(GL_CULL_FACE);
        glDefaultVAOs::BindVAO(gl);

		gl->glUniform1i(u_layer, 0);
        gl->glUniform1i(u_texture, 0);
	}

	gl->glUniform1i(u_layer, (int)layer);
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
		uniform int u_layer;

		in vec2 v_texCoord0;

		out vec4 frag_color;

		void main()
		{
			switch(u_layer)
			{
			case 2:
			{ // normals
				frag_color = texture(u_texture, v_texCoord0);
				vec2 rg = frag_color.rg * 2.0 - 1.0;
				frag_color.b = (1.0 - dot(rg, rg)) * 0.5 + 0.5;
				break;
			}
			//AO/depth
			case 3:
			case 4:
				frag_color = texture(u_texture, v_texCoord0);
				frag_color.rgb = vec3(frag_color.r);
				break;
			default:
				frag_color = texture(u_texture, v_texCoord0);
				break;
			}
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

