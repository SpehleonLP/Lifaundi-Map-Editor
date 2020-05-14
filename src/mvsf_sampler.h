#ifndef MVSF_SAMPLER_H
#define MVSF_SAMPLER_H
#include <vector>
#include <memory>
#include <glm/gtc/type_precision.hpp>

class Metaroom;

class MVSF_sampler
{
public:
	static std::vector<glm::i16vec4> GetBoundingBoxes(Metaroom * metaroom);
	static std::vector<int>          GetSamplingRooms(Metaroom * metaroom);

	MVSF_sampler(Metaroom * metaroom, int stride);
	~MVSF_sampler() = default;

	uint32_t size() const { return m_size.x * m_size.y; }

	void AssignInternalRoomIds();
	void PerformSampling();
	void SampleRoom(int);

	static uint32_t GetDistanceSquaredToRoom(glm::ivec2 point, const glm::ivec2 * verts);

	std::vector<glm::i16vec4> GetBoundingBoxes() const;
	glm::i16vec4 GetBoundingBox(int) const;

private:
	struct Pixel
	{
		int      room_id;
		uint32_t length2;
	};

	Metaroom *               m_metaroom{};
	glm::i16vec4             m_bounds{};
	const short              m_stride{};
	glm::ivec2               m_size{};
	std::unique_ptr<Pixel[]> m_pixels{};

};

#endif // MVSF_SAMPLER_H
