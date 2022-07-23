#ifndef BLITSHADER_H
#define BLITSHADER_H
#include "../glprogram.h"
#include <glm/mat4x4.hpp>

enum class BackgroundLayer : char;

class BlitShader : public glProgram
{
public:
static BlitShader Shader;
    void bind(GLViewWidget *, BackgroundLayer layer);
    void render(GLViewWidget *);

	void AddRef();
    void Release(GLViewWidget * gl);

private:
    void construct(GLViewWidget *);

	int32_t u_projection;
	int32_t u_modelview;
	int32_t u_texture;
	int32_t u_layer;
};

#endif // TRANSPARENCYSHADER_H
