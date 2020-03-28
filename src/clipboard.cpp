#include "clipboard.h"
#include "metaroom.h"

static std::vector<Room> g_ClipBoard;

std::vector<Room>   Clipboard::Extract(Metaroom* metaroom)
{
	return Extract(metaroom, metaroom->m_selection.GetFaceSelection());
}

std::vector<Room>   Clipboard::Extract(Metaroom* metaroom, std::vector<int> const& selection)
{
	std::vector<Room> r;
	r.reserve(selection.size());

	for(int i : selection)
	{
		Room room;

		room.type        = metaroom->m_roomType[i];
		room.music_track = metaroom->m_music[i];
		room.gravity    = metaroom->m_gravity[i];

		for(int j = 0; j < 4; ++j)
		{
			room.wall_types[j] = metaroom->m_doorType[i*4+j];
			room.verts[j]      = metaroom->m_verts[i*4+j];
		}

		r.push_back(room);
	}

	return r;

}

std::vector<Room> & Clipboard::GetClipBoard()
{
	return g_ClipBoard;
}

void                Clipboard::SetClipBoard(std::vector<Room> && it)
{
	g_ClipBoard = std::move(it);
}

void                Clipboard::Center(std::vector<Room> &room)
{
	if(room.empty())
		return;

	auto min = room[0].verts[0];
	auto max = room[0].verts[0];

	for(auto const& r : room)
	{
		for(int j = 0; j < 4; ++j)
		{
			min = glm::min(min, r.verts[j]);
			max = glm::max(max, r.verts[j]);
		}
	}

	auto center = (min + max) / 2;

	for(auto & r : room)
		for(int j = 0; j < 4; ++j)
			r.verts[j] -= center;
}

void                Clipboard::Translate(std::vector<Room> &room, glm::vec2 center)
{

	for(auto & r : room)
		for(int j = 0; j < 4; ++j)
			r.verts[j] += center;
}
