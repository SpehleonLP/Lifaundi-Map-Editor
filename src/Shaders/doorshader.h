#ifndef DOORSHADER_H
#define DOORSHADER_H
#include "../glprogram.h"


class DoorShader : public glProgram
{
public:
static DoorShader Shader;

    void Bind(GLViewWidget * gl, int selected_door_type);

private:
    void construct(GLViewWidget * gl);

	int32_t u_screensize;
	int32_t u_vertices;
	int32_t u_activeType;
};

#endif // DOORSHADER_H
