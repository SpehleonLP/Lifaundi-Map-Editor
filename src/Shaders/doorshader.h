#ifndef DOORSHADER_H
#define DOORSHADER_H
#include "qt-gl/simpleshaderbase.h"


class DoorShader : public ShaderBase
{
public:
static DoorShader Shader;

	void Bind(QOpenGLFunctions * gl, int selected_door_type);

	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source);

private:
	int32_t u_screensize;
	int32_t u_vertices;
	int32_t u_activeType;
};

#endif // DOORSHADER_H
