#ifndef ROOMOUTLINESHADER_H
#define ROOMOUTLINESHADER_H
#include "qt-gl/simpleshaderbase.h"

class RoomOutlineShader : public ShaderBase
{
public:
static RoomOutlineShader Shader;

	void operator()(QOpenGLFunctions * gl, uint32_t faces, bool selected);

	void Initialize(QOpenGLFunctions * gl);
	void Destroy(QOpenGLFunctions * gl);

private:
	int32_t u_vertices;
	int32_t u_selected;
	int32_t u_renderSelected;
};

#endif // ROOMOUTLINESHADER_H
