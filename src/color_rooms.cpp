#include "color_rooms.h"


int ColorRooms::GetMaxDegree(int * room) const
{
	int best_room{-1};
	int best_degree{};

	//find the one with the highest degree
	for(uint32_t i = 0; i < indices.size(); i += 4)
	{
		int degree =
				indices[i+0].length +
				indices[i+1].length +
				indices[i+2].length +
				indices[i+3].length ;

		if(degree > best_degree)
		{
			best_degree = degree;
			best_room   = i / 4;
		}
	}

	if(room) *room = best_room;
	return best_degree;
}

std::vector<std::vector<int>> ColorRooms::GetEdgeList(std::vector<Door> const& doors, std::vector<DoorList> const& indices)
{
	std::vector<std::vector<int>>  r;
	r.resize(indices.size()/4);

	Range range(doors, indices);
	Range r2(doors, indices);

	for(uint32_t i = 0; i < r.size(); ++i)
	{
		std::vector<int> v;

		for(range.setRoom(i); !range.empty(); range.popFront())
		{
			v.push_back(range.front());

			for(r2.setRoom(range.front()); !r2.empty(); r2.popFront())
			{
				if((uint32_t)r2.front() != i)
					v.push_back(r2.front());
			}
		}

		std::sort(v.begin(), v.end());

		uint32_t write = 0, read = 0, j;
		for(;read < v.size(); read = j)
		{
			for(j = read; j < v.size() && v[j] == v[read]; ++j) {}
			v[write++] = v[read];
		}

		v.resize(write);
		r[i] = std::move(v);
	}

	return r;
}

uint64_t ColorRooms::GetColorFlags(int room_id) const
{
	if((edge_flags[room_id] & 0x03) == 3 || (edge_flags[room_id] & 0x0C) == 0xC)
		return 1;

	uint32_t bits = 0x00;

	for(auto e : edges[room_id])
	{
		if(coloring[e] >= 0)
			bits |= 1 << coloring[e];
	}

	if(edge_flags[room_id] & 0x06)
		return 0x0E & ~bits;

	return 0xFFFFFFFFFFFFFFF0 & ~bits;
}

int32_t ColorRooms::MaxColorUsed() const
{
	int8_t r = 0;

	for(auto c : coloring)
		r = std::max(c, r);

	return r;
}

void ColorRooms::DoColoring(std::shared_ptr<ColorProgress> progress)
{

	Range range(doors, indices);
	auto edges = GetEdgeList(doors, indices);

	coloring.clear();
	coloring.resize(edges.size(), -1);
	auto islands = GetIslands();

	progress->_totalIslands = islands.size();
	progress->_processedIslands = 0;

	for(auto & island : islands)
	{
		if(!DoColoringInternal(island, progress))
			throw std::runtime_error("Coloring failed");

		++(progress->_processedIslands);
		(progress->_totalBacktrack) += progress->_backtrackInIsland;
		progress->_backtrackInIsland = 0;
	}

	CheckColoring();

	progress->noColors = MaxColorUsed();
	progress->coloring = std::move(coloring);
	progress->_complete = true;

/*
	assert(r.size() == edge_flags.size());

	for(uint32_t i = 0; i < edge_flags.size(); ++i)
	{
//color edges differently for stitching.
		if(edge_flags[i] & 0x06)
			r[i] += 4;

//give a special color to things that span
		if((edge_flags[i] & 0x03) == 3 || (edge_flags[i] & 0x0C) == 0xC)
			r[i] = 9;
	}*/
}


void InsertUnique(std::vector<uint32_t> & vec, uint32_t item)
{
	for(uint32_t i = 0; i < vec.size(); ++i)
	{
		if(vec[i] == item)
			return;
	}

	vec.insert(vec.begin(), item);
}

std::vector<std::vector<ColorRooms::StackFrame>>  ColorRooms::GetIslands() const
{
	Range range(doors, indices);
	std::vector<std::vector<StackFrame>> r;
	std::vector<uint8_t> marks(edges.size(), 0);

// this is just a BFS
	for(uint32_t i = 0; i < marks.size(); ++i)
	{
		if(marks[i] != 0) continue;
		marks[i] = 1;

		std::vector<uint32_t> island;
		island.push_back(i);
		bool flags = (edge_flags[i] & 0x06) != 0;

		for(uint32_t j = 0; j < island.size(); ++j)
		{
			// iterate over face room this face is attached to.
			for(range.setRoom(island[j]); !range.empty(); range.popFront())
			{
				if(marks[range.front()] == 0 && ((edge_flags[range.front()] & 0x06) != 0) == flags)
				{
					marks[range.front()] = 1;
					island.push_back(range.front());
				}
			}
		}

/* now we have an island in BFS order, we want ordered such that:
*	the smallest face from the island where it is attached to something in the list is added to the list
*	if the graph is optimized for the navier stokes caching, this order should also get a linear 4 coloring almost every time.
*/
		std::sort(island.begin(), island.end(), [](auto a, auto b)
		{
			return a > b;
		});

		std::vector<StackFrame> sorted_island;
		sorted_island.reserve(island.size());

		sorted_island.push_back({island.back(), 0});
		island.pop_back();

		while(island.size())
		{
			for(int j = island.size()-1; j >= 0; --j)
			{
	// iterate over each thing
				for(range.setRoom(island[j]); !range.empty(); range.popFront())
				{
					if(std::find(sorted_island.rbegin(), sorted_island.rend(), range.front()) != sorted_island.rend())
					{
						sorted_island.push_back({island[j], 0});
						island.erase(island.begin() + j);
						goto repeat;
					}
				}
			}

			assert(false);

repeat:
			(void)0;
		}

		r.push_back(std::move(sorted_island));
	}

	return r;
}


bool ColorRooms::DoColoringInternal(std::vector<StackFrame> & stack, std::shared_ptr<ColorProgress> progress)
{
	progress->_totalRooms = stack.size();

	for(uint32_t i = 0; i < stack.size(); ++i)
	{
		progress->_currentRoom = i;

//no available colors
		while(!DoColoring(stack[i]))
		{
			if(--i > stack.size())
			{
				return false;
			}
			else
			{
				progress->_backtrackInIsland += 1;
				progress->_currentRoom = i;
			}
		}
	}

	return true;
}

bool ColorRooms::DoColoring(StackFrame & frame)
{
	Range range(doors, indices);
	auto flags = GetColorFlags(frame.face_id);

//no available colors
	if(flags == 0)
		return false;

	for(;; ++frame.color)
	{
		uint64_t c = 1 << frame.color;

		if(0 == (flags & c))
			continue;
		else if(c > flags)
			break;

		coloring[frame.face_id] = frame.color;
		return true;
	}

	frame.color = 0;
	return false;
}

void ColorRooms::CheckColoring() const
{
	for(uint32_t i = 0; i < edges.size(); ++i)
	{
		if(coloring[i] < 0)
			throw std::runtime_error("room not colored");

		if(coloring[i] == 0)
			continue;

		for(auto e : edges[i])
		{
			if(coloring[e] == coloring[i])
			{
				auto & v_0 = edges[i];
				auto & v_1 = edges[e];

				(void)v_0;
				(void)v_1;

				throw std::runtime_error("two adjacent rooms have same color");
			}
		}
	}
}
