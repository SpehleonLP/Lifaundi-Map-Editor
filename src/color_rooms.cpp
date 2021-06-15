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

		if(i == 13 || i == 164)
		{
			int break_point = 0;
			++break_point;
		}

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

	return 0x0000000000FFFFF0 & ~bits;
}

int32_t ColorRooms::NoColorsUsed() const
{
	int8_t r = 0;

	for(auto c : coloring)
		r = std::max(c, r);

	return r;
}

void ColorRooms::DoColoring()
{
	Range range(doors, indices);
	auto edges = GetEdgeList(doors, indices);
	if(!DoColoringInternal())
		throw std::runtime_error("Coloring failed");

	CheckColoring();
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

bool ColorRooms::DoColoringInternal()
{
	coloring.clear();
	coloring.resize(edges.size(), -1);

	std::vector<StackFrame> stack;
	std::vector<uint32_t>   face_queue;
	stack.push_back({0, 0});

	while(stack.size())
	{
//no available colors
		if(!DoColoring(face_queue, stack.back()))
		{
			stack.pop_back();
			continue;
		}

		for(;face_queue.size(); face_queue.pop_back())
		{
			if(coloring[face_queue.back()] < 0)
			{
				stack.push_back({face_queue.back(), 0});
				face_queue.pop_back();
				goto end_loop;
			}
		}

		for(uint32_t i = 0; i < coloring.size(); ++i)
		{
			if(coloring[i] < 0)
			{
				stack.push_back({i, 0});
				goto end_loop;
			}
		}

		return true;

end_loop:
		(void)0L;
	}

	return false;
}

bool ColorRooms::DoColoring(std::vector<uint32_t> & face_queue, StackFrame & frame)
{
	Range range(doors, indices);

	auto flags = GetColorFlags(frame.face_id);

//no available colors
	if(flags == 0)
		return false;

	for(;; ++frame.color)
	{
		uint32_t c = 1 << frame.color;

		if(0 == (flags & c))
			continue;
		else if(c > flags)
			break;

		coloring[frame.face_id] = frame.color;
		for(range.setRoom(frame.face_id); !range.empty(); range.popFront())
		{
			if(coloring[range.front()] < 0)
				face_queue.push_back(range.front());
		}

		return true;
	}

	frame.color = -1;
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

				throw std::runtime_error("two adjacent rooms have same color");
			}
		}
	}
}
