#include "metaroom_memory.h"
#include "src/clipboard.h"
#include "src/lf_math.h"
#include <glm/packing.hpp>
#include <glm/glm.hpp>
#include <loguru.hpp>



template<typename T>
shared_array<T> realloc(shared_array<T>  & it, size_t size)
{
	shared_array<T> x(size);
	memcpy(x.data(), it.data(), it.byteLength());
	return x;
}

void MetaroomMemory::Realloc(size_t size)
{
	_verts				= realloc(_verts, size);
	_gravity			= realloc(_gravity, size);
	_music				= realloc(_music, size);
	_roomType			= realloc(_roomType, size);
	_color				= realloc(_color, size);
	_directionalShade	= realloc(_directionalShade, size);
	_ambientShade		= realloc(_ambientShade, size);
	_audio				= realloc(_audio, size);

	_selection.resize(size);
}

void MetaroomMemory::SetRoom(int index, Room const& room)
{
	_roomType[index]		= room.type;
	_gravity[index]		= room.gravity;
	_music[index]			= room.music_track;

	_directionalShade[index]	= room.directionalShade;
	_ambientShade[index]		= room.ambientShade;
	_audio[index]				= room.audio;

	for(int j = 0; j < 4; ++j)
		_verts[index][j] = room.verts[j];
}


void  MetaroomMemory::CopyRoom(int dst, int src)
{
	if(dst == src) return;

	_roomType[dst]		= _roomType[src];
	_gravity[dst]		= _gravity[src];
	_music[dst]		= _music[src];

	_directionalShade[dst] = _directionalShade[src];
	_ambientShade[dst] = _ambientShade[src];
	_audio[dst] = _audio[src];

	for(int j = 0; j < 4; ++j)
		_verts[dst*4 + j] = _verts[src*4 + j];
}


glm::vec2 MetaroomMemory::GetGravity(int room) const
{
	if((uint32_t)room >= _entitySystem.used())
		throw std::runtime_error("tried to get gravity of room which doesn't exist");

	return glm::unpackHalf2x16(_gravity[room]);
}

glm::vec2 MetaroomMemory::GetCenter(int room) const
{
	if((uint32_t)room >= _entitySystem.used())
		throw std::runtime_error("tried to get center of room which doesn't exist");

	return glm::vec2(
			_verts[room][0] +
			_verts[room][1] +
			_verts[room][2] +
			_verts[room][3]) / 4.f;
}


int MetaroomMemory::GetPermeability(int a, int b) const
{
	if(a < 0 || b < 0)
		return -1;

	uint64_t key = GetDoorKey(a, b);

	auto itr = _permeabilities.find(key);

	if(itr == _permeabilities.end())
		return 100;

	return itr->second;
}

glm::ivec2 MetaroomMemory::GetPointOnEdge(int v0, float p) const
{
	glm::vec2 vec(raw()[NextInEdge(v0)] - raw()[v0]);

	 glm::ivec2 offset = glm::vec2(
		vec.x < 0? ceil(vec.x * p) : floor(vec.x * p),
		vec.y < 0? ceil(vec.y * p) : floor(vec.y * p));

	if((v0 & 0x03) < 2)
		return raw()[NextInEdge(v0)] - offset;

	return offset + raw()[v0];
}


float     MetaroomMemory::GetPercentOfEdge(int v0, glm::ivec2 pos) const
{
	glm::vec2 a(raw()[NextInEdge(v0)] - raw()[v0]);
	glm::vec2 b(pos - raw()[v0]);

	float value = std::max(0.f, std::min(1.f, glm::length(b) / glm::length(a)));

	if((v0 & 0x03) < 2)
		return 1.f - value;

	return value;
}

float     MetaroomMemory::ProjectOntoEdge(int v0, glm::ivec2 pos) const
{
	glm::vec2 a(pos - raw()[v0]);
	glm::vec2 b(raw()[NextInEdge(v0)] - raw()[v0]);
	float length = math::length2(b);

	float value = glm::dot(a, b) / length;
	value = std::max(0.f, std::min(1.f, value));

	if((v0 & 0x03) < 2)
		return 1.f - value;

	return value;
}

void MetaroomMemory::GetFaceAABB(int id, glm::i16vec2 & tl, glm::i16vec2 & br) const
{
	tl.x = std::min(
		std::min(_verts[id][0].x, _verts[id][1].x),
		std::min(_verts[id][2].x, _verts[id][3].x));

	tl.y = std::min(
		std::min(_verts[id][0].y, _verts[id][1].y),
		std::min(_verts[id][2].y, _verts[id][3].y));

	br.x = std::max(
		std::max(_verts[id][0].x, _verts[id][1].x),
		std::max(_verts[id][2].x, _verts[id][3].x));

	br.y = std::max(
		std::max(_verts[id][0].y, _verts[id][1].y),
		std::max(_verts[id][2].y, _verts[id][3].y));
}

bool MetaroomMemory::IsPointContained(int i, glm::ivec2 pos)
{
	return math::IsPointContained(&_verts[i][0], pos);
}

glm::i16vec4 MetaroomMemory::GetBoundingBox() const
{
	if(_entitySystem.used() == 0)
		return glm::i16vec4(0, 0, 0, 0);

	glm::ivec2 min{_verts[0][0]};
	glm::ivec2 max{_verts[0][0]};

	for(auto i : edgeRange())
	{
		min = glm::min(min, raw()[i]);
		max = glm::max(max, raw()[i]);
	}

	return glm::i16vec4(min, max);
}

std::vector<std::pair<uint64_t, float>> MetaroomMemory::GetPermeabilities(std::vector<uint32_t> const& indices, bool remove)
{
	std::vector<std::pair<uint64_t, float>> r;

	for(auto itr = _permeabilities.begin(), next = itr; itr != _permeabilities.end(); itr = next)
	{
		++next;

		uint32_t first = (itr->first) & 0xFFFFFFFF;
		uint32_t second = (itr->first >> 32) & 0xFFFFFFFF;

		if(std::find(indices.begin(), indices.end(), first) != indices.end()
		|| std::find(indices.begin(), indices.end(), second) != indices.end())
		{
			r.push_back({itr->first, itr->second});

			if(remove)
				_permeabilities.erase(itr);
		}
	}

	return r;
}

std::vector<std::pair<uint64_t, float>> MetaroomMemory::GetInvalidPermeabilities()
{
	std::vector<std::pair<uint64_t, float>> r;

	for(auto itr = _permeabilities.begin(), next = itr; itr != _permeabilities.end(); itr = next)
	{
		++next;

		uint32_t first = (itr->first) & 0xFFFFFFFF;
		uint32_t second = (itr->first >> 32) & 0xFFFFFFFF;

		if(DoShareEdge(first, second) == false)
		{
			r.push_back({itr->first, itr->second});
			_permeabilities.erase(itr);
		}
	}

	return r;
}

void MetaroomMemory::RemovePermeabilities(std::vector<std::pair<uint64_t, float>> const& vec)
{
	for(auto item : vec)
	{
		auto itr = _permeabilities.find(item.first);

		if(itr != _permeabilities.end())
			_permeabilities.erase(itr);
	}

}
void MetaroomMemory::AddPermeabilities(std::vector<std::pair<uint64_t, float>>  const& vec)
{
	for(auto item : vec)
	{
		if(item.second == 100.f)
		{
			auto itr = _permeabilities.find(item.first);

			if(itr != _permeabilities.end())
				_permeabilities.erase(itr);
		}
		else
		{
			_permeabilities[item.first] = item.second;
		}
	}

}

void MetaroomMemory::AddPermeabilities(MetaroomMemory & src)
{
	auto inverse = src._entitySystem.GetInversse();

	for(auto const& item : src._permeabilities)
	{
		auto a = item.first & 0xFFFFFFFF;
		auto b = (item.first >> 32) & 0xFFFFFFFF;
		auto key = GetDoorKey(inverse[a], inverse[b]);
		_permeabilities[key] = item.second;
	}
}


bool	   MetaroomMemory::DoShareEdge(uint32_t i, uint32_t j) const
{
	std::pair<float, int> begin, end;

	for(int k = 0; k < 16; ++k)
	{
	//	if(types[0][k%4] != 0 || types[1][k/4] != 0)continue;

		const auto v0 = _verts[i][k%4];
		const auto v1 = _verts[i][(k+1)%4];

		const auto a0 = _verts[j][k/4];
		const auto a1 = _verts[j][((k/4)+1)%4];

		if(math::IsOpposite(a1-a0, v1-v0)
		&& math::IsColinear(v0, v1, a0, a1)
		&& math::GetOverlap(v0, v1, a0, a1, &begin, &end))
			return true;
	}

	return false;
}

glm::vec2 MetaroomMemory::GetGravity() const
{
	float length{};
	float angle{};
	int   count{};

	for(auto i : range())
	{
		if(!_selection.IsFaceSelected(i))
			continue;

		glm::vec2 vec = glm::unpackHalf2x16(_gravity[i]);

		float len = glm::length(vec);
		length += len;

		if(len)
		{
			vec /= len;
			angle += std::atan2(vec.y, vec.x);
		}

		++count;
	}

	return glm::vec2(length, angle) / (count? count : 1.f);
}

glm::vec3 MetaroomMemory::GetShade() const
{
	float length{};
	float angle{};
	float ambient{};
	int   count{};

	for(auto i : range())
	{
		if(!_selection.IsFaceSelected(i))
			continue;

		glm::vec2 vec = glm::unpackHalf2x16(_directionalShade[i]);
		ambient += _ambientShade[i];

		float len = glm::length(vec);
		length += len;

		if(len)
		{
			vec /= len;
			angle += std::atan2(vec.y, vec.x);
		}

		++count;
	}

	return glm::vec3(length, angle, ambient) / (count? count : 1.f);
}

glm::vec4 MetaroomMemory::GetAudio() const
{
	glm::vec4 audio{0};
	int   count{};

	for(auto i : range())
	{
		if(!_selection.IsFaceSelected(i))
			continue;

		audio += _audio[i];
		++count;
	}

	if(count)
	{
		audio /= count;
	}

	return audio;
}




int MetaroomMemory::GetMusicTrack() const
{
	bool     match = false;
	int      track = 0;

	for(auto i : range())
	{
		if(_selection.IsFaceSelected(i))
		{
			if(match == true && track != _music[i])
				return false;

			track = _music[i];
			match = true;
		}
	}

	return track;
}

int MetaroomMemory::GetRoomType() const
{
	bool match = false;
	int   r_type = -1;

	for(auto i : range())
	{
		if(_selection.IsFaceSelected(i))
		{
			if(match == true && r_type != _roomType[i])
				return -1;

			r_type = _roomType[i];
			match = true;
		}
	}

	return r_type;
}
