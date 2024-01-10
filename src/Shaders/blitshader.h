#ifndef BLITSHADER_H
#define BLITSHADER_H
#include "qt-gl/simpleshaderbase.h"
#include <glm/mat4x4.hpp>

enum class BackgroundLayer : char;

class BlitShader : public ShaderBase
{
public:
static BlitShader Shader;
	void Bind(QOpenGLFunctions *, BackgroundLayer layer);
	void operator()(QOpenGLFunctions *);

	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);
	void Destroy(QOpenGLFunctions * gl);

private:
	uniform_t u_projection;
	uniform_t u_modelview;
	uniform_t u_texture;
	uniform_t u_layer;
};

#endif // TRANSPARENCYSHADER_H
