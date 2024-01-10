#ifndef GLBLITVAO_H
#define GLBLITVAO_H
#include <cstdint>

typedef class QOpenGLFunctions_4_5_Core QOpenGLFunctions;

class glDefaultVAOs
{
public:
	void Initialize(QOpenGLFunctions * gl);
	void Destroy(QOpenGLFunctions * gl);

	void BindVAO(QOpenGLFunctions * gl);

	void RenderPoint(QOpenGLFunctions * gl);
	void RenderSquare(QOpenGLFunctions * gl);
	void RenderCube(QOpenGLFunctions * gl);

private:
	uint32_t _vao, _vbo;
};


#endif // GLBLITVAO_H
