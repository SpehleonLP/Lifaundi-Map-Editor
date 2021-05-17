#include "color_rooms.h"


int ColorRooms::GetMaxDegree(std::vector<DoorList> const& indicies, int * room)
{
	int best_room{-1};
	int best_degree{};

	//find the one with the highest degree
	for(uint32_t i = 0; i < indicies.size(); i += 4)
	{
		int degree =
				indicies[i+0].length +
				indicies[i+1].length +
				indicies[i+2].length +
				indicies[i+3].length ;

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

	for(uint32_t i = 0; i < r.size(); ++i)
	{
		for(uint32_t j = 0; j < 4; ++j)
		{
			Door const* begin = doors.data() + indices[j].index;
			Door const* end   = begin + indices[j].length;

			for(;begin < end; ++begin)
			{
				if(begin->face >= 0)
					r[i].push_back(begin->face);
			}
		}
	}

	return r;
}

uint32_t ColorRooms::GetColorFlags(Range range, int room_id, std::vector<int8_t> const& color, std::vector<uint8_t> const& edge_flags)
{
	if((edge_flags[room_id] & 0x03) == 3 || (edge_flags[room_id] & 0x0C) == 0xC)
		return 1;

	uint32_t bits = 0x00;

	Range r2 = range;
	for(range.setRoom(room_id); !range.empty(); range.popFront())
	{
		auto front = range.front();
		assert((uint32_t)front < color.size());

		if(color[front] >= 0)
			bits |= 1 << color[front];

		for(r2.setRoom(front); !r2.empty(); r2.popFront())
		{
			auto front = r2.front();
			assert((uint32_t)front < color.size());

			if(color[front] >= 0)
				bits |= 1 << color[front];
		}
	}

	if(edge_flags[room_id] & 0x06)
		return 0x07 ^ bits;

	return 0x03F8 ^ bits;
}

std::vector<int8_t> ColorRooms::DoColoring(std::vector<Door> const& doors, std::vector<DoorList> const& indices, std::vector<uint8_t> const& edge_flags)
{
	std::vector<int8_t> r;
	if(!DoColoring(r, doors, indices, edge_flags))
		throw std::runtime_error("Coloring failed");

	CheckColoring(r, doors, indices);
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

	return r;
}

bool ColorRooms::DoColoring(std::vector<int8_t> & r, std::vector<uint32_t> & face_queue, StackFrame & frame, Range range, std::vector<uint8_t> const& edge_flags)
{
	auto flags = GetColorFlags(range, frame.face_id, r, edge_flags);

//no available colors
	if(flags == 0)
		return false;

	for(; frame.color < 4; ++frame.color)
	{
		auto c = 1 << frame.color;
		auto f = flags & c;
		if(0 == f)
			continue;

		r[frame.face_id] = frame.color;
		for(range.setRoom(frame.face_id); !range.empty(); range.popFront())
		{
			if(r[range.front()] < 0)
				face_queue.push_back(range.front());
		}

		return true;
	}

	return false;
}

bool ColorRooms::DoColoring(std::vector<int8_t> & r, std::vector<Door> const& doors, std::vector<DoorList> const& indices, std::vector<uint8_t> const& edge_flags)
{
	r.clear();
	r.resize(indices.size()/4, -1);

	Range range(doors, indices);

	std::vector<StackFrame> stack;
	std::vector<uint32_t>   face_queue;
	stack.push_back({0, 0});

	while(stack.size())
	{
//no available colors
		if(!DoColoring(r, face_queue, stack.back(), range, edge_flags))
		{
			stack.pop_back();
			continue;
		}

		for(;face_queue.size(); face_queue.pop_back())
		{
			if(r[face_queue.back()] < 0)
			{
				stack.push_back({face_queue.back(), 0});
				face_queue.pop_back();
				goto end_loop;
			}
		}

		for(uint32_t i = 0; i < r.size(); ++i)
		{
			if(r[i] < 0)
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

void ColorRooms::CheckColoring(std::vector<int8_t> const& r, std::vector<Door> const& doors, std::vector<DoorList> const& indices)
{
	Range range(doors, indices);

	for(uint32_t i = 0; i < r.size(); ++i)
	{
		if(r[i] < 0)
			throw std::runtime_error("room not colored");

		for(range.setRoom(i); !range.empty(); range.popFront())
		{
			if(r[range.front()] == r[i])
				throw std::runtime_error("two adjacent rooms have same color");
		}
	}
}
