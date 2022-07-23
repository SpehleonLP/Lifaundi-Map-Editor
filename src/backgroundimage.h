#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H
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
class GLViewWidget;

//TODO: atlas textures with GL_TEXTURE_2D_ARRAY

class BackgroundImage
{
public:
	enum { NoLayers = 5 };

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
    BackgroundImage(GLViewWidget * gl, std::string const& filename);
	~BackgroundImage();

    void Release(GLViewWidget * gl);
    void Render(GLViewWidget* gl);

	glm::ivec2 dimensions() const { return pixels.x == 0? glm::ivec2(SHRT_MAX, SHRT_MAX) : glm::ivec2(pixels); }
	int        size() const { return (int)tiles.x * tiles.y; }
	int        width() const { return pixels.x; }
	int        height() const { return pixels.y; }

    void CreateVBO(GLViewWidget* gl);

	std::string const& GetFilename() const { return m_filename; }
	Type               GetType()     const { return m_type; }

	void SetBackgroundLayer(GLViewWidget * gl, BackgroundLayer);

private:
	void LoadLifaundi(GLViewWidget * gl, std::ifstream file, BackgroundLayer layer);
	void LoadSpr(GLViewWidget * gl, std::ifstream file);
	void LoadBlk(GLViewWidget * gl, std::ifstream file);
	void LoadS16(GLViewWidget * gl, std::ifstream file);
	void LoadImage(GLViewWidget * gl, std::string const& filename);
	void ImportS16Frames(GLViewWidget * gl, std::ifstream file, uint32_t no_tiles, uint32_t gl_type);

    std::string		m_filename;
	Type			m_type;
	BackgroundLayer m_layer{BackgroundLayer::None};

	short			version{};
	glm::u8vec2		tiles{0, 0};
	glm::u16vec2	pixels{0, 0};
	glm::u16vec2	tile_size{256, 256};

	std::unique_ptr<uint16_t[]> m_flags;
	std::unique_ptr<uint32_t[]> m_textures;

	size_t m_texels{};
	size_t m_offset{};

	uint32_t m_vao{};
	uint32_t m_vbo{};

//	TransparencyShader m_transparencyShader;
};


#endif // BACKGROUNDIMAGE_H
