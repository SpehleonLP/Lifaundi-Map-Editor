#ifndef ARROWSHADER_H
#define ARROWSHADER_H
#include "qt-gl/simpleshaderbase.h"
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>
#include <vector>

struct Arrow
{
	glm::ivec2	position;
	glm::i16vec2  rotation;
};

class ArrowShader : public ShaderBase
{
public:
static ArrowShader Shader;
#ifndef PRODUCTION_BUILD
	static const char Vertex[];
	static const char Fragment[];
#endif

	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);
	void Destroy(QOpenGLFunctions * gl);

	void operator()(QOpenGLFunctions * gl, std::vector<Arrow> const& arrows, glm::vec4 color);

private:
	uniform_t u_vertices;
	uniform_t u_color;

	uint32_t  _vao;
	uint32_t  _vbo[3];
	uint32_t  _count;
};

#endif // ARROWSHADER_H
