#include "doorshader.h"

#include "src/glviewwidget.h"

#define SHADER(k) "#version 150\n" #k
static const char * kVert();
static const char * kFrag();

DoorShader DoorShader::Shader;


void DoorShader::construct(GLViewWidget * gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
    attribute(gl, 0, "a_position");
    attribute(gl, 1, "a_type");
	attribute(gl, 1, "a_permeability");
    link(gl);

#define UNIFORM(x) uniform(gl, x, #x)
	UNIFORM(u_activeType);
#undef UNIFORM

    uniformBlock(gl, 0, "Matrices");
}

void DoorShader::Bind(GLViewWidget * gl, int selected_door_type)
{
    if(bindShader(gl))
	{
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_FALSE);
        gl->glDisable(GL_CULL_FACE);

        gl->glEnable(GL_BLEND);
        gl->glBlendEquation(GL_FUNC_ADD);
        gl->glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
	}

    gl->glUniform1i(u_activeType, selected_door_type);
}

static const char * kVert()
{
	return SHADER(
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
		out vec2 v_stipple_position;
		out float v_time;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_position, 0, 1.0));

			float perm = a_permeability / 100.f;

			v_color = vec4(pow(vec2(1.f - perm, perm), vec2(1 / 2.2)), 0, 1);
			v_stipple_position = mix(vec2(0, 0), gl_Position.xy * u_screenSize + u_ctime, float(u_activeType == a_type));
			v_time = mod(u_ctime, 60);
		});
}

static const char * kFrag()
{
	return SHADER(
		in vec4 v_color;
		in vec2 v_stipple_position;
		in vec4 gl_FragCoord;
		in float v_time;

		out vec4 frag_color;

		void main()
		{
			ivec2 coords = ivec2(gl_FragCoord.xy);
			int tile = (int(coords.x / 10) & 0x01) ^ (int(coords.y / 10) & 0x01);

			frag_color = v_color;
		});
}
