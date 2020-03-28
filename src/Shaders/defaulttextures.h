#ifndef DEFAULTTEXTURES_H
#define DEFAULTTEXTURES_H
#include <atomic>

class GLViewWidget;

class DefaultTextures
{
public:
	static DefaultTextures & GetDefaultTextures()
	{
		static DefaultTextures defaults;
		return defaults;
	}

	~DefaultTextures();

	inline void AddRef() { ++refCount; }
    inline void Release(GLViewWidget* gl) { if(--refCount == 0) destroyTextures(gl); }

    uint32_t GetWhiteTexture(GLViewWidget*);
    uint32_t GetNormalTexture(GLViewWidget*);
	uint32_t GetNoiseTexture(GLViewWidget*);

private:
	enum
	{
		WhiteTexture,
		NormalTexture,
		TransparencyTexture,
		NoiseTexture,
		TotalTextures
	};

    std::atomic<int> refCount{};
    uint32_t textures[TotalTextures]{};

    DefaultTextures() = default;

    void createTextures(GLViewWidget*);
    void destroyTextures(GLViewWidget*);
};

#endif // DEFAULTTEXTURES_H
