#include "backgroundimage.h"
#include "enums.hpp"
#include <QProgressDialog>
#include "src/Shaders/shaders.h"
#include <QOpenGLFunctions_4_5_Core>
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <lz4/lib/lz4.h>

const uint32_t BackgroundImage::g_ImageFormat[NoLayers] =
{
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,	//basecolor
	GL_COMPRESSED_RG_RGTC2,	//roughness
	GL_COMPRESSED_RG_RGTC2,	//normal
	GL_COMPRESSED_RED_RGTC1,//occlusion
	GL_R16, //depth
};

const uint8_t BackgroundImage::g_BytesPerBlock[NoLayers] =
{
	8,	//basecolor
	16,	//roughness
	16,	//normal
	8, //occlusion
	2, //depth
};

static const unsigned int texture_offset[(int)BackgroundLayer::Total] =
{
	Background::Blocks * (int)BaseColorOffset,
	Background::Blocks * (int)RoughOffset,
	Background::Blocks * (int)NormalOffset,
	Background::Blocks * (int)OcclusionOffset,
	Background::Blocks * (int)DepthOffset,
};


#if defined(__GNUC__) || defined(__clang__)
#define __unused __attribute__((unused))
#else
#define __unused
#endif

std::string ToHex(const uint8_t * s, int length, bool upper_case  = true)
{
    std::ostringstream ret;

    for (int i = 0; i < length; ++i)
        ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << (int)s[i];

    return ret.str();
}

static uint8_t * _interleave_DXT1(uint8_t * dst, uint8_t * src, int blocks);
static uint8_t * interleave_DXT5(uint8_t * dst, uint8_t * src, int blocks);
static uint8_t * interleave_BC5(uint8_t * dst, uint8_t * src, int blocks);

BackgroundImage::Type BackgroundImage::GetType(std::string const& filename)
{
	size_t dot = filename.find_last_of('.');

	if(dot == std::string::npos)
		return Type::Lifaundi;

	std::string extension = filename.substr(dot+1);

	for(char & c : extension)
		c = tolower(c);

	if(extension == "spr")
		return Type::Creatures1;
	if(extension == "s16")
		return Type::Creatures2;
	if(extension == "blk")
		return Type::Creatures3;
	if(extension == "lf_bck")
		return Type::Lifaundi;

	return Type::Image;
}


BackgroundImage::BackgroundImage(Shaders * shaders, std::string const& filename) :
	m_filename(filename)
{
	std::ifstream file(m_filename, std::ios::binary);

	if(!file.is_open())
		throw std::system_error(errno, std::system_category(), m_filename);

    file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

	switch((m_type = GetType(filename)))
	{
	case Type::Creatures1:
		LoadSpr(shaders, std::move(file));
		break;
	case Type::Creatures2:
		LoadS16(shaders, std::move(file));
		break;
	case Type::Creatures3:
		LoadBlk(shaders, std::move(file));
		break;
	case Type::Lifaundi:
		LoadLifaundi(shaders, std::move(file));
		break;
	case Type::Image:
		file.close();
		LoadImage(shaders, filename);
		break;
	}
}

BackgroundImage::~BackgroundImage()
{
}

void BackgroundImage::Release(Shaders * shaders)
{
	if(m_textures.size())
		shaders->gl->glDeleteTextures(m_textures.size(), &m_textures[0]);

	if(m_depth.size() && m_depth != m_textures)
		shaders->gl->glDeleteTextures(m_depth.size(), &m_depth[0]);

	shaders->gl->glDeleteVertexArrays(1, &m_vao);
	shaders->gl->glDeleteBuffers(1, &m_vbo);

	if(_depthTileBuffer)
		shaders->gl->glDeleteBuffers(1, &_depthTileBuffer);
}

void BackgroundImage::Render(Shaders * shaders,  glm::uvec2 range)
{
	auto gl = shaders->gl;

//	shaders->transparencyShader(shaders);

	CreateVBO(shaders);

	shaders->blitShader.Bind(shaders->gl, m_layer, range);

	gl->glBindVertexArray(m_vao);
	gl->glActiveTexture(GL_TEXTURE0);

	for(uint32_t i = 0, texture = 0; i < m_noQuads; i += MAX_ARRAY_LAYERS, ++texture)
	{
		auto no_tris = 6 * std::min<size_t>(m_noQuads - i, MAX_ARRAY_LAYERS);

		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[texture]);
		gl->glDrawArrays(GL_TRIANGLES, i*6, no_tris);
	}
}

#if 0
bool BackgroundImage::LoadTextures(QWidget *)
{
	if(m_textures == nullptr)
		return false;



	std::unique_ptr<uint8_t[]> deinterleave(new uint8_t[128*128]);

	glGenTextures(1, &m_texture);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, tiles.x*128, tiles.y*128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	;

	for(size_t i = 0; i < size(); ++i)
	{
		if(m_offsets[i*3+1] == m_offsets[i*3+2])
			continue;

		int tile_y = i / tiles.x;
		int tile_x = i % tiles.x;

		size_t address = m_offsets[i*3+1] - m_texel_offset;
		int    blocks  = 4 * m_dimensions[i].x1 * m_dimensions[i].y1;

		interleave_DXT5(&deinterleave[0], &m_image[address], blocks);

		glCompressedTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			tile_x * 128 + m_dimensions[i].x0,
			tile_y * 128 + m_dimensions[i].y0,
			m_dimensions[i].x1 * 8,
			m_dimensions[i].y1 * 8,
			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
			128*128,
			&deinterleave[0]);

		;
	}

	CreateVBO();
	;

	return true;
}
#endif

immutable_array<glm::u8vec2> BackgroundImage::idToTile() const
{
	if(_idToTile != nullptr)
		return _idToTile;

	std::vector<glm::u8vec2> r(size());

	int counter[2] = {0, 0};
	for(auto i = 0; i < size(); ++i)
	{
		if(m_flags.size() && m_flags[i] == 0)
		{
			r[i] = glm::u8vec2(255, 255);
			continue;
		}

		r[i] = glm::u8vec2(counter[0], counter[1]++);

		if(counter[1] == 256)
		{
			counter[0] += 1;
			counter[1] = 0;
		}
	}

	_idToTile = shared_array<glm::u8vec2>::FromArray(r.data(), r.size());
	return _idToTile;
}

void BackgroundImage::CreateVBO(Shaders * shaders)
{
	if(m_vao != 0)
		return;

	auto gl = shaders->gl;

	struct vertex
	{
		glm::ivec3   position;
		glm::u16vec2 texCoord;
	};

	std::unique_ptr<vertex[]> buffer(new vertex[size()*6 + 6]);

	vertex * corner = &buffer[0];

	for(uint8_t y = 0; y < _tiles.y; ++y)
	{
		for(uint8_t x = 0; x < _tiles.x; ++x)
		{
			auto i          = (int) y * _tiles.x + x;

			if(m_flags.size() && m_flags[i] == 0)
				continue;

			if(tile_size.x == 256)
			{
				corner[0].texCoord = glm::u16vec2(0, 0);
				corner[1].texCoord = glm::u16vec2(1, 0);
				corner[2].texCoord = glm::u16vec2(0, 1);
				corner[5].texCoord = glm::u16vec2(1, 1);
			}
			else
			{
				corner[0].texCoord = glm::u16vec2(0, 1);
				corner[1].texCoord = glm::u16vec2(1, 1);
				corner[2].texCoord = glm::u16vec2(0, 0);
				corner[5].texCoord = glm::u16vec2(1, 0);
			}

			glm::ivec3 offset(x * tile_size.x - pixels.x/2, y * tile_size.y - pixels.y/2, 0);

			corner[0].position = offset + glm::ivec3(0, 0, 0);
			corner[1].position = offset + glm::ivec3(tile_size.x, 0, 0);
			corner[2].position = offset + glm::ivec3(0, tile_size.y, 0);
			corner[5].position = offset + glm::ivec3(tile_size.x, tile_size.y, 0);

			corner[3] = corner[2];
			corner[4] = corner[1];

			corner += 6;
		}
	}

	{
		glm::ivec3 half_size(pixels.x/2, pixels.y/2, 0);

		corner[0].position = glm::ivec3(-half_size.x, -half_size.y, 0);
		corner[0].texCoord = glm::u16vec2(0, 0);

		corner[1].position = glm::ivec3(half_size.x, -half_size.y, 0);
		corner[1].texCoord = glm::u16vec2(1, 0);

		corner[2].position = glm::ivec3(-half_size.x, half_size.y, 0);
		corner[2].texCoord = glm::u16vec2(0, 1);

		corner[5].position = glm::ivec3(half_size.x, half_size.y, 0);
		corner[5].texCoord = glm::u16vec2(1, 1);

		corner[3] = corner[2];
		corner[4] = corner[1];

		corner += 6;
	}

    gl->glGenVertexArrays(1, &m_vao);
    gl->glGenBuffers(1, &m_vbo);

    gl->glBindVertexArray(m_vao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	gl->glBufferData(GL_ARRAY_BUFFER, (uint8_t*)corner - (uint8_t*)&buffer[0], &buffer[0], GL_STATIC_DRAW);

	m_noQuads = (corner - &buffer[0])/6 - 1;

	//position
    gl->glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, sizeof(vertex), (void*) offsetof(vertex, position));

//	glVertexAttribPointer(2, 4, GL_BYTE, GL_FALSE, sizeof(gltfVertex), (void*) 8);
	//texcoord0
    gl->glVertexAttribPointer(3, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*) offsetof(vertex, texCoord));

    gl->glEnableVertexAttribArray(0);
    gl->glEnableVertexAttribArray(3);


}

template<int bytes, int stride>
static uint8_t * interleave_primitive(uint8_t * dst, uint8_t * src, int blocks)
{
	for(int j = 0; j < blocks; ++j)
	{
		for(int i = 0; i < bytes; ++i)
		{
			dst[j * stride + i] = *src;
			++src;
		}
	}

	return src;
}

template<int stride>
static uint8_t * interleave_DXT1(uint8_t * dst, uint8_t * src, int blocks)
{
	src = interleave_primitive<2, stride>(dst+0, src, blocks);
	src = interleave_primitive<2, stride>(dst+2, src, blocks);
	src = interleave_primitive<4, stride>(dst+4, src, blocks);

	return src;
}


template<int stride=8>
static uint8_t * interleave_BC4(uint8_t * dst, uint8_t * src, int blocks)
{
	src = interleave_primitive<1, stride>(dst+0, src, blocks);
	src = interleave_primitive<1, stride>(dst+1, src, blocks);
	src = interleave_primitive<6, stride>(dst+2, src, blocks);

	return src;
}

static __unused uint8_t * interleave_DXT5(uint8_t * dst, uint8_t * src, int blocks)
{
	src = interleave_BC4 <16>(dst  , src, blocks);
	src = interleave_DXT1<16>(dst+8, src, blocks);

	return src;
}

static uint8_t * _interleave_DXT1(uint8_t * dst, uint8_t * src, int blocks)
{
	return interleave_DXT1<8>(dst, src, blocks);
}

static __unused uint8_t * interleave_BC5(uint8_t * dst, uint8_t * src, int blocks)
{
	src = interleave_BC4<16>(dst  , src, blocks);
	src = interleave_BC4<16>(dst+8, src, blocks);

	return src;
}

static __unused uint8_t * interleave_Depth(uint8_t * dst, uint8_t * src, int blocks)
{
	int pixels = blocks * sizeof(DepthBlock) / sizeof(uint16_t);
	assert(pixels * sizeof(uint16_t) == blocks * sizeof(DepthBlock));

	{
		int8_t * src0 = (int8_t*)src;
		for(int i = 1; i < blocks*2-1; ++i)
		{
			src0[i] = src0[i] + src0[i-1];
		}
	}

	for(int i = 0; i < pixels; ++i)
	{
		dst[i*2+0] = (int)src[i];
	}

	for(int i = 0; i < pixels; ++i)
	{
		dst[i*2+1] = (int)src[i+pixels];
	}


//	memcpy(dst, src, pixels * sizeof(uint16_t));

	/*



	memcpy(dst, ptr, blocks* sizeof(uint16_t));*/

	return src+blocks*2;
}


void BackgroundImage::SetBackgroundLayer(Shaders * shaders, BackgroundLayer layer)
{
//don't try to load something we already have.
	if(m_layer == layer || m_type != Type::Lifaundi || isUnlit())
	{
		return;
	}

	std::ifstream file(m_filename, std::ios::binary);

	if(!file.is_open())
		throw std::system_error(errno, std::system_category(), m_filename);

    file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

	if(m_textures.size() && m_textures != m_depth)
	{
		shaders->gl->glDeleteTextures(m_textures.size(), &m_textures[0]);
	}

	if(layer == BackgroundLayer::Depth)
	{
		m_textures = m_depth;
	}
	else
	{
		m_textures = LoadLifaundiLayer(shaders, file, layer);
	}

	file.close();
	m_layer = layer;
}

struct mip_offsets { uint32_t mip0, mip1, mip2; };
static_assert(sizeof(mip_offsets) == sizeof(uint32_t)*3);

void BackgroundImage::LoadLifaundi(Shaders * shaders, std::ifstream file)
{
	char title[4];

	file.read(&title[0], sizeof(title));
	file.read((char*)&version, sizeof(version));
	file.read((char*)&_tiles, sizeof(_tiles));

	if(!havePixelSize())
		pixels = glm::u16vec2(_tiles) * (uint16_t) 256;
	else
		file.read((char*)&pixels, sizeof(pixels));

	if(m_flags == nullptr)
	{
		m_flags		= shared_array<uint16_t>(size());
	}

	if(memcmp(title, "lbck", 4))
		throw std::runtime_error("Bad File");

	file.read((char*)&m_flags[0], size() * sizeof(m_flags[0]));
	m_offset = file.tellg();

	if(isUnlit() == false)
		m_depth = LoadLifaundiLayer(shaders, file, BackgroundLayer::Depth);
	else
		m_textures = LoadLifaundiLayer(shaders, file, BackgroundLayer::BaseColor);

	file.close();

	if(m_depth == nullptr)
		return;

	std::vector<glm::ivec4> tile_data(256 * m_depth.size());
	glm::ivec2 tileSize = tile_size;
	auto halfSize = glm::ivec2(pixels)/2;

	for(auto i = 0u; i < m_flags.size(); ++i)
	{
		if(m_flags.size() && m_flags[i] == 0)
			continue;

		auto x = i % _tiles.x;
		auto y = i / _tiles.x;

		glm::ivec2 offset = {x, y};
		offset = offset * tileSize - halfSize;
		tile_data[_noTiles++] = glm::ivec4(offset, offset + tileSize);
	}

	memset(&tile_data[_noTiles], 0, (tile_data.size() - _noTiles) * sizeof(glm::ivec4));

	auto gl = shaders->gl;
	gl->glCreateBuffers(1, &_depthTileBuffer);
	gl->glNamedBufferStorage(_depthTileBuffer, tile_data.size() * sizeof(glm::ivec4), tile_data.data(), 0);
}

shared_array<uint32_t> BackgroundImage::LoadLifaundiLayer(Shaders * shaders, std::ifstream & file, BackgroundLayer layer) const
{
	auto gl = shaders->gl;

	file.seekg(m_offset, std::ifstream::beg);
	auto offsets = std::vector<mip_offsets>(size()+1);
	file.read((char*)&offsets[0], (size()*3 + 1) * sizeof(uint32_t));

	std::unique_ptr<uint8_t[]> buffer(new uint8_t[TILE_BYTES]);
	std::unique_ptr<uint8_t[]> buffer2(new uint8_t[TILE_BYTES]);

	auto textures = shared_array<uint32_t>(noTextures(), 0);

	gl->glGenTextures(textures.size(), &textures[0]);

	for(auto i = 0u; i < textures.size(); ++i)
	{
		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, textures[i]);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, BASE_LEVEL);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, MAX_LEVEL);

		if(g_ImageFormat[(int)layer] == GL_R16)
		{
			gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R16, 128, 128, MAX_ARRAY_LAYERS, 0, GL_RED, GL_FLOAT, nullptr);
			gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 2, GL_R16, 64, 64, MAX_ARRAY_LAYERS, 0, GL_RED, GL_FLOAT, nullptr);

		}
		else
		{
			auto image_size = MAX_ARRAY_LAYERS * 128*128 / 16 * g_BytesPerBlock[(int)layer];
			gl->glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 1, g_ImageFormat[(int)layer], 128, 128, MAX_ARRAY_LAYERS, 0, image_size, nullptr);
			gl->glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 2, g_ImageFormat[(int)layer], 64, 64, MAX_ARRAY_LAYERS, 0, image_size >> 2, nullptr);

		}
	}

	for(int i = 0, texture = 0; i < size(); ++i, ++texture)
	{
		if(m_flags[i] == 0)
		{
			--texture;
			continue;
		}

		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, textures[texture / MAX_ARRAY_LAYERS]);
		auto texture_layer = texture % MAX_ARRAY_LAYERS;

		uint32_t * mip = (uint32_t*)&offsets[i];

		int x_offset = 0;
		int y_offset = 0;
		int width    = 256; //(dscr.x1 - dscr.x0) * 16;
		int height   = 256; //(dscr.y1 - dscr.y0) * 16;
		auto area    = width * height;

// deinterleave data
		uint32_t blocks_x = (width) / 4;
		uint32_t blocks_y = (height) / 4;

		for(int mipLevel = BASE_LEVEL; mipLevel <= MAX_LEVEL; ++mipLevel)
		{
			blocks_x >>= 1;
			blocks_y >>= 1;
			auto bytes  = mip[mipLevel+1] - mip[mipLevel];
			auto decompressed_bytes = bytes;

			if(bytes == 0)
				continue;

			assert(bytes <= TILE_BYTES);

			file.seekg(mip[mipLevel], std::ios_base::beg);
			file.read((char*) &buffer[0], bytes);

			if(isLz4Compressed())
			{
				decompressed_bytes = LZ4_decompress_safe(
					(const char *)&buffer[0], /* src */
					(char*)&buffer2[0], /* dst */
					bytes,			/* compressed size */
					TILE_BYTES) ;   /* decompressed capacity */

				if(decompressed_bytes < 0)
					throw std::runtime_error("failed to decompress all data");

				std::swap(buffer, buffer2);
			//	bytes = zlib.total_out;
			}

			const int d0 = 1 << mipLevel;
			const int d1 = d0*d0;

			switch(layer)
			{
			case BackgroundLayer::BaseColor:
				_interleave_DXT1(&buffer2[0], &buffer[0], blocks_x * blocks_y);
				break;
			case BackgroundLayer::Normals:
				interleave_BC5(&buffer2[0], &buffer[texture_offset[(int)BackgroundLayer::Normals] / d1], blocks_x * blocks_y);
				break;
			case BackgroundLayer::MetallicRoughness:
				interleave_BC5(&buffer2[0], &buffer[texture_offset[(int)BackgroundLayer::MetallicRoughness] / d1], blocks_x * blocks_y);
				break;
			case BackgroundLayer::AmbientOcclusion:
				interleave_BC4(&buffer2[0], &buffer[texture_offset[(int)BackgroundLayer::AmbientOcclusion] / d1], blocks_x * blocks_y);
				break;
			case BackgroundLayer::Depth:
				interleave_Depth(&buffer2[0], &buffer[texture_offset[(int)BackgroundLayer::Depth] / d1], blocks_x * blocks_y);
				break;
			default:
				throw std::logic_error("unhandled image format case");
			}


			if(g_ImageFormat[(int)layer] != GL_R16)
			{

				gl->glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY,
					mipLevel,
					0, 0, texture_layer,
					width    / d0,
					height   / d0,
					1,
					g_ImageFormat[(int)layer],
					area / d1 / 16 * g_BytesPerBlock[(int)layer],		//no bytes
					&buffer2[0]);
			}
			else
			{
				gl->glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
					mipLevel,
					0, 0, texture_layer,
					width    / d0,
					height   / d0,
					1,
					GL_RED,
					GL_UNSIGNED_SHORT,	// type
					&buffer2[0]);

			}


		}
	}

	return textures;
}

std::vector<uint16_t> GetC1Pallette();

struct spr_header
{
	uint32_t offset;
	uint16_t width, height;
};

void BackgroundImage::LoadSpr(Shaders * shaders, std::ifstream file)
{
	auto gl = shaders->gl;

	const static uint8_t PALETTE_DTA[256][3] = {
		{0x00,0x00,0x00}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F},
		{0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x04,0x02,0x02},
		{0x05,0x06,0x0A}, {0x06,0x0A,0x04}, {0x06,0x09,0x0C}, {0x0B,0x04,0x02}, {0x0A,0x06,0x09}, {0x0D,0x0A,0x04},
		{0x0C,0x0B,0x0C}, {0x06,0x07,0x11}, {0x05,0x0D,0x15}, {0x06,0x0F,0x18}, {0x09,0x07,0x11}, {0x0B,0x0D,0x12},
		{0x0B,0x0E,0x1A}, {0x07,0x10,0x07}, {0x07,0x10,0x0A}, {0x0D,0x12,0x06}, {0x0D,0x12,0x0B}, {0x0F,0x18,0x06},
		{0x0F,0x18,0x0A}, {0x06,0x10,0x17}, {0x07,0x10,0x19}, {0x0D,0x11,0x14}, {0x0B,0x13,0x1A}, {0x0E,0x18,0x13},
		{0x0F,0x18,0x1C}, {0x12,0x06,0x02}, {0x12,0x07,0x09}, {0x14,0x0B,0x04}, {0x12,0x0D,0x0B}, {0x1A,0x06,0x03},
		{0x1B,0x07,0x09}, {0x1B,0x0C,0x04}, {0x1A,0x0D,0x09}, {0x12,0x0E,0x12}, {0x12,0x0E,0x1A}, {0x1A,0x0D,0x12},
		{0x1D,0x0D,0x1A}, {0x14,0x12,0x05}, {0x14,0x12,0x0C}, {0x14,0x19,0x06}, {0x13,0x1A,0x0B}, {0x1C,0x12,0x05},
		{0x1B,0x13,0x0B}, {0x1C,0x19,0x05}, {0x1D,0x19,0x0C}, {0x13,0x13,0x13}, {0x13,0x15,0x1B}, {0x15,0x19,0x14},
		{0x15,0x19,0x1C}, {0x1A,0x15,0x13}, {0x1A,0x16,0x1A}, {0x1C,0x1A,0x14}, {0x1B,0x1B,0x1B}, {0x0C,0x0F,0x21},
		{0x0E,0x17,0x24}, {0x10,0x0F,0x21}, {0x13,0x16,0x23}, {0x12,0x16,0x2C}, {0x14,0x1A,0x23}, {0x12,0x1B,0x2B},
		{0x19,0x16,0x22}, {0x19,0x17,0x2B}, {0x1B,0x1C,0x23}, {0x1B,0x1D,0x2A}, {0x13,0x17,0x31}, {0x14,0x1D,0x32},
		{0x17,0x1C,0x3B}, {0x1A,0x1E,0x33}, {0x19,0x1E,0x3D}, {0x1A,0x23,0x0D}, {0x17,0x21,0x13}, {0x17,0x20,0x1A},
		{0x1B,0x23,0x13}, {0x1D,0x22,0x1C}, {0x1E,0x29,0x13}, {0x1E,0x29,0x1A}, {0x16,0x20,0x23}, {0x17,0x20,0x2E},
		{0x1C,0x21,0x25}, {0x1D,0x22,0x2B}, {0x1F,0x29,0x23}, {0x1E,0x29,0x2C}, {0x16,0x21,0x33}, {0x16,0x24,0x39},
		{0x16,0x29,0x3C}, {0x1C,0x22,0x33}, {0x1D,0x22,0x3F}, {0x1E,0x28,0x36}, {0x1C,0x29,0x3B}, {0x23,0x06,0x04},
		{0x24,0x07,0x09}, {0x22,0x0D,0x04}, {0x23,0x0D,0x0A}, {0x2B,0x06,0x04}, {0x2B,0x07,0x08}, {0x2A,0x0C,0x04},
		{0x2B,0x0C,0x0A}, {0x26,0x0D,0x12}, {0x23,0x13,0x05}, {0x23,0x14,0x0A}, {0x24,0x1A,0x05}, {0x24,0x1A,0x0C},
		{0x2B,0x14,0x05}, {0x2A,0x15,0x0A}, {0x2C,0x1A,0x05}, {0x2B,0x1B,0x0B}, {0x22,0x15,0x12}, {0x22,0x16,0x1B},
		{0x23,0x1B,0x13}, {0x22,0x1D,0x1B}, {0x2B,0x14,0x12}, {0x2C,0x15,0x19}, {0x2A,0x1D,0x12}, {0x2B,0x1D,0x1A},
		{0x34,0x0B,0x07}, {0x35,0x0D,0x12}, {0x32,0x15,0x05}, {0x32,0x15,0x0A}, {0x33,0x1A,0x05}, {0x33,0x1C,0x0B},
		{0x3A,0x14,0x05}, {0x3A,0x14,0x0B}, {0x3A,0x1D,0x05}, {0x3A,0x1D,0x0A}, {0x33,0x14,0x12}, {0x33,0x15,0x19},
		{0x33,0x1D,0x12}, {0x32,0x1D,0x1A}, {0x3A,0x14,0x14}, {0x3B,0x16,0x18}, {0x3C,0x1C,0x12}, {0x3B,0x1C,0x1C},
		{0x24,0x0F,0x21}, {0x23,0x14,0x21}, {0x21,0x1E,0x24}, {0x21,0x1E,0x2A}, {0x2A,0x1E,0x22}, {0x29,0x1F,0x29},
		{0x20,0x1F,0x31}, {0x34,0x0C,0x20}, {0x36,0x1C,0x22}, {0x3B,0x1D,0x33}, {0x29,0x22,0x0B}, {0x25,0x21,0x14},
		{0x24,0x22,0x1C}, {0x22,0x2B,0x14}, {0x23,0x2B,0x1B}, {0x2C,0x22,0x14}, {0x2B,0x23,0x1B}, {0x2D,0x29,0x14},
		{0x2D,0x2A,0x1C}, {0x27,0x31,0x0F}, {0x29,0x34,0x17}, {0x34,0x22,0x06}, {0x34,0x22,0x0C}, {0x35,0x2A,0x05},
		{0x34,0x2A,0x0B}, {0x3C,0x23,0x05}, {0x3B,0x23,0x0B}, {0x3D,0x2B,0x05}, {0x3D,0x2B,0x0C}, {0x33,0x23,0x13},
		{0x32,0x25,0x1A}, {0x34,0x2A,0x14}, {0x34,0x2A,0x1C}, {0x3B,0x24,0x12}, {0x3B,0x24,0x19}, {0x3C,0x2B,0x13},
		{0x3B,0x2C,0x1B}, {0x34,0x31,0x0E}, {0x3D,0x33,0x03}, {0x3E,0x33,0x0C}, {0x3F,0x3C,0x03}, {0x3F,0x3B,0x0B},
		{0x35,0x31,0x14}, {0x35,0x31,0x1C}, {0x32,0x3D,0x14}, {0x33,0x3D,0x1B}, {0x3E,0x32,0x13}, {0x3D,0x33,0x1B},
		{0x3E,0x3B,0x13}, {0x3F,0x3A,0x1C}, {0x23,0x22,0x24}, {0x23,0x24,0x2B}, {0x24,0x2A,0x24}, {0x25,0x2A,0x2D},
		{0x2A,0x24,0x23}, {0x29,0x26,0x2C}, {0x2C,0x2A,0x24}, {0x2B,0x2A,0x2D}, {0x22,0x25,0x33}, {0x21,0x26,0x3E},
		{0x25,0x29,0x34}, {0x24,0x2A,0x3F}, {0x28,0x27,0x31}, {0x2B,0x2B,0x33}, {0x29,0x2E,0x3D}, {0x2A,0x32,0x2A},
		{0x26,0x31,0x31}, {0x2C,0x30,0x34}, {0x2A,0x31,0x3F}, {0x2C,0x3A,0x31}, {0x2E,0x39,0x3A}, {0x33,0x24,0x24},
		{0x32,0x26,0x29}, {0x33,0x2C,0x23}, {0x32,0x2C,0x2C}, {0x3B,0x24,0x23}, {0x3B,0x24,0x29}, {0x3A,0x2D,0x22},
		{0x3A,0x2D,0x2A}, {0x31,0x2E,0x32}, {0x31,0x2F,0x38}, {0x3D,0x2B,0x33}, {0x35,0x32,0x24}, {0x34,0x32,0x2C},
		{0x33,0x3C,0x22}, {0x33,0x39,0x2C}, {0x3C,0x33,0x24}, {0x3B,0x34,0x2B}, {0x3E,0x3A,0x24}, {0x3E,0x3B,0x2C},
		{0x35,0x32,0x33}, {0x32,0x32,0x3A}, {0x35,0x39,0x33}, {0x36,0x3A,0x39}, {0x39,0x35,0x34}, {0x38,0x34,0x38},
		{0x3C,0x3A,0x34}, {0x3D,0x3D,0x3B}, {0x3F,0x3F,0x3F}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
		{0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F},
		{0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}, {0x3F,0x3F,0x3F}
	};

	uint16_t length;
	file.read((char*)&length, 2);

	if(length != 58*8)
		throw std::runtime_error("Creatures 1 backgrounds must contain exactly 464 images.");


	std::vector<spr_header> header(length);
	file.read((char*)&header[0], sizeof(header[0]) * header.size());

	for(auto const& h : header)
	{
		if(h.width != 144 || h.height != 150)
		{
			throw std::runtime_error("Each image in a Creatures 1 background must be 144x150.");
		}
	}

	version       = -1;
	_tiles         = glm::u8vec2(58, 8);
	tile_size     = glm::u16vec2(144, 150);
	pixels        = glm::u16vec2(_tiles) * tile_size;

	m_flags		  = shared_array<uint16_t>(size(), ~0u);

	m_textures  = shared_array<uint32_t>(noTextures(), 0);
	gl->glGenTextures(m_textures.size(), &m_textures[0]);

	for(auto i = 0u; i < m_textures.size(); ++i)
	{
		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[i]);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);

		gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, 144, 150, MAX_ARRAY_LAYERS, 0, GL_RGBA, GL_FLOAT, nullptr);
	}

	uint8_t raw_data[144*150];
	uint8_t rgb_data[144*150*3];

	for(int i = 0; i < size(); ++i)
	{
		int x = i / _tiles.y;
		int y = i % _tiles.y;

		int j = ((_tiles.y-1) - y) * _tiles.x + x;

//decode image
		file.read((char*)raw_data, sizeof(raw_data));

		for(int i = 0; i < 144*150; ++i)
		{
			memcpy(&rgb_data[i*3], &PALETTE_DTA[raw_data[i]][0], 3);
			rgb_data[i*3+0] *= 4;
			rgb_data[i*3+1] *= 4;
			rgb_data[i*3+2] *= 4;

		}

		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[j/MAX_ARRAY_LAYERS]);
		auto texture_layer = j % MAX_ARRAY_LAYERS;
		gl->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture_layer, 144, 150, 1, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
	}

	file.close();
}

void BackgroundImage::LoadS16(Shaders * shaders, std::ifstream file)
{
	uint32_t RGBformat;
	uint16_t length;

	file.read((char*)&RGBformat, 4);
	file.read((char*)&length, 2);

	version       = -1;
	_tiles         = glm::u8vec2(58, 16);
	tile_size     = glm::u16vec2(144, 150);
	pixels        = glm::u16vec2(_tiles) * tile_size;

	return ImportS16Frames(shaders, std::move(file), length, RGBformat);
}

void BackgroundImage::LoadBlk(Shaders * shaders, std::ifstream file)
{
	uint32_t RGBformat;
	uint16_t width, height, length;

	file.read((char*)&RGBformat, 4);
	file.read((char*)&width, 2);
	file.read((char*)&height, 2);
	file.read((char*)&length, 2);

	version       = -1;
	_tiles         = glm::u8vec2(width, height);
	tile_size     = glm::u16vec2(128, 128);
	pixels        = glm::u16vec2(_tiles) * tile_size;

	return ImportS16Frames(shaders, std::move(file), length, RGBformat);
}

void BackgroundImage::ImportS16Frames(Shaders * shaders, std::ifstream file, uint32_t no_tiles, uint32_t gl_type)
{
	auto gl = shaders->gl;

	if(no_tiles != (uint32_t)size())
		throw std::runtime_error("Number of images in background file is incorrect.");

	if(gl_type > 1)
		throw std::runtime_error("RGB format byte has incorrect value (must be 0 or 1).");

	std::vector<spr_header> header(no_tiles);

	file.read((char*)&header[0], sizeof(header[0]) * header.size());

	for(uint32_t i = 0; i < no_tiles; ++i)
	{
		if(header[i].width != tile_size.x || header[i].height != tile_size.y)
			throw std::runtime_error("some image in file has incorrect dimensions...");

		header[i].offset += (tile_size.x == 128) * 4;
	}

	gl_type = gl_type? GL_UNSIGNED_SHORT_5_6_5 :  GL_UNSIGNED_SHORT_5_5_5_1;
	uint8_t buffer[144*150*2];

	m_flags		  = shared_array<uint16_t>(size(), ~0u);

	m_textures  = shared_array<uint32_t>(noTextures(), 0);
	gl->glGenTextures(m_textures.size(), &m_textures[0]);

	for(auto i = 0u; i < m_textures.size(); ++i)
	{
		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[i]);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);

		gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, tile_size.x, tile_size.y, MAX_ARRAY_LAYERS, 0, GL_RGBA, GL_FLOAT, nullptr);
	}

	for(uint32_t i = 0; i < no_tiles; ++i)
	{
		int x = i / _tiles.y;
		int y = i % _tiles.y;

		int j = ((_tiles.y-1) - y) * _tiles.x + x;

		file.seekg(header[i].offset, std::ios_base::beg);
		file.read((char*)&buffer[0], tile_size.x*tile_size.y*2);

		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[j/MAX_ARRAY_LAYERS]);
		auto texture_layer = j % MAX_ARRAY_LAYERS;
		gl->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture_layer, tile_size.x, tile_size.y, 1, GL_RGB, gl_type, buffer);
	}

	file.close();
}

#include <QImage>
#include <QImageReader>

void BackgroundImage::LoadImage(Shaders * shaders, std::string const& filename)
{
	auto gl = shaders->gl;

	QImageReader reader(QString::fromStdString(filename));
	QImage image = reader.read();

	version       = -1;
	_tiles = (glm::ivec2(image.width(), image.height()) + 255) / 256;
	tile_size     = glm::u16vec2(256, 256);
	pixels        = glm::u16vec2(image.width(), image.height());

	m_textures  = shared_array<uint32_t>(noTextures(), 0);
	gl->glGenTextures(m_textures.size(), &m_textures[0]);

	for(auto i = 0u; i < m_textures.size(); ++i)
	{
		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[i]);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);

		gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 256, 256, MAX_ARRAY_LAYERS, 0, GL_RGBA, GL_FLOAT, nullptr);
	}

	std::vector<glm::u8vec4> RGB(256*256);

	for(int i = 0; i < size(); ++i)
	{
		int x = i / _tiles.y;
		int y = i % _tiles.y;

		int j = ((_tiles.y-1) - y) * _tiles.x + x;

		for(int px = 0; px < 65536; ++px)
		{
			QColor color{};
			auto _x = x * 256 + px % 256;
			auto _y =  y * 256 + 256 - px / 256;

			if(_x < image.width() && _y < image.height())
				color = image.pixelColor(_x, _y);

			RGB[px].r = color.red();
			RGB[px].g = color.green();
			RGB[px].b = color.blue();
			RGB[px].a = color.alpha();
		}

		gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_textures[j/MAX_ARRAY_LAYERS]);
		auto texture_layer = j % MAX_ARRAY_LAYERS;
		gl->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture_layer, tile_size.x, tile_size.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, &RGB[0]);
	}
}

