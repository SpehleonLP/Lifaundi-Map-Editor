#ifndef DEFAULTTEXTURES_H
#define DEFAULTTEXTURES_H
#include <atomic>

typedef class QOpenGLFunctions_4_5_Core QOpenGLFunctions;

class GLViewWidget;

class DefaultTextures
{
public:
    uint32_t GetWhiteTexture(GLViewWidget*);
    uint32_t GetNormalTexture(GLViewWidget*);
	uint32_t GetNoiseTexture(GLViewWidget*);

	void Initialize(QOpenGLFunctions * gl);
	void Destroy(QOpenGLFunctions * gl);

private:
	enum
	{
		WhiteTexture,
		NormalTexture,
		TransparencyTexture,
		NoiseTexture,
		TotalTextures
	};

	uint32_t textures[TotalTextures]{};
};

#endif // DEFAULTTEXTURES_H
