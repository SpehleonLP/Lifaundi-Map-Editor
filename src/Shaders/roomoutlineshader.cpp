#include "roomoutlineshader.h"

#include "src/glviewwidget.h"

#define SHADER(k) "#version 150\n" #k
static const char * kVert();
static const char * kFrag();

RoomOutlineShader RoomOutlineShader::Shader;


void RoomOutlineShader::construct(GLViewWidget*gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
    attribute(gl, 0, "a_position");
    attribute(gl, 1, "a_selected");
    link(gl);

#define UNIFORM(x) uniform(gl, x, #x)
	UNIFORM(u_renderSelected);
#undef UNIFORM

    uniformBlock(gl, 0, "Matrices");
}

void RoomOutlineShader::Render(GLViewWidget*gl, uint32_t faces, bool selected)
{
    gl->glAssert();

    if(bindShader(gl))
	{
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_FALSE);

        gl->glEnable(GL_BLEND);
	}

    gl->glUniform1i(u_renderSelected, selected);
    gl->glDrawElements(GL_LINES, faces*8, GL_UNSIGNED_INT, 0L);
}

static const char * kVert()
{
	return SHADER(
		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
		};

		uniform int u_renderSelected;

		in ivec2 a_position;
		in int  a_selected;

		out vec4 v_color;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_position, 0, 1.0));
			int s = a_selected * u_renderSelected;
			v_color = vec4(s, 1, 1-s, (1 - u_renderSelected) + s);
		});
}

static const char * kFrag()
{
	return SHADER(
		in vec4 v_color;

		out vec4 frag_color;

		void main()
		{
			frag_color = v_color;
		});
}

