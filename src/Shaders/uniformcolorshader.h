#ifndef UNIFORMCOLORSHADER_H
#define UNIFORMCOLORSHADER_H
#include "../glprogram.h"
#include <atomic>


class UniformColorShader : public glProgram
{
public:
static UniformColorShader Shader;

    void Bind(GLViewWidget*,float r, float green, float blue, float alpha);

private:
    void construct(GLViewWidget*);

	int32_t u_color;
};

#endif // UNIFORMCOLORSHADER_H
