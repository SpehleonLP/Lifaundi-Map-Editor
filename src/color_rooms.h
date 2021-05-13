#ifndef COLOR_ROOMS_H
#define COLOR_ROOMS_H
#include "quadtree.h"


namespace ColorRooms
{
struct Range;
typedef QuadTree::Door Door;
typedef QuadTree::DoorList DoorList;
	std::vector<std::vector<int>> GetEdgeList(std::vector<Door> const& doors, std::vector<DoorList> const& indices);

	bool DoColoring(std::vector<int8_t> & r, std::vector<Door> const& doors, std::vector<DoorList> const& indicies, std::vector<uint8_t> const& edge_flags);
	bool DoColoring(std::vector<int8_t> & r, std::vector<Door> const& doors, std::vector<DoorList> const& indicies);
	void CheckColoring(std::vector<int8_t> const& r, std::vector<Door> const& doors, std::vector<DoorList> const& indicies);
	int GetMaxDegree( std::vector<DoorList> const& indicies, int * room);
	uint32_t GetColorFlags(Range range, int room_id, const std::vector<int8_t> & color);

	struct Range
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

				face = 0;
			} while(++wall < 4);
		}

	private:
		Door const* doors;
		DoorList const* indices;
		uint32_t room_id;
		uint8_t wall;
		int16_t face;
	};
};

#endif // COLOR_ROOMS_H
