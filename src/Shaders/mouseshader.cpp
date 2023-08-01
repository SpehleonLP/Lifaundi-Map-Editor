#include "mouseshader.h"
#include "defaultvaos.h"

#include "src/glviewwidget.h"

#define SHADER(k) "#version 150\n" #k
static const char * kVert();
static const char * kFrag();

MouseShader MouseShader::Shader;


void MouseShader::construct(GLViewWidget*gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
    link(gl);

#define UNIFORM(x) uniform(gl, x, #x)
	UNIFORM(u_position);
	UNIFORM(u_color);
#undef UNIFORM

    uniformBlock(gl, 0, "Matrices");
}

void MouseShader::Render(GLViewWidget* gl, glm::ivec2 position, uint32_t size, glm::vec4 color)
{
    gl->glAssert();

    if(bindShader(gl))
	{
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_FALSE);

        gl->glEnable(GL_BLEND);
	}
	
	gl->glPointSize(size);
	gl->glUniform2iv(u_position, 1, &position.x);
	gl->glUniform4fv(u_color, 1, &color.x);
	
	glDefaultVAOs::BindVAO(gl);	
	glDefaultVAOs::RenderPoint(gl);
}

static const char * kVert()
{
	return SHADER(
		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
		};

		uniform ivec2 u_position;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(u_position, 0, 1.0));
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

