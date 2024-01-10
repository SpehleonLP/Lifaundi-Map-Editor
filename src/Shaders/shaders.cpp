#include "shaders.h"
#include "gl/compressedshadersource.h"
#include <QOpenGLFunctions_4_5_Core>

Shaders::Shaders(QOpenGLFunctions_4_5_Core * gl) :
	_context(gl)
{
	CompressedShaderSource source;

	defaultVaos.Initialize(gl);
	defaultTextures.Initialize(gl);

	arrowShader.Initialize(gl, source);
	blitShader.Initialize(gl, source);
	doorShader.Initialize(gl, source);
	mouseShader.Initialize(gl, source);
	roomOutlineShader.Initialize(gl, source);
	selectedRoomShader.Initialize(gl, source);
	transparencyShader.Initialize(gl, source);
	uniformColorShader.Initialize(gl, source);
}

Shaders::~Shaders()
{
	auto gl = _context;

	defaultVaos.Destroy(gl);
	defaultTextures.Destroy(gl);

	arrowShader.Destroy(gl);
	blitShader.Destroy(gl);
	doorShader.Destroy(gl);
	mouseShader.Destroy(gl);
	roomOutlineShader.Destroy(gl);
	selectedRoomShader.Destroy(gl);
	transparencyShader.Destroy(gl);
	uniformColorShader.Destroy(gl);
}

void glBindUniformBlocks(QOpenGLFunctions * gl, GLuint program)
{
	auto glBindUniformBlock = [gl, program](const char * name, int index)
	{
		int block_id = gl->glGetUniformBlockIndex(program, name);

		if(block_id >= 0)
			gl->glUniformBlockBinding(program, block_id, index);
	};

	glBindUniformBlock("Matrices", 0);
}
