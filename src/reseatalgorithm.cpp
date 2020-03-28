#include "reseatalgorithm.h"
#include "metaroom.h"
#include "lf_math.h"
#include <cstdint>

std::vector<int> Reseat(Metaroom * metaroom, std::vector<int> && selection)
{
	std::pair<float, int> begin, end;
	glm::ivec2 verts[2][4];
//	uint8_t    types[2][4];

	std::vector<ReseatData> r(selection.size());

    for(uint32_t i = 0; i < selection.size(); ++i)
	{
		memcpy(verts[0], &metaroom->m_verts[selection[i]*4], sizeof(verts[0]));
//		memcpy(types[0], &metaroom->m_doorType[selection[i]*4], sizeof(types[0]));

		glm::vec2 gravity = metaroom->GetGravity(selection[i]);
		glm::vec2 center  = metaroom->GetCenter(selection[i]);

		if(gravity.x == 0 && gravity.y == 0)
			gravity = glm::vec2(1, 0);
		else
			gravity = glm::normalize(glm::vec2(gravity.y, -gravity.x));

		int   next = -1;
		int   prev = -1;
		float best_next = 0;
		float best_prev = 0;

        for(uint32_t j = i+1; j < selection.size(); ++j)
		{
			memcpy(verts[1], &metaroom->m_verts[selection[j]*4], sizeof(verts[1]));
			glm::vec2 gravity2 = metaroom->GetGravity(selection[i]);

			if(gravity2.x == 0 && gravity2.y == 0)
				gravity2 = glm::vec2(1, 0);
			else
				gravity2 = glm::normalize(glm::vec2(gravity2.y, -gravity2.x));

		//	memcpy(types[1], &metaroom->m_doorType[selection[j]*4], sizeof(types[1]));

			float next_dirn = glm::dot(gravity, center - metaroom->GetCenter(selection[i]));
			float prev_dirn = -glm::dot(gravity2, center - metaroom->GetCenter(selection[i]));

			for(int k = 0; k < 16; ++k)
			{
			//	if(types[0][k%4] != 0 || types[1][k/4] != 0)continue;

				const auto v0 = verts[0][k%4];
				const auto v1 = verts[0][(k+1)%4];

				const auto a0 = verts[1][k/4];
				const auto a1 = verts[1][((k/4)+1)%4];

				if(math::IsOpposite(a1-a0, v1-v0) == false
				|| math::IsColinear(v0, v1, a0, a1) == false
				|| math::GetOverlap(v0, v1-v0, a0, a1, &begin, &end) == false)
					continue;

				const auto vec     = glm::vec2(v1 - v0) * (begin.first - end.first);
				const auto length2 = glm::dot(vec, vec);

				auto weight = length2 * next_dirn;

				if(weight > best_next)
				{
					best_next = weight;
					next      = j;
				}

				weight = length2 * prev_dirn;

				if(-weight > best_prev)
				{
					best_prev =-weight;
					prev      = j;
				}
			}
		}

		if(best_next > r[i].best_next)
		{
			int eax = r[i].next;

			r[i].best_next = best_next;
			r[i].next      = next;

			if(eax >= 0)
			{
				r[eax].best_prev = best_next;
				r[eax].prev     = i;
			}
		}

		if(best_prev > r[i].best_prev)
		{
			int eax = r[i].prev;

			r[i].best_prev = best_prev;
			r[i].prev      = prev;

			if(eax >= 0)
			{
				r[eax].best_next = best_prev;
				r[eax].next     = i;
			}
		}
	}

	return GetReseatingArray(selection, std::move(r));
}




std::vector<int> GetReseatingArray(std::vector<int> const& selection, std::vector<ReseatData> && vec)
{
	std::vector<int> r;
	r.reserve(vec.size());

	for(uint32_t i = 0; i < vec.size(); ++i)
	{
		if(vec[i].prev == -1 && vec[i].next == -1)
			continue;

//get start of the run
		uint32_t j = i;
		while(vec[j].prev != -1) { j = vec[j].prev; }

		while(vec[j].next != -1)
		{
			r.push_back(j);
			auto k = vec[j].next;
			vec[j].prev = vec[j].next = -1;
			j = k;
		}
	}

	for(uint32_t i = 0; i < r.size(); ++i)
		r[i] = selection[r[i]];

	return r;
}
