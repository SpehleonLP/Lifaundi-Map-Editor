#ifndef SELECTEDROOMSHADER_H
#define SELECTEDROOMSHADER_H
#include "qt-gl/simpleshaderbase.h"

class SelectedRoomShader  : public ShaderBase
{
public:
#ifndef PRODUCTION_BUILD
	static const char Vertex[];
	static const char Fragment[];
#endif

	void operator()(QOpenGLFunctions * gl, uint32_t faces);

	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source);
};

#endif // SELECTEDROOMSHADER_H
