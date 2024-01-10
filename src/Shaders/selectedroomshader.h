#ifndef SELECTEDROOMSHADER_H
#define SELECTEDROOMSHADER_H
#include "qt-gl/simpleshaderbase.h"

class SelectedRoomShader  : public ShaderBase
{
public:
static SelectedRoomShader Shader;
	void operator()(QOpenGLFunctions * gl, uint32_t faces);

	void Initialize(QOpenGLFunctions * gl);
	void Destroy(QOpenGLFunctions * gl);
};

#endif // SELECTEDROOMSHADER_H
