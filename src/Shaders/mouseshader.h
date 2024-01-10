#ifndef MOUSESHADER_H
#define MOUSESHADER_H
#include "qt-gl/simpleshaderbase.h"
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>


class MouseShader : public ShaderBase
{
public:
static MouseShader Shader;
	void operator()(QOpenGLFunctions * gl, glm::ivec2 position, uint32_t size = 5, glm::vec4 color = glm::ivec4(1, 0, 1, 1));

	void Initialize(QOpenGLFunctions * gl);
	void Destroy(QOpenGLFunctions * gl);
private:

	int32_t u_position;
	int32_t u_color;
};

#endif // MOUSESHADER_H
