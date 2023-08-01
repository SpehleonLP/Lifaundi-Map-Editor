#ifndef MOUSESHADER_H
#define MOUSESHADER_H
#include "../glprogram.h"
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>


class MouseShader : public glProgram
{
public:
static MouseShader Shader;
    void Render(GLViewWidget* gl, glm::ivec2 position, uint32_t size = 5, glm::vec4 color = glm::ivec4(1, 0, 1, 1));
	
private:
    void construct(GLViewWidget*gl);

	int32_t u_position;
	int32_t u_color;
};

#endif // MOUSESHADER_H
