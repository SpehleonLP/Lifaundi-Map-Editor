#ifndef CLIPBOARD_H
#define CLIPBOARD_H
#include <array>
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>
#include <vector>

struct Room
{
	uint8_t    type;
	 int16_t   music_track;
	//uint8_t    wall_types[4];
	uint32_t   gravity;
	std::array<glm::ivec2, 4> verts;

	uint32_t	directionalShade;
	float		ambientShade;
	glm::u8vec4 audio;
	glm::u16vec2 depth;
};

class Metaroom;

namespace Clipboard
{
	std::vector<Room>   Extract(Metaroom*);
	std::vector<Room>   Extract(Metaroom*, const std::vector<uint32_t> &);
	std::vector<Room> & GetClipBoard();
	void                SetClipBoard(std::vector<Room> &&);
	void                Center(std::vector<Room> &room);
	void                Translate(std::vector<Room> &room, glm::vec2 center);
};

#endif // CLIPBOARD_H
