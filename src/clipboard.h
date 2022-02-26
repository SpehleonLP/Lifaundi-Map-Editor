#ifndef CLIPBOARD_H
#define CLIPBOARD_H
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>
#include <vector>

struct Room
{
	uint8_t    type;
	 int16_t   music_track;
	//uint8_t    wall_types[4];
	uint32_t   gravity;
	glm::ivec2 verts[4];
	int        uuid{-1};

	uint32_t	directionalShade;
	float		ambientShade;
	glm::u8vec4 audio;
};

class Metaroom;

namespace Clipboard
{
	std::vector<Room>   Extract(Metaroom*);
	std::vector<Room>   Extract(Metaroom*, std::vector<int> const&);
	std::vector<Room> & GetClipBoard();
	void                SetClipBoard(std::vector<Room> &&);
	void                Center(std::vector<Room> &room);
	void                Translate(std::vector<Room> &room, glm::vec2 center);
};

#endif // CLIPBOARD_H
