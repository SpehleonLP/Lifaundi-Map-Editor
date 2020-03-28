#ifndef ROOMOUTLINESHADER_H
#define ROOMOUTLINESHADER_H
#include "../glprogram.h"

class RoomOutlineShader : public glProgram
{
public:
static RoomOutlineShader Shader;

    void Render(GLViewWidget*gl, uint32_t faces, bool selected);

private:
    void construct(GLViewWidget*gl);

	int32_t u_vertices;
	int32_t u_selected;
	int32_t u_renderSelected;
};

#endif // ROOMOUTLINESHADER_H
