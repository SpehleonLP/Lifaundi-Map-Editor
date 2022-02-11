#include "walltype_db.h"
#include "ui_mainwindow.h"

namespace MapData
{

inline void from_json(const nlohmann::json & json, Header & db)
{
	detail::ReadRequiredField("version", json, db.version);
	detail::ReadOptionalField("copyright", json, db.copyright);
	detail::ReadRequiredField("magic", json, db.magic);

	if(db.magic != 0x6D6F6F72)
		throw std::runtime_error("file is not a lifaundi room type database");
}

inline void from_json(const nlohmann::json & json, RoomType & db)
{
	detail::ReadRequiredField("name",         json, db.name);
}

inline void from_json(const nlohmann::json & json, DoorType & db)
{
	detail::ReadRequiredField("name",         json, db.name);
	detail::ReadRequiredField("permeability", json, db.permeability);

	db.permeability = std::max(0, std::min(255, 255 * db.permeability / 100));
}

void from_json(const nlohmann::json & json, WallTypeDB & db)
{
	detail::ReadRequiredField("header",  json, db.header);
	detail::ReadRequiredField("room types",   json, db.room_types);
	detail::ReadOptionalField("door types",   json, db.door_types);
}

QStringList WallTypeDB::GetRoomTypes() const
{
	QStringList list;

	if(room_types.empty())
	{
		list<< "Atmosphere"
			<< "Upper Atmosphere"
			<< "Space"

			<< "Wood"
			<< "Steel"
			<< "Plastic"

			<< "Stone"
			<< "Gravel"
			<< "Glass"

			<< "Normal Soil"
			<< "Boggy Soil"
			<< "Sandy Soil";
	}

	uint32_t N = std::min<uint32_t>(MAX_ROOM_TYPES, room_types.size());

	for(uint32_t i = 0; i < N; ++i)
	{
		list << QString::fromStdString(room_types[i].name);
	}

	while(list.size() < MAX_ROOM_TYPES)
		list << QString::number(list.size());

	return list;
}

QStringList WallTypeDB::GetDoorTypes() const
{
	QStringList list;

	if(door_types.empty())
	{
		list << "atmosphere"
			<< "wooden walkway"
			<< "concrete"
			<< "normal soil"
			<< "boggy soil"
			<< "drained soil"
			<< "sandy soil"
			<< "clay soil";
	}

	uint32_t N = std::min<uint32_t>(MAX_WALL_TYPES, room_types.size());

	for(uint32_t i = 0; i < N; ++i)
	{
		list << QString::fromStdString(door_types[i].name);
	}

	while(list.size() < MAX_WALL_TYPES)
		list << QString::number(list.size());

	return list;
}

void WallTypeDB::UpdateTypeLists(Ui::MainWindow * ui)
{
	int current_row = ui->room_type->currentIndex();
	ui->room_type->clear();
	ui->room_type->addItems(GetRoomTypes());
	ui->room_type->setCurrentIndex(current_row);

#if HAVE_WALL_TYPE
	current_row = ui->door_type->currentIndex();
	ui->door_type->clear();
	ui->door_type->addItems(GetDoorTypes());
	ui->door_type->setCurrentIndex(current_row);
#endif
}

}
