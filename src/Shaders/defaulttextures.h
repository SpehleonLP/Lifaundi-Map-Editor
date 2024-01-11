#ifndef DEFAULTTEXTURES_H
#define DEFAULTTEXTURES_H
#include <atomic>

class Shaders;

class DefaultTextures
{
public:
	typedef class QOpenGLFunctions_4_5_Core QOpenGLFunctions;

	~DefaultTextures();

	uint32_t GetWhiteTexture(QOpenGLFunctions*) const { return _textures[WhiteTexture]; }
	uint32_t GetNormalTexture(QOpenGLFunctions*) const { return _textures[WhiteTexture]; }
	uint32_t GetTransparencyTexture(QOpenGLFunctions*) const { return _textures[TransparencyTexture]; }
	uint32_t GetNoiseTexture(QOpenGLFunctions*) const { return _textures[WhiteTexture]; }

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

	uint32_t _textures[TotalTextures]{};
};

#endif // DEFAULTTEXTURES_H
