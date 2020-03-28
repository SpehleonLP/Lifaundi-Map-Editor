#ifndef WALLTYPE_DB_H
#define WALLTYPE_DB_H
#include "nlohmann/json_detail.hpp"

class QStringList;

namespace Ui {
class MainWindow;
}


namespace MapData
{

struct Header
{
	std::string copyright{};
	float       version{ 1.0 };
	uint32_t    magic{};

	bool empty() { return false; }
};

struct DoorType
{
	std::string name;
	uint8_t     permeability{};

	bool empty() { return false; }
};

struct RoomType
{
	std::string name;
	uint8_t     permeability{};

	bool empty() { return false; }
};

struct WallTypeDB
{
	enum
	{
		MAX_WALL_TYPES = 16,
		MAX_ROOM_TYPES = 16
	};

	Header header;

	std::vector<RoomType> room_types;
	std::vector<DoorType> door_types;

	QStringList GetRoomTypes() const;
	QStringList GetDoorTypes() const;

	void UpdateTypeLists(Ui::MainWindow *);

	bool empty() { return room_types.empty() && door_types.empty(); }
};

void from_json(const nlohmann::json & json, WallTypeDB & extras);

}


#endif // WALLTYPE_DB_H
