#ifndef METAROOM_MEMORY_H
#define METAROOM_MEMORY_H
#include "Spehleon/lib/Support/shared_array.hpp"
#include "src/entitysystem.h"
#include "src/metaroomselection.h"
#include <glm/gtc/type_precision.hpp>
#include <map>

struct Room;

class MetaroomMemory
{
public:
	struct xxEdgeRange;

	EntitySystem::Range range() const { return _entitySystem; }
	xxEdgeRange edgeRange() const;

	__always_inline bool empty() const { return noFaces() == 0; }
	__always_inline size_t noFaces() const { return _entitySystem.used(); };
	__always_inline size_t noEdges() const { return noFaces()*4; }
	__always_inline size_t noVertices() const { return noFaces() * 4; }

	__always_inline static int NextInEdge(int id) { return (id & 0xFFFFFFFC) + ((id+1) & 0x03); }
	__always_inline static int GetOppositeEdge(int id) { return id ^ 0x02; }
	__always_inline static uint32_t GetDoorKey(uint32_t a, uint32_t b) { return a < b? ((uint64_t)a << 32) | b : ((uint64_t)b << 32) | a; }

	inline glm::ivec2 GetNextVertex(int i) const { return raw()[(i & 0xFFFFFFFC) + ((i+1)&0x03)]; }
	inline glm::ivec2 GetVertex(int i) const { return raw()[i]; }
	inline glm::ivec2 GetEdge(int i) const { return raw()[(i & 0xFFFFFFFC) + ((i+1)&0x03)] - raw()[i]; }

	inline glm::ivec2 GetVertex(int i, int j) const { return raw()[i*4 + j%4]; }
	inline glm::ivec2 GetEdge(int i, int j) const { return GetVertex(i, j+1) - GetVertex(i, j); }

	__always_inline glm::ivec2 * raw() { return &_verts[0][0]; };
	__always_inline glm::ivec2 const* raw() const { return &_verts[0][0]; };

	void SetRoom(int index, Room const&);
	void CopyRoom(int dst, int src);

	glm::vec2 GetGravity(int room) const;
	glm::vec2 GetCenter(int room) const;
	int GetPermeability(int a, int b) const;

	glm::i16vec4 GetBoundingBox() const;
	glm::ivec2 GetPointOnEdge(int v0, float p) const;
	float      GetPercentOfEdge(int v0, glm::ivec2) const;
	float      ProjectOntoEdge(int v0, glm::ivec2) const;
	bool	   DoShareEdge(uint32_t, uint32_t) const;

	void GetFaceAABB(int id, glm::i16vec2 & tl, glm::i16vec2 & br) const;
	bool IsPointContained(int face, glm::ivec2 point);

	std::vector<std::pair<uint64_t, float>> GetInvalidPermeabilities();
	std::vector<std::pair<uint64_t, float>> GetPermeabilities(std::vector<uint32_t> const& indices, bool remove = false);
	void RemovePermeabilities(std::vector<std::pair<uint64_t, float>> const&);
	void AddPermeabilities(std::vector<std::pair<uint64_t, float>>  const&);
	void DumpPermeabilityTable() { _permeabilities.clear(); }

	void AddPermeabilities(MetaroomMemory & src);

	glm::vec2 GetGravity() const;
	glm::vec3 GetShade() const;
	glm::vec4 GetAudio() const;

	int GetMusicTrack() const;
	int GetRoomType() const;

public:
	std::map<uint64_t, float> _permeabilities;

	shared_array<std::array<glm::ivec2, 4>> _verts;
	shared_array<std::array<glm::ivec2, 4>> _scratch;
	shared_array<std::array<glm::ivec2, 4>> _prev;

	auto & verts(int i) const { return _verts[i>>2][i&0x03]; }
	auto & scratch(int i) const { return _scratch[i>>2][i&0x03]; }
	auto & prev(int i) const { return _prev[i>>2][i&0x03]; }

	auto & verts(int i) { return _verts[i>>2][i&0x03]; }
	auto & scratch(int i) { return _scratch[i>>2][i&0x03]; }
	auto & prev(int i) { return _prev[i>>2][i&0x03]; }

//room properties
	shared_array<uint32_t>  _gravity;  // 2 half floats
	shared_array<int8_t>    _music;
	shared_array<uint8_t>   _roomType;
	shared_array<int>	    _color;

	shared_array<uint32_t>    _directionalShade;  // 2 half floats
	shared_array<uint8_t >    _ambientShade;
	shared_array<glm::u8vec4> _audio;  // 2 half floats

// needed for realloc
	MetaroomSelection			_selection;

	EntitySystem _entitySystem{std::bind(&MetaroomMemory::Realloc, this, std::placeholders::_1)};
	void Realloc(size_t);
};

struct MetaroomMemory::xxEdgeRange
{
	struct iterator
	{
		iterator(std::vector<uint32_t>::const_iterator const& p) : p(p), e(0) {}

		bool operator==(iterator const& it) { return p == it.p && e == it.e; }
		bool operator!=(iterator const& it) { return p != it.p || e != it.e; }

		iterator & operator++() { if((e = (e+1) % 4) == 0) ++p; return *this; }
		iterator & operator--() { if((e = (e+3) % 4) == 3) --p; return *this; }

		uint32_t  operator*() const { return (*p)*4 + e; }

		std::vector<uint32_t>::const_iterator p;
		uint8_t e;
	};

	xxEdgeRange(EntitySystem::Range const& x) : _begin(x._begin), _end(x._end) {}
	iterator begin() const { return _begin; }
	iterator end() const { return _end; }

	std::vector<uint32_t>::const_iterator _begin, _end;
};

__always_inline
MetaroomMemory::xxEdgeRange MetaroomMemory::edgeRange() const { return EntitySystem::Range(_entitySystem); }

#endif // METAROOM_MEMORY_H
