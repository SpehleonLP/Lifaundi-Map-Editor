#include "uniformcolorshader.h"
#include "src/glviewwidget.h"

#define SHADER(k) "#version 150\n" #k
static const char * kVert();
static const char * kFrag();

UniformColorShader UniformColorShader::Shader;

void UniformColorShader::construct(GLViewWidget* gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
    attribute(gl, 0, "a_vertex");
    link(gl);

#define UNIFORM(x) uniform(gl,x, #x)
	UNIFORM(u_color);
#undef UNIFORM

    uniformBlock(gl, 0, "Matrices");
}

void UniformColorShader::Bind(GLViewWidget* gl, float red, float green, float blue, float alpha)
{
    gl->glAssert();

    if(bindShader(gl))
	{
        gl->glDisable(GL_DEPTH_TEST);
        gl->glDepthMask(GL_FALSE);
        gl->glDisable(GL_CULL_FACE);

        gl->glEnable(GL_BLEND);
        gl->glBlendEquation(GL_FUNC_ADD);
        gl->glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
	}

    gl->glUniform4f(u_color, red, green, blue, alpha);
}

static const char * kVert()
{
	return SHADER(
		layout(std140) uniform Matrices
		{
			mat4 u_projection;
			mat4 u_modelview;
		};

		in ivec2 a_vertex;

		void main()
		{
			gl_Position = u_projection * (u_modelview * vec4(a_vertex, 0, 1.0));
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

