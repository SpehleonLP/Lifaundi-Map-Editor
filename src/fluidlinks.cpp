#include "fluidlinks.h"
#include "metaroom.h"
#include <algorithm>


static bool DoesContain(int * begin, int * end, int value)
{
	for(; begin != end; ++begin)
	{
		if(*begin == value)
			return true;
	}

	return false;
}

static void RemoveIndex(int * array, std::pair<int, int> * pair, int value)
{
	if(pair->first == pair->second)
		return;

	if(array[pair->second-1] != value)
	{
		auto itr = array + pair->first;
		auto end = array + pair->second;

		for(; itr != end; ++itr )
		{
			if(*itr == value)
			{
				std::swap(*itr, *(end-1));
				break;
			}
		}
	}

	pair->second -= 1;
}

FluidLinks::FluidLinks(Metaroom * metaroom)
{
	std::vector<int> doors;
	std::vector<std::pair<int, int>> indices;

	metaroom->m_tree.GetHallDoors(doors, indices);

//create hall index...
	for(uint32_t i = 0; i < indices.size(); )
	{
		auto begin = i;

		for(; i < indices.size(); ++i)
		{
			if(DoesContain(
				&doors[0] + indices[i].first,
				&doors[0] + indices[i].second, i+1))
			{
				RemoveIndex(&doors[0], &indices[i], i+1);
				continue;
			}

			break;
		}

		if(begin == i)
		{
			++i;
			continue;
		}

		halls.push_back({begin, i});
	}

//for each hall
	for(auto h : halls)
	{
		for(int i = h.first; i < h.second; ++i)
		{
			for(auto itr = &doors[0] + indices[i].first; itr < &doors[0] + indices[i].second; ++itr)
			{
				auto h2 = GetHall(*itr);
				int length = std::min(h.second - i, h2.second - *itr);

				int count = 0;

				while(count < length &&
					!DoesContain(
						&doors[0] + indices[i + count].first,
						&doors[0] + indices[i + count].second,
						*itr + count))
				{
					++count;
				}

				if(count <= 1)
					continue;

				for(int j = 0; j < count; ++j)
				{
					RemoveIndex(&doors[0], &indices[i+j], *itr+j);
					RemoveIndex(&doors[0], &indices[*itr+j], i+j);
				}

				hall_links.push_back({i, *itr, count});
			}
		}
	}

//dump remaining into misc
	for(uint32_t i = 0; i < indices.size(); ++i)
	{
		auto i_begin = &doors[0] + indices[i].first;
		auto i_end   = &doors[0] + indices[i].second;

		std::sort(i_begin, i_end);

		for(auto itr = i_begin; itr != i_end; ++itr)
		{
			if(i < *itr)
				misc.push_back({i, *itr});
		}
	}
}

std::pair<int, int> FluidLinks::GetHall(int room) const
{
	for(uint32_t i = 0; i < halls.size(); ++i)
	{
		if(halls[i].first <= room && room < halls[i].second)
			return halls[i];
	}

	return std::make_pair(0, 0);
}
