#ifndef TRANSPARENCYSHADER_H
#define TRANSPARENCYSHADER_H
#include "../glprogram.h"
#include <atomic>

class TransparencyShader : public glProgram
{
public:
static TransparencyShader Shader;
    void bind(GLViewWidget* gl);

    void AddRef();
    void Release(GLViewWidget* gl);

private:
	std::atomic<int> refCount{0};

    void construct(GLViewWidget* gl);
};

#endif // TRANSPARENCYSHADER_H
