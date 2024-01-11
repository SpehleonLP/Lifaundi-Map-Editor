#ifndef ROOMOUTLINESHADER_H
#define ROOMOUTLINESHADER_H
#include "qt-gl/simpleshaderbase.h"

class RoomOutlineShader : public ShaderBase
{
public:
#ifndef PRODUCTION_BUILD
	static const char Vertex[];
	static const char Fragment[];
#endif

	void operator()(QOpenGLFunctions * gl, uint32_t faces, bool selected);
	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source);

private:
	uniform_t u_vertices;
	uniform_t u_selected;
	uniform_t u_renderSelected;
};

#endif // ROOMOUTLINESHADER_H
