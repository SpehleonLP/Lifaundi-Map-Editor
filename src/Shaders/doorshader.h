#ifndef DOORSHADER_H
#define DOORSHADER_H
#include "qt-gl/simpleshaderbase.h"


class DoorShader : public ShaderBase
{
public:
#ifndef PRODUCTION_BUILD
	static const char Vertex[];
	static const char Fragment[];
#endif

	void Bind(QOpenGLFunctions * gl, int selected_door_type);

	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source);

private:
	uniform_t u_activeType;
};

#endif // DOORSHADER_H
