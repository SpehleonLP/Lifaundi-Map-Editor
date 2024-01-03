#include "selectedroomshader.h"
#include "src/glviewwidget.h"

#define SHADER(k) "#version 150\n" #k
static const char * kVert();
static const char * kFrag();

SelectedRoomShader SelectedRoomShader::Shader;

void SelectedRoomShader::construct(GLViewWidget * gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
    attribute(gl, 0, "a_position");
    link(gl);

    uniformBlock(gl, 0, "Matrices");
}


void SelectedRoomShader::Render(GLViewWidget * gl, uint32_t faces)
{
    gl->glAssert();

    if(bindShader(gl))
	{
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_FALSE);

        gl->glEnable(GL_BLEND);
	}

    gl->glDrawElements(GL_TRIANGLES, faces*6, GL_UNSIGNED_INT, 0L);
}

static const char * kVert()
{
	return SHADER(
		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
		};

		in ivec2 a_position;

		out float texCoord;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_position, 0, 1.0));
		});
}

static const char * kFrag()
{
	return SHADER(
		out vec4 frag_color;

		void main()
		{
			int x = int(gl_FragCoord.x) % 16;
			int y = int(gl_FragCoord.y) % 16;

			frag_color = vec4(int(x == y || x == 16 - y));
		});
}

