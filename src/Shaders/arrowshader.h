#ifndef ARROWSHADER_H
#define ARROWSHADER_H
#include "../glprogram.h"
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>

class ArrowShader : public glProgram
{
public:
static ArrowShader Shader;

	struct Vertex
	{
		glm::ivec2 position;
		glm::i16vec2  rotation;
	};

    void Render(GLViewWidget* gl, uint32_t size);

private:
    void construct(GLViewWidget* gl);

	int32_t u_vertices;
};

#endif // ARROWSHADER_H
