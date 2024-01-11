#include "blitshader.h"
#include "defaultvaos.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>

void BlitShader::Bind(QOpenGLFunctions * gl, BackgroundLayer layer)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);
	gl->glEnable(GL_BLEND);
	gl->glDisable(GL_CULL_FACE);

	gl->glUniform1i(u_layer, 0);
	gl->glUniform1i(u_texture, 0);

	gl->glUniform1i(u_layer, (int)layer);
}

const char BlitShader::Vertex[] = SHADER(150,
		layout(std140) uniform Matrices
		{
			mat4  u_projection;
			mat4  u_modelview;
			ivec2 u_screenSize;
		};

		in vec3 a_vertex;
		in vec2 a_texCoord0;

		out vec2 v_texCoord0;
		flat out int v_layer;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_vertex, 1.0));
			v_texCoord0 = a_texCoord0;
			v_layer = (gl_VertexID / 6) % 256;
		});

const char BlitShader::Fragment[] = SHADER(150,
		uniform sampler2DArray u_texture;
		uniform int u_layer;

		in vec2 v_texCoord0;
		flat in int v_layer;

		out vec4 frag_color;

		void main()
		{
			switch(u_layer)
			{
			case 2:
			{ // normals
				frag_color = texture(u_texture, vec3(v_texCoord0, v_layer));
				vec2 rg = frag_color.rg * 2.0 - 1.0;
				frag_color.b = (1.0 - dot(rg, rg)) * 0.5 + 0.5;
				break;
			}
			//AO/depth
			case 3:
			case 4:
				frag_color = texture(u_texture, vec3(v_texCoord0, v_layer));
				frag_color.rgb = vec3(frag_color.r);
				break;
			default:
				frag_color = texture(u_texture, vec3(v_texCoord0, v_layer));
				break;
			}
		});

void BlitShader::Initialize(QOpenGLFunctions* gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "BlitShader", vertex, nullptr, fragment))
		return;

	attribute(gl, 0, "a_vertex");
	attribute(gl, 3, "a_texCoord0");

	OpenGL_LinkProgram(gl, "BlitShader", program());

	uniform(gl, u_texture, "u_texture");
	uniform(gl, u_layer, "u_layer");
}
