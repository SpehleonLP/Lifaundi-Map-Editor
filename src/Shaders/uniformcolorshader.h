#ifndef UNIFORMCOLORSHADER_H
#define UNIFORMCOLORSHADER_H
#include "qt-gl/simpleshaderbase.h"

class UniformColorShader : public ShaderBase
{
public:
static UniformColorShader Shader;
	void Bind(QOpenGLFunctions*,float r, float green, float blue, float alpha);

	void Initialize(QOpenGLFunctions * gl);
	void Destroy(QOpenGLFunctions * gl);

private:
	int32_t u_color;
};

#endif // UNIFORMCOLORSHADER_H
