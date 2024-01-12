#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H
#include "Support/shared_array.hpp"
#include "enums.hpp"
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_precision.hpp>
#include <atomic>
#include <memory>
#include <mutex>
#include <cstddef>
#include <cstdint>
#include <climits>

class QWidget;
class Shaders;

//TODO: atlas textures with GL_TEXTURE_2D_ARRAY

class BackgroundImage
{
public:
	enum { NoLayers = 5, MAX_ARRAY_LAYERS=256, BASE_LEVEL=1, MAX_LEVEL=2 };

	const static uint32_t g_ImageFormat[NoLayers];
	const static uint8_t g_BytesPerBlock[NoLayers];

	enum class Type : unsigned char
	{
		Creatures1,
		Creatures2,
		Creatures3,
		Lifaundi,
		Image,
	};

	static Type GetType(std::string const& filename);

	BackgroundImage() = default;
    BackgroundImage(Shaders * shaders, std::string const& filename);
	~BackgroundImage();

    void Release(Shaders * shaders);
    void Render(Shaders * gl);

	uint32_t   noTextures() const { return ((size() + MAX_ARRAY_LAYERS-1) / MAX_ARRAY_LAYERS); }
	glm::ivec2 dimensions() const { return pixels.x == 0? glm::ivec2(SHRT_MAX, SHRT_MAX) : glm::ivec2(pixels); }
	glm::ivec4 AABB() const { return {-dimensions()/2, dimensions() - dimensions()/2}; }
	uint32_t  noTiles() const { return _noTiles; }
	int        size() const { return (int)tiles.x * tiles.y; }
	int        width() const { return pixels.x; }
	int        height() const { return pixels.y; }

    void CreateVBO(Shaders * gl);

	std::string const& GetFilename() const { return m_filename; }
	Type               GetType()     const { return m_type; }

	void SetBackgroundLayer(Shaders * shaders, BackgroundLayer);

	shared_array<uint32_t> depth() const { return m_depth; };
	uint32_t depthTileBuffer() const { return _depthTileBuffer; }

private:
	shared_array<uint32_t> LoadLifaundiLayer(Shaders * shaders, std::ifstream & file, BackgroundLayer layer) const;

	void LoadLifaundi(Shaders * shaders, std::ifstream file);
	void LoadSpr(Shaders * shaders, std::ifstream file);
	void LoadBlk(Shaders * shaders, std::ifstream file);
	void LoadS16(Shaders * shaders, std::ifstream file);
	void LoadImage(Shaders * shaders, std::string const& filename);
	void ImportS16Frames(Shaders * shaders, std::ifstream file, uint32_t no_tiles, uint32_t gl_type);

    std::string		m_filename;
	Type			m_type;
	BackgroundLayer m_layer{BackgroundLayer::None};

	bool			havePixelSize() const { return (version&0xFF) > 1; }
	bool			isLz4Compressed() const { return (version&0xFF) > 3; }
	bool			isUnlit() const { return !!(version & 0x8000); }
	bool			isMetallicRoughnessHalfSize() const { return !(version & 0x4000); }
	bool			haveMetallic() const { return !(version & 0x2000); }
	bool			haveRoughness() const { return !(version & 0x1000); }

	short			version{};
	glm::u8vec2		tiles{0, 0};
	glm::u16vec2	pixels{0, 0};
	glm::u16vec2	tile_size{256, 256};

	std::unique_ptr<uint16_t[]> m_flags;
	shared_array<uint32_t> m_textures;
	shared_array<uint32_t> m_depth;

	size_t m_texels{};
	size_t m_offset{};

	uint32_t m_vao{};
	uint32_t m_vbo{};
	uint32_t m_noQuads{};

// ivec4 for each tile's position
	uint32_t _depthTileBuffer{};
	uint32_t _noTiles{};
//	TransparencyShader m_transparencyShader;
};


#endif // BACKGROUNDIMAGE_H
