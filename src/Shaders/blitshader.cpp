#include "blitshader.h"
#include "defaultvaos.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>

void BlitShader::Bind(QOpenGLFunctions * gl, BackgroundLayer layer, glm::uvec2 range)
{
	gl->glUseProgram(program());

	gl->glDisable(GL_DEPTH_TEST);
	gl->glDepthMask(GL_FALSE);
	gl->glEnable(GL_BLEND);
	gl->glDisable(GL_CULL_FACE);

	gl->glUniform1i(u_layer, 0);
	gl->glUniform1i(u_texture, 0);
	gl->glUniform2f(u_range, range.x / (float)USHRT_MAX, range.y / (float)USHRT_MAX);

	gl->glUniform1i(u_layer, (int)layer);
}

const char BlitShader::Vertex[] = SHADER(400,
		layout(std140) uniform Matrices
		{
			mat4  u_projection;
			mat4  u_modelview;
			ivec4 u_screenSize;
			vec4  u_misc;
		};

		in vec3 a_vertex;
		in vec2 a_texCoord0;
		out vec2 v_position;

		out vec2 v_texCoord0;
		flat out int v_layer;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_vertex, 1.0));
			v_texCoord0 = a_texCoord0;
			v_position  = vec2(a_vertex);
			v_layer = (gl_VertexID / 6) % 256;
		});

const char BlitShader::Fragment[] = SHADER(400, )
	"#define u_time u_misc.x\n"
	"#define u_zoom u_misc.y\n"
	"#define u_pxPerMeter int(u_misc.z)\n"
	"#define u_grid (int(u_misc.w) & 0x01)\n"
TEXT(
   layout(std140) uniform Matrices
   {
	   mat4  u_projection;
	   mat4  u_modelview;
	   ivec4 u_screenSize;
	   vec4  u_misc;
   };

	uniform sampler2DArray u_texture;
	uniform int u_layer;
	uniform float u_invZoom;
	uniform vec2 u_range;


	in vec2 v_texCoord0;
	in vec2 v_position;
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
			frag_color = texture(u_texture, vec3(v_texCoord0, v_layer));
			frag_color.rgb = vec3(frag_color.r);
			break;
		case 4:
			frag_color = texture(u_texture, vec3(v_texCoord0, v_layer));

			frag_color.rgb = vec3((frag_color.r - u_range.x) / (u_range.y - u_range.x));
			break;
		default:
			frag_color = texture(u_texture, vec3(v_texCoord0, v_layer));
			break;
		}

		if(u_grid != 0
		&&(int(abs(v_position.x)) % u_pxPerMeter <= int(1.f / u_zoom)
		|| int(abs(v_position.y)) % u_pxPerMeter <= int(1.f / u_zoom)))
			frag_color = vec4(1, 0, 0, 1);
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
	uniform(gl, u_range, "u_range");
}
