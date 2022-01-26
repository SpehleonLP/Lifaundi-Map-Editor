#include "backgroundimage.h"
#include "Shaders/blitshader.h"
#include "Shaders/transparencyshader.h"
#include "enums.hpp"
#include <QProgressDialog>
#include "src/glviewwidget.h"
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>
#include <cassert>
#include <zlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

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

	return Type::Lifaundi;
}


BackgroundImage::BackgroundImage(GLViewWidget * gl, std::string const& filename) :
	m_filename(filename)
{
	std::ifstream file(m_filename, std::ios::binary);

	if(!file.is_open())
		throw std::system_error(errno, std::system_category(), m_filename);

    file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

	switch((m_type = GetType(filename)))
	{
	case Type::Creatures1:
		LoadSpr(gl, std::move(file));
		break;
	case Type::Creatures2:
		LoadS16(gl, std::move(file));
		break;
	case Type::Creatures3:
		LoadBlk(gl, std::move(file));
		break;
	case Type::Lifaundi:
		LoadLifaundi(gl, std::move(file));
		break;
	}
}

BackgroundImage::~BackgroundImage()
{
}

void BackgroundImage::Release(GLViewWidget * gl)
{
    gl->glDeleteTextures(size(), &m_textures[0]); gl->glAssert();
    gl->glDeleteVertexArrays(1, &m_vao); gl->glAssert();
    gl->glDeleteBuffers(1, &m_vbo); gl->glAssert();
}

void BackgroundImage::Render(GLViewWidget* gl)
{
    CreateVBO(gl);

//	m_transparencyShader.bind(gl);
//	gl->glDrawArrays(GL_TRIANGLES, size()*6, 6); gl->glAssert();

    BlitShader::Shader.bind(gl); gl->glAssert();

    gl->glBindVertexArray(m_vao); gl->glAssert();
    gl->glActiveTexture(GL_TEXTURE0);

	for(int i = 0; i < size(); ++i)
	{
		if(m_dimensions[i].x0 == m_dimensions[i].x1)
			continue;

        gl->glBindTexture(GL_TEXTURE_2D, m_textures[i]); gl->glAssert();
        gl->glDrawArrays(GL_TRIANGLES, i*6, 6); gl->glAssert();
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

    gl->glAssert();;

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

        gl->glAssert();;
	}

	CreateVBO();
    gl->glAssert();;

	return true;
}
#endif

void BackgroundImage::CreateVBO(GLViewWidget* gl)
{
	if(m_vao != 0)
		return;

	struct vertex
	{
		glm::ivec3   position;
		glm::u16vec2 texCoord;
	};

	std::unique_ptr<vertex[]> buffer(new vertex[size()*6 + 6]);

	for(uint8_t y = 0; y < tiles.y; ++y)
	{
		for(uint8_t x = 0; x < tiles.x; ++x)
		{
			auto i          = (int) y * tiles.x + x;
			auto info      = m_dimensions[i];
			vertex * corner = &buffer[i * 6];

			if(tile_size.x == 256)
			{
				info.x0 *= 8;
				info.y0 *= 8;
				info.y1 *= 8;
				info.x1 *= 8;

				corner[0].texCoord = glm::u16vec2(0, 0);
				corner[1].texCoord = glm::u16vec2(1, 0);
				corner[2].texCoord = glm::u16vec2(0, 1);
				corner[5].texCoord = glm::u16vec2(1, 1);
			}
			else
			{
				info.x0 = info.y0 = 0;
				info.x1 = tile_size.x/2;
				info.y1 = tile_size.y/2;

				corner[0].texCoord = glm::u16vec2(0, 1);
				corner[1].texCoord = glm::u16vec2(1, 1);
				corner[2].texCoord = glm::u16vec2(0, 0);
				corner[5].texCoord = glm::u16vec2(1, 0);
			}

			glm::ivec3 offset(x * tile_size.x - pixels.x/2, y * tile_size.y - pixels.y/2, 0);

			corner[0].position = offset + glm::ivec3(info.x0*2, info.y0*2, 0);
			corner[1].position = offset + glm::ivec3(info.x1*2, info.y0*2, 0);
			corner[2].position = offset + glm::ivec3(info.x0*2, info.y1*2, 0);
			corner[5].position = offset + glm::ivec3(info.x1*2, info.y1*2, 0);

			corner[3] = corner[2];
			corner[4] = corner[1];
		}
	}

	{
		vertex * corner = &buffer[size() * 6];
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
	}

    gl->glGenVertexArrays(1, &m_vao);
    gl->glGenBuffers(1, &m_vbo);

    gl->glBindVertexArray(m_vao);
    gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    gl->glBufferData(GL_ARRAY_BUFFER, (size()+1) * 6 * sizeof(vertex), &buffer[0], GL_STATIC_DRAW);

	//position
    gl->glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, sizeof(vertex), (void*) offsetof(vertex, position));

//	glVertexAttribPointer(2, 4, GL_BYTE, GL_FALSE, sizeof(gltfVertex), (void*) 8);
	//texcoord0
    gl->glVertexAttribPointer(3, 2, GL_SHORT, GL_FALSE, sizeof(vertex), (void*) offsetof(vertex, texCoord));

    gl->glEnableVertexAttribArray(0);
    gl->glEnableVertexAttribArray(3);

	gl->glAssert();
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

static uint8_t * interleave_DXT5(uint8_t * dst, uint8_t * src, int blocks)
{
	src = interleave_BC4 <16>(dst  , src, blocks);
	src = interleave_DXT1<16>(dst+8, src, blocks);

	return src;
}

static uint8_t * _interleave_DXT1(uint8_t * dst, uint8_t * src, int blocks)
{
	return interleave_DXT1<8>(dst, src, blocks);
}

static uint8_t * interleave_BC5(uint8_t * dst, uint8_t * src, int blocks)
{
	src = interleave_BC4<16>(dst  , src, blocks);
	src = interleave_BC4<16>(dst+8, src, blocks);

	return src;
}


void BackgroundImage::LoadLifaundi(GLViewWidget * gl, std::ifstream file)
{
#define BUFFER_SIZE 350000

	z_stream zlib;
	memset(&zlib, 0, sizeof(zlib));
	inflateInit(&zlib);

	char title[4];

	file.read(&title[0], sizeof(title));
	file.read((char*)&version, sizeof(version));
	file.read((char*)&tiles, sizeof(tiles));

	if(version == 1)
		pixels = glm::u16vec2(tiles) * (uint16_t) 256;
	else
	{
		file.read((char*)&pixels, sizeof(pixels));
	}


	if(memcmp(title, "lbck", 4))
		throw std::runtime_error("Bad File");

	m_dimensions  = std::unique_ptr<TileInfo[]> (new TileInfo[size()]);
	m_textures    = std::unique_ptr<uint32_t[]> (new uint32_t[size()]);
	auto offsets  = std::unique_ptr<uint32_t[]>(new uint32_t[size()*3+1]);

	memset(&m_textures[0], 0, sizeof(uint32_t) * size());
	file.read((char*)&m_dimensions[0], size() * sizeof(m_dimensions[0]));
	file.read((char*)&offsets[0], (size()*3 + 1) * sizeof(uint32_t));

    gl->glGenTextures(size(), &m_textures[0]);

	std::unique_ptr<uint8_t[]> buffer(new uint8_t[TILE_BYTES]);
	std::unique_ptr<uint8_t[]> buffer2(new uint8_t[TILE_BYTES]);

	for(int i = 0; i < size(); ++i)
	{
		if(m_dimensions[i].x0 == m_dimensions[i].x1)
			continue;

        gl->glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

		uint32_t * mip = &offsets[i*3];
		auto dscr = m_dimensions[i];

		int x_offset = dscr.x0 * 16;
		int y_offset = dscr.y0 * 16;
		int width    = (dscr.x1 - dscr.x0) * 16;
		int height   = (dscr.y1 - dscr.y0) * 16;
		auto area    = width * height;

// deinterleave data
		uint32_t blocks_x = (dscr.x1 - dscr.x0) * 4;
		uint32_t blocks_y = (dscr.y1 - dscr.y0) * 4;

		for(int j = 1; j <= 2; ++j)
		{
			blocks_x >>= 1;
			blocks_y >>= 1;
			auto bytes  = mip[j+1] - mip[j];

			if(bytes == 0)
				continue;

			assert(bytes <= TILE_BYTES);

			file.seekg(mip[j], std::ios_base::beg);
			file.read((char*) &buffer[0], bytes);

			if(version == 3)
			{
				if(i == 0 && j == 1)
				{
					std::cerr << "offset " << std::hex << std::setfill('0') << mip[j] << std::endl;
					std::cerr << ToHex(&buffer[0], 32) << std::endl;
				}


				zlib.next_in  = &buffer[0];
				zlib.avail_in = bytes;
				zlib.total_in = 0;

				zlib.next_out  = &buffer2[0];
				zlib.avail_out = TILE_BYTES;
				zlib.total_out = 0;

				int code = inflate(&zlib, Z_FINISH);

				if(zlib.msg != nullptr)
					throw std::runtime_error(zlib.msg);

				if(zlib.total_in != bytes)
					throw std::runtime_error("failed to decompress all data");

				std::swap(buffer, buffer2);
				bytes = zlib.total_out;

				inflateReset(&zlib);
			}

			const int d0 = 1 << j;
			const int d1 = d0*d0;

			auto format = (bytes / (blocks_x * blocks_y));
			format = format < 64? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

			if(format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
				_interleave_DXT1(&buffer2[0], &buffer[0], blocks_x * blocks_y);
			else
				interleave_DXT5(&buffer2[0], &buffer[0], blocks_x * blocks_y);

            gl->glCompressedTexImage2D(GL_TEXTURE_2D,
				j-1,
				format,
				width    / d0,
				height   / d0,
				0,
				area     / d1 / (1 + (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)),
				&buffer2[0]);

            gl->glAssert();
		}
	}


	inflateEnd(&zlib);
	file.close();
}

std::vector<uint16_t> GetC1Pallette();

struct spr_header
{
	uint32_t offset;
	uint16_t width, height;
};

void BackgroundImage::LoadSpr(GLViewWidget * gl, std::ifstream file)
{
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
	tiles         = glm::u8vec2(58, 8);
	tile_size     = glm::u16vec2(144, 150);
	pixels        = glm::u16vec2(tiles) * tile_size;

	m_dimensions  = std::unique_ptr<TileInfo[]> (new TileInfo[size()]);
	m_textures    = std::unique_ptr<uint32_t[]> (new uint32_t[size()]);

	memset(&m_textures[0], 0, sizeof(uint32_t) * size());

	gl->glGenTextures(size(), &m_textures[0]);

	uint8_t raw_data[144*150];
	uint8_t rgb_data[144*150*3];

	for(int i = 0; i < size(); ++i)
	{
		int x = i / tiles.y;
		int y = i % tiles.y;

		int j = ((tiles.y-1) - y) * tiles.x + x;

//decode image
		file.read((char*)raw_data, sizeof(raw_data));

		for(int i = 0; i < 144*150; ++i)
		{
			memcpy(&rgb_data[i*3], &PALETTE_DTA[raw_data[i]][0], 3);
			rgb_data[i*3+0] *= 4;
			rgb_data[i*3+1] *= 4;
			rgb_data[i*3+2] *= 4;

		}


		gl->glBindTexture(GL_TEXTURE_2D, m_textures[j]);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 144, 150, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
	}

	file.close();
}

void BackgroundImage::LoadS16(GLViewWidget * gl, std::ifstream file)
{
	uint32_t RGBformat;
	uint16_t length;

	file.read((char*)&RGBformat, 4);
	file.read((char*)&length, 2);

	version       = -1;
	tiles         = glm::u8vec2(58, 16);
	tile_size     = glm::u16vec2(144, 150);
	pixels        = glm::u16vec2(tiles) * tile_size;

	return ImportS16Frames(gl, std::move(file), length, RGBformat);
}

void BackgroundImage::LoadBlk(GLViewWidget * gl, std::ifstream file)
{
	uint32_t RGBformat;
	uint16_t width, height, length;

	file.read((char*)&RGBformat, 4);
	file.read((char*)&width, 2);
	file.read((char*)&height, 2);
	file.read((char*)&length, 2);

	version       = -1;
	tiles         = glm::u8vec2(width, height);
	tile_size     = glm::u16vec2(128, 128);
	pixels        = glm::u16vec2(tiles) * tile_size;

	return ImportS16Frames(gl, std::move(file), length, RGBformat);
}

void BackgroundImage::ImportS16Frames(GLViewWidget * gl, std::ifstream file, uint32_t no_tiles, uint32_t gl_type)
{
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

	m_dimensions  = std::unique_ptr<TileInfo[]> (new TileInfo[size()]);
	m_textures    = std::unique_ptr<uint32_t[]> (new uint32_t[size()]);

	gl->glGenTextures(size(), &m_textures[0]);

	for(uint32_t i = 0; i < no_tiles; ++i)
	{
		int x = i / tiles.y;
		int y = i % tiles.y;

		int j = ((tiles.y-1) - y) * tiles.x + x;

		file.seekg(header[i].offset, std::ios_base::beg);
		file.read((char*)&buffer[0], tile_size.x*tile_size.y*2);

		gl->glBindTexture(GL_TEXTURE_2D, m_textures[j]);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tile_size.x, tile_size.y, 0, GL_RGB, gl_type, buffer);
	}

	file.close();
}

