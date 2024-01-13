#ifndef BLITSHADER_H
#define BLITSHADER_H
#include "qt-gl/simpleshaderbase.h"
#include <glm/mat4x4.hpp>

enum class BackgroundLayer : char;

class DefaultVAOs;

class BlitShader : public ShaderBase
{
public:
#ifndef PRODUCTION_BUILD
	static const char Vertex[];
	static const char Fragment[];
#endif

	void Bind(QOpenGLFunctions *, BackgroundLayer layer, glm::uvec2 range);

	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);

private:

	uniform_t u_projection;
	uniform_t u_modelview;
	uniform_t u_texture;
	uniform_t u_layer;
	uniform_t u_range;
};

#endif // TRANSPARENCYSHADER_H
