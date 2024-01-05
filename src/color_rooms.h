#ifndef COLOR_ROOMS_H
#define COLOR_ROOMS_H
#include "quadtree.h"

class ColorRooms
{
struct Range;
struct StackFrame;
typedef QuadTree::Door Door;
typedef QuadTree::DoorList DoorList;
	static std::vector<std::vector<int>> GetEdgeList(std::vector<Door> const& doors, std::vector<DoorList> const& indices);

public:
	ColorRooms(std::vector<Door> & doors, std::vector<DoorList> & indices, std::vector<uint8_t> && edge_flags) :
		doors(doors),
		indices(indices),
		edge_flags(std::move(edge_flags)),
		edges(GetEdgeList(doors, indices))
	{
	}


	void DoColoring();
	std::vector<int8_t> const& GetColoring() const { return coloring; }

	int		 GetMaxDegree(int * room) const;
	uint64_t GetColorFlags(int room_id) const;
	int      MaxColorUsed() const;
	void	 CheckColoring() const;

private:
	bool DoColoringInternal(std::vector<StackFrame> & face_queue);
	bool DoColoring(StackFrame & frame);
	std::vector<std::vector<StackFrame> > GetIslands() const;

	std::vector<Door>				const& doors;
	std::vector<DoorList>			const& indices;
	std::vector<uint8_t>			const  edge_flags;
	std::vector<std::vector<int>>	edges;

	std::vector<int8_t>				coloring;
};

struct ColorRooms::StackFrame
{
	uint32_t face_id;
	 int32_t color;

	bool operator==(uint32_t it) const { return face_id == it; };
};

struct ColorRooms::Range
{
	Range(std::vector<Door> const& doors, std::vector<DoorList> const& indices) :
		doors(doors.data()),
		indices(indices.data())
	{
	}

	void setRoom(int room)
	{
		room_id = room;
		wall = 0;
		face = -1;
		popFront();
	}

	bool empty() const { return wall >= 4; }
	int front() const { return doors[(indices[room_id*4 + wall].index + face)].face; }
	void popFront()
	{
		do
		{
			while(++face < indices[room_id*4 + wall].length)
			{
				if(front() != -1)
					return;
			}

			face = -1;
		} while(++wall < 4);
	}

private:
	Door const* doors;
	DoorList const* indices;
	uint32_t room_id;
	uint8_t wall;
	int16_t face;
};

#endif // COLOR_ROOMS_H
