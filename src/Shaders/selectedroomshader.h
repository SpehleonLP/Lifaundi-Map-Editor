#ifndef SELECTEDROOMSHADER_H
#define SELECTEDROOMSHADER_H
#include "../glprogram.h"

class SelectedRoomShader  : public glProgram
{
public:
static SelectedRoomShader Shader;
    void Render(GLViewWidget * gl, uint32_t faces);

private:
    void construct(GLViewWidget *) override;
};

#endif // SELECTEDROOMSHADER_H
