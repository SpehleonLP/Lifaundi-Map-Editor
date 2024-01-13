#ifndef TRANSPARENCYSHADER_H
#define TRANSPARENCYSHADER_H
#include "qt-gl/simpleshaderbase.h"
#include <atomic>

class Shaders;

class TransparencyShader : public ShaderBase
{
public:
#ifndef PRODUCTION_BUILD
	static const char Vertex[];
	static const char Fragment[];
#endif


	void operator()(Shaders * gl);
	void Bind(QOpenGLFunctions * gl);
	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source);

private:
};

#endif // TRANSPARENCYSHADER_H
