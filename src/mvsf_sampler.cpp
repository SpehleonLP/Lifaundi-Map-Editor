#include "mvsf_sampler.h"
#include "metaroom.h"
#include "lf_math.h"
#include <QProgressDialog>
#include <QCoreApplication>
#include <climits>

std::vector<glm::i16vec4> MVSF_sampler::GetBoundingBoxes(Metaroom * metaroom)
{
	MVSF_sampler sampler(metaroom, 4);

	sampler.AssignInternalRoomIds();
	sampler.PerformSampling();

	return sampler.GetBoundingBoxes();
}

MVSF_sampler::MVSF_sampler(Metaroom * metaroom, int stride) :
m_metaroom(metaroom),
m_bounds(metaroom->GetBoundingBox()),
m_stride(stride <= 0? 1 : stride),
m_size(((m_bounds.z - m_bounds.x) + (stride-1))/stride,
	   ((m_bounds.w - m_bounds.y) + (stride-1))/stride),
m_pixels(new Pixel[size()])
{
	for(uint32_t i = 0; i < size(); ++i)
	{
		m_pixels[i].room_id = -1;
		m_pixels[i].length2 = UINT_MAX;
	}
}

std::vector<int> MVSF_sampler::GetSamplingRooms(Metaroom * metaroom)
{
	std::vector<int> r;
	r.reserve(metaroom->size()/4);

	for(uint32_t i = 0; i < metaroom->size(); ++i)
	{
		for(uint32_t j = 0; j < 4; ++j)
		{
			if(!metaroom->m_tree.HasFullOverlap(i*4 + j))
			{
				r.push_back(i);
				break;
			}
		}
	}

	return r;
}


void MVSF_sampler::AssignInternalRoomIds()
{
	glm::i16vec2 min, max;
	glm::i16vec2 offset(m_bounds.x, m_bounds.y);

	for(uint32_t i = 0; i < m_metaroom->size(); ++i)
	{
		m_metaroom->GetFaceAABB(i, min, max);

		min =  (min - offset)                        / m_stride;
		max = ((max - offset) + (short)(m_stride-1)) / m_stride;

		for(int y = min.y; y < max.y; ++y)
		{
			int _y = y * m_stride + m_bounds.y;

			for(int x = min.x; x < max.x; ++x)
			{
				int _x = x * m_stride + m_bounds.x;

				if(m_metaroom->IsPointContained(i, glm::ivec2(_x, _y)))
				{
					m_pixels[y * m_size.x + x].room_id = i;
					m_pixels[y * m_size.x + x].length2 = 0;
				}
			}
		}
	}
}

std::vector<glm::i16vec4> MVSF_sampler::GetBoundingBoxes() const
{
	std::vector<glm::i16vec4> r(m_metaroom->size());

	for(uint32_t i = 0; i < r.size(); ++i)
		r[i] = GetBoundingBox(i);

	return r;
}

uint32_t MVSF_sampler::GetDistanceSquaredToRoom(glm::ivec2 point, glm::ivec2 const* verts)
{
	if(math::IsPointContained(verts, point))
		return 0;

	return std::min(std::min(
		math::SemgentDistanceSquared(verts[0], verts[1], point),
		math::SemgentDistanceSquared(verts[1], verts[2], point)),
	std::min(
		math::SemgentDistanceSquared(verts[2], verts[3], point),
		math::SemgentDistanceSquared(verts[3], verts[0], point)));
}

#include <chrono>
#include <iostream>

void MVSF_sampler::PerformSampling()
{
#if 0
	auto rooms = GetSamplingRooms(m_metaroom);

	QProgressDialog progress(QString("Sampling Safe Distances..."), QString(""), 0, rooms.size());
	progress.setWindowModality(Qt::WindowModal);

	auto time = std::chrono::system_clock::now();

	for(uint32_t j = 0; j < rooms.size(); ++j)
	{
		auto i = rooms[j];

		for(short y = 0; y < m_size.y; ++y)
		{
			for(short x = 0; x < m_size.x; ++x)
			{

				if(m_pixels[y*m_size.x + x].length2 == 0)
					continue;

				glm::ivec2 point(x * m_stride + m_bounds.x, y * m_stride + m_bounds.y);

				uint32_t distance2 = GetDistanceSquaredToRoom(point, &m_metaroom->m_verts[i]);

				if(distance2 < m_pixels[y*m_size.x + x].length2)
				{
					m_pixels[y*m_size.x + x].length2 = distance2;
					m_pixels[y*m_size.x + x].room_id = i;
				}
			}
		}

		progress.setValue(j);
 	  QCoreApplication::processEvents();
	}

	 std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - time;
	 std::cerr << "Time to sample room major: " << elapsed.count() << std::endl;

#else
	QProgressDialog progress(QString("Sampling Safe Distances..."), QString("cannot cancel"), 0, m_size.y);
	progress.setWindowModality(Qt::WindowModal);

	auto rooms = GetSamplingRooms(m_metaroom);
	std::vector<glm::ivec2> room_centers(rooms.size());

	for(uint32_t j = 0; j < rooms.size(); ++j)
	{
		room_centers[j]	= m_metaroom->GetCenter(rooms[j]);
	}

	auto time = std::chrono::system_clock::now();

	for(short y = 0; y < m_size.y; ++y)
	{
		for(short x = 0; x < m_size.x; ++x)
		{
			if(m_pixels[y*m_size.x + x].length2 == 0)
				continue;

			glm::ivec2 point(x * m_stride + m_bounds.x, y * m_stride + m_bounds.y);

			for(uint32_t j = 0; j < rooms.size(); ++j)
			{
				if((uint32_t)math::length2(room_centers[j] - point) > m_pixels[y*m_size.x + x].length2)
					continue;

				auto i = rooms[j];

				uint32_t distance2 = GetDistanceSquaredToRoom(point, &m_metaroom->m_verts[i*4]);

				if(distance2 < m_pixels[y*m_size.x + x].length2)
				{
					 m_pixels[y*m_size.x + x].length2 = distance2;
					 m_pixels[y*m_size.x + x].room_id = i;
				}
			}
		}

		progress.setValue(y);

		if(y % 10 == 0)
		  QCoreApplication::processEvents();
	}

	std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - time;
   std::cerr << "Time to sample pixel major: " << elapsed.count() << std::endl;

#endif
}

glm::i16vec4 MVSF_sampler::GetBoundingBox(int id) const
{
	glm::i16vec2 min{SHRT_MAX, SHRT_MAX};
	glm::i16vec2 max{SHRT_MIN, SHRT_MIN};

	glm::i16vec2 i;

	for(i.y = 0; i.y < m_size.y; ++i.y)
	{
		for(i.x = 0; i.x < m_size.x; ++i.x)
		{
			if(m_pixels[i.y*m_size.x + i.x].room_id == id)
			{
				min = glm::min(min, i);
				max = glm::max(max, i);
			}
		}
	}

	min = glm::ivec2(min) * (int)m_stride + glm::ivec2(m_bounds.x, m_bounds.y) - (int)(m_stride*2-1);
	max = glm::ivec2(max) * (int)m_stride + glm::ivec2(m_bounds.x, m_bounds.y) + (int)(m_stride*2-1) ;

	glm::i16vec2 min2, max2;
	m_metaroom->GetFaceAABB(id, min2, max2);
/*
	if(!(min.x <= min2.x && max2.x <= max.x
	&&   min.y <= min2.y && max2.y <= max.y))
	{
		throw std::runtime_error("calculated bounding boxes incorrectly :(");
	}*/

	min = glm::min(min, min2);
	max = glm::max(max, max2);

	return glm::i16vec4(min.x, min.y, max.x, max.y);
}
