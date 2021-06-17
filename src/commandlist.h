#ifndef COMMANDLIST_H
#define COMMANDLIST_H
#include "clipboard.h"
#include <glm/vec2.hpp>
#include <vector>
#include <memory>

class Metaroom;
class Document;
struct SliceInfo;

class CommandInterface
{
public:
	CommandInterface() = default;
	virtual ~CommandInterface() {}

	virtual void RollForward() = 0;
	virtual void RollBack() = 0;
};

class TransformCommand : public CommandInterface
{
public:
	TransformCommand(Document * document);
	virtual ~TransformCommand() = default;

	void RollForward();
	void RollBack();

private:
	Metaroom                    * metaroom;
	std::unique_ptr<glm::ivec2[]> verts;
	std::vector<int>              indices;
};

class DeleteCommand : public CommandInterface
{
public:
	DeleteCommand(Document * document, std::vector<int> && selection);
	virtual ~DeleteCommand() = default;

	void RollForward();
	void RollBack();

private:
	Metaroom          * metaroom;
	std::vector<Room>   rooms;
	std::vector<int>    indices;
};

class InsertCommand : public CommandInterface
{
public:
	InsertCommand(Document * document, std::vector<Room> && insertion);
	virtual ~InsertCommand() = default;

	void RollForward();
	void RollBack();

private:
	Metaroom          * metaroom;
	std::vector<Room>   rooms;
	int                 first;
};

class SliceCommand : public CommandInterface
{
public:
	SliceCommand(Document * document, std::vector<SliceInfo> && slice);
	virtual ~SliceCommand() = default;

	void RollForward();
	void RollBack();

private:
	Metaroom *                    metaroom;
	std::vector<SliceInfo>        slice;
	uint32_t                      first{};
};

class ReorderCommand : public CommandInterface
{
public:
	ReorderCommand(Document * document, std::vector<int> && ordering);
	virtual ~ReorderCommand() = default;

	void RollForward();
	void RollBack();

private:
	Metaroom *                    metaroom;
	std::vector<int>              indices;
	uint32_t                      first{};
};

class PermeabilityCommand : public CommandInterface
{
public:
	PermeabilityCommand(Document * document, std::vector<std::pair<int, int>> && doors, int perm_value);
	virtual ~PermeabilityCommand() = default;

	void RollForward();
	void RollBack();

	bool IsSelection(std::vector<std::pair<int, int>> const& list) const;
	void SetValue(int);

private:
	void InsertOriginalValues();
	void InsertNewValues();
	void RemoveDoubles();

	Metaroom *                                 metaroom;
	uint32_t                                   length;
	uint8_t                                    permeability;
	std::unique_ptr<uint8_t[]>                 heap;
	uint64_t                                 * keys;
	uint8_t                                  * values;
};

class SettingCommand : public CommandInterface
{
public:
	enum class Type
	{
		Gravity,
		Track,
		RoomType,
		DrawDistance,
		DoorType,
	};

	SettingCommand(Document * document, std::vector<int> && list, uint32_t value, Type type);
	virtual ~SettingCommand() = default;

	void RollForward();
	void RollBack();

	bool IsSelection(std::vector<int> const& list) const { return list == indices; }
	bool IsType(Type value) const { return type == value; }
	void SetValue(uint32_t it)
	{
		value = it;
		RollForward();
	}

private:

	Metaroom                  * metaroom;
	std::unique_ptr<uint32_t[]> prev_values;
	std::vector<int>            indices;

	uint32_t                value;
	Type                    type;
};

class DifferentialSetCommmand : public CommandInterface
{
public:
	enum class Type
	{
		Gravity,
		Track,
		RoomType,
		DrawDistance,
		DoorType,
		GravityAngle,
		GravityStrength,
	};

	static std::unique_ptr<CommandInterface> GravityCommand(Document * document, std::vector<int> && list, float value, Type type);
	DifferentialSetCommmand(Document * document, std::vector<int> && list, std::vector<uint32_t> && value, Type type);
	virtual ~DifferentialSetCommmand() = default;

	void RollForward();
	void RollBack();

	bool IsSelection(std::vector<int> const& list) const { return list == indices; }
	bool IsType(Type value) const { return type == value; }

private:
	Metaroom                  * metaroom;
	std::unique_ptr<uint32_t[]> prev_values;
	std::vector<uint32_t>		new_values;
	std::vector<int>            indices;

	Type                    type;
};


#endif // COMMANDLIST_H
