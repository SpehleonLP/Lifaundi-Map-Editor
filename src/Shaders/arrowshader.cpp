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
	return
		"#version 330\n"
		"layout(points) in;\n"
		"layout(triangle_strip, max_vertices = 18) out;\n"
		"\n"
		"const ivec2 verts[7] = ivec2[](\n"
			"ivec2(-10,  4),\n"
			"ivec2(- 8,  0),\n"
			"ivec2(-10, -4),\n"
			"ivec2(  2, -6),\n"
			"ivec2( 10,  0),\n"
			"ivec2(  2,  6),\n"
			"ivec2(  5,  0)\n"
		");\n"
		"\n"
		"const int indices[10] = int[](\n"
			"0, 4, 1, 2, 2,\n"
			"3, 3, 4, 6, 5\n"
		");\n"

		TO_STRING(
		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
		};

		in vec2 v_position[];
		in vec2 v_rotation[];

		void main()
		{
			mat2 rotation = mat2(1);
		//	rotation[0] = vec2(v_rotation[0].x, -v_rotation[0].y);
		//	rotation[1] = vec2(v_rotation[0].y,  v_rotation[0].x);

			vec4 position = u_modelview * vec4(v_position[0], 0, 1);

			for(int i = 0; i < 10; ++i)
			{
				vec4 pos    = vec4(rotation * verts[indices[i]], 0, 0) + position;

				gl_Position = u_projection * (pos);
				EmitVertex();
			}

			EndPrimitive();
		});
}

static const char * kFrag()
{
	return SHADER(
		uniform vec4 u_color;

		out vec4 frag_color;

		void main()
		{
			frag_color = u_color;
		});
}
