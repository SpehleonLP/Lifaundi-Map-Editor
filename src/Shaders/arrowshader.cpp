#include "arrowshader.h"
#include "src/glviewwidget.h"

#define SHADER(k) "#version 330\n" #k
static const char * kVert();
static const char * kGeometry();
static const char * kFrag();

ArrowShader ArrowShader::Shader;


void ArrowShader::construct(GLViewWidget* gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kGeometry(), GL_GEOMETRY_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
    attribute(gl, 0, "a_position");
    attribute(gl, 1, "a_rotation");
    link(gl);

	uniform(gl, u_color, "u_color");

    uniformBlock(gl, 0, "Matrices");
}

void ArrowShader::Render(GLViewWidget* gl, uint32_t size, glm::vec4 color)
{
    if(bindShader(gl))
	{
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_FALSE);
        gl->glDisable(GL_CULL_FACE);

        gl->glEnable(GL_BLEND);
	}

    gl->glPointSize(10.f);
	gl->glUniform4fv(u_color, 1, &color[0]);
    gl->glDrawArrays(GL_POINTS, 0, size);
}

static const char * kVert()
{
	return SHADER(
		in ivec2 a_position;
		in vec2 a_rotation;

		out vec2 v_position;
		out vec2 v_rotation;


		void main()
		{
			v_position = a_position;
			v_rotation = a_rotation;
			gl_Position = vec4(0, 0, 0, 1);
		});
}

#define TO_STRING(x) #x

static const char * kGeometry()
{
	return SHADER(
		layout(points) in;
		layout(triangle_strip, max_vertices = 22) out;

		const vec2 verts[11] = vec2[](
			vec2(-10,  4), //0
			vec2(- 8,  0), //1
			vec2(-10, -4), //2
			vec2(  2, -6), //3
			vec2( 10,  0), //4
			vec2(  2,  6), //5
			vec2(  5,  0), //6

			vec2(3,  0), //7
			vec2(0, -6), //8
			vec2(0,  6),  //9
			vec2(0, 0)
		);

		const int indices[] = int[](
			0,
			0, 4, 1, 2, 2,
			3, 3, 4, 6, 5,
			0, 4, 1, 2, 2,
			8, 8, 4, 7, 9, 9, 9
		);

		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
			ivec4 u_screenSize;
			vec4 u_misc;
		};

		uniform vec4 u_color;
		out vec4 g_color;

		in vec2 v_position[];
		in vec2 v_rotation[];

		void DrawArrow(in vec4 position, in mat2 rotation, in vec2 scale, in vec4 color, int start)
		{
			for(int i = 0; i < 11; ++i)
			{
				vec4 pos    = vec4(rotation * (verts[indices[start+i]] * scale), 0, 0) + position;

				gl_Position = u_projection * (pos);
				g_color     = color;
				EmitVertex();
			}
		}

		void main()
		{
			mat2 rotation = mat2(1);
			rotation[0] = vec2(v_rotation[0].x, -v_rotation[0].y);
			rotation[1] = vec2(v_rotation[0].y,  v_rotation[0].x);

			vec4 position = u_modelview * vec4(v_position[0], 0, 1);

			DrawArrow(position, rotation, 2* vec2(1.25, 1.5) * u_misc.y, vec4(1), 11);
			DrawArrow(position, rotation, 2* vec2(1) * u_misc.y, u_color, 0);

			EndPrimitive();
		});
}

static const char * kFrag()
{
	return SHADER(

		in vec4  g_color;
		out vec4 frag_color;

		void main()
		{
			frag_color = g_color;
		});
}
