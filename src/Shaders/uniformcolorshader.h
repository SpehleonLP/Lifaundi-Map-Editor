#ifndef UNIFORMCOLORSHADER_H
#define UNIFORMCOLORSHADER_H
#include "qt-gl/simpleshaderbase.h"

class UniformColorShader : public ShaderBase
{
public:
#ifndef PRODUCTION_BUILD
	static const char Vertex[];
	static const char Fragment[];
#endif

	void Bind(QOpenGLFunctions*,float r, float green, float blue, float alpha);

	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source);

private:
	uniform_t u_color;
};

#endif // UNIFORMCOLORSHADER_H
