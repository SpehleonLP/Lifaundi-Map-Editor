#include "commandlist.h"
#include "document.h"
#include "metaroom.h"
#include <glm/glm.hpp>

TransformCommand::TransformCommand(Document * document) :
	metaroom(&document->m_metaroom),
	indices(metaroom->m_selection.GetVertSelection())
{
	verts = std::unique_ptr<glm::ivec2[]>(new glm::ivec2[indices.size()]);

	for(size_t i = 0; i < indices.size(); ++i)
	{
		verts[i] = metaroom->m_prev_verts[indices[i]];
	}

	metaroom->CommitMove();
}

void TransformCommand::RollForward()
{
	metaroom->m_selection.SetVertexSelection(indices);

	for(size_t i = 0; i < indices.size(); ++i)
	{
		std::swap(verts[i], metaroom->m_verts[indices[i]]);
	}

	metaroom->CommitMove();
}

void TransformCommand::RollBack()
{
	metaroom->m_selection.SetVertexSelection(indices);

	for(size_t i = 0; i < indices.size(); ++i)
	{
		std::swap(verts[i], metaroom->m_verts[indices[i]]);
	}

	metaroom->CommitMove();
}

DeleteCommand::DeleteCommand(Document * document, std::vector<int> && selection) :
	metaroom(&document->m_metaroom),
	rooms(Clipboard::Extract(metaroom, selection)),
	indices(std::move(selection))
{
	DeleteCommand::RollForward();
}

void DeleteCommand::RollForward()
{
	metaroom->RemoveFaces(indices);
}

void DeleteCommand::RollBack()
{
	metaroom->Insert(rooms, indices);
}

InsertCommand::InsertCommand(Document * document, std::vector<Room> && insertion) :
	metaroom(&document->m_metaroom),
	rooms(std::move(insertion))
{
	for(auto & r : rooms)
		r.uuid = ++(metaroom->m_uuid_counter);

	InsertCommand::RollForward();
}

void InsertCommand::RollForward()
{
	first = metaroom->Insert(rooms);
}

void InsertCommand::RollBack()
{
	metaroom->faces = first;
	metaroom->m_tree.SetDirty();
	metaroom->m_dirty = true;

	metaroom->m_selection.clear();
}



SliceCommand::SliceCommand(Document * document, std::vector<SliceInfo> && _slice) :
	metaroom(&document->m_metaroom),
	slice(std::move(_slice))
{
	SliceCommand::RollForward();
}

void SliceCommand::RollForward()
{
	first = metaroom->Slice(slice);
	metaroom->CommitMove();

	metaroom->m_selection.clear();

	for(size_t i = 0; i < slice.size(); i += 2)
	{
		int j = first + i/2;

		int e = slice[i].edge % 4;

		metaroom->m_selection.select_vertex(Metaroom::NextInEdge(slice[i].edge), Bitwise::OR);
		metaroom->m_selection.select_vertex(slice[i+1].edge, Bitwise::OR);

		metaroom->m_selection.select_vertex(j*4 + e, Bitwise::OR);
		metaroom->m_selection.select_vertex(j*4 + (e+3)%4, Bitwise::OR);
	}
}

void SliceCommand::RollBack()
{
	metaroom->faces = first;
	for(size_t i = 0; i < slice.size(); i += 2)
	{
		std::swap(metaroom->m_verts[Metaroom::NextInEdge(slice[i].edge)], slice[i  ].vertex);
		std::swap(metaroom->m_verts[slice[i+1].edge], slice[i+1].vertex);
	}

	metaroom->CommitMove();

	metaroom->m_selection.clear();

	for(uint i = 0; i < slice.size(); i += 2)
	{
		metaroom->m_selection.select_face(slice[i].edge / 4, Bitwise::OR);
	}
}

ReorderCommand::ReorderCommand(Document * document, std::vector<int> && ordering) :
	metaroom(&document->m_metaroom),
	indices(std::move(ordering))
{
	ReorderCommand::RollForward();
}

void ReorderCommand::RollForward()
{
	std::vector<int> tmp = indices;
	std::sort(tmp.begin(), tmp.end());

	metaroom->Duplicate(indices);

//transfer uuid
	for(uint32_t i = 0; i < indices.size(); ++i)
		metaroom->m_uuid[first+i] = metaroom->m_uuid[indices[i]];

	metaroom->RemoveFaces(tmp);

	metaroom->CommitMove();
	metaroom->m_selection.clear();

	for(size_t i = metaroom->faces - indices.size(); i < metaroom->faces; ++i)
	{
		metaroom->m_selection.select_face(i, Bitwise::SET);
	}
}

void ReorderCommand::RollBack()
{
	std::vector<int> tmp = indices;
	std::sort(tmp.begin(), tmp.end());

	uint32_t first = metaroom->size();
	metaroom->Expand(tmp);

	for(uint32_t i = 0; i < indices.size(); ++i)
	{
		metaroom->CopyRoom(indices[i], first+i);
		metaroom->m_uuid[indices[i]] = metaroom->m_uuid[first+i];
	}

	metaroom->faces = first;

	metaroom->CommitMove();
	metaroom->m_selection.clear();

	for(auto i : indices)
	{
		metaroom->m_selection.select_face(i, Bitwise::SET);
	}
}

SettingCommand::SettingCommand(Document * document, std::vector<int> && list, uint32_t value, Type type) :
	metaroom(&document->m_metaroom),
	indices(std::move(list)),
	value(value),
	type(type)
{
	prev_values.reset(new uint32_t[indices.size()]);

	switch(type)
	{
	case Type::Gravity:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_gravity[indices[i]];
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_music[indices[i]];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_roomType[indices[i]];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_directionalShade[indices[i]];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_ambientShade[indices[i]];
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&prev_values[i], &metaroom->m_audio[indices[i]], 4);
		break;
	default:
		break;
	}

	SettingCommand::RollForward();

}

void SettingCommand::RollForward()
{
	switch(type)
	{
	case Type::Gravity:
		for(auto i : indices)
			metaroom->m_gravity[i] = value;
		metaroom->m_dirty = true;
		break;
	case Type::Track:
		for(auto i : indices)
			metaroom->m_music[i]   = value;
		break;
	case Type::RoomType:
		for(auto i : indices)
			metaroom->m_roomType[i] = value;
		break;
	case Type::Shade:
		for(auto i : indices)
			metaroom->m_directionalShade[i] = value;
		break;
	case Type::AmbientShade:
		for(auto i : indices)
			metaroom->m_ambientShade[i] = value;
		break;
	case Type::Audio:
		for(auto i : indices)
			memcpy(&metaroom->m_audio[i], &value, 4);
		break;
	default:
		break;
	}
}

void SettingCommand::RollBack()
{
	switch(type)
	{
	case Type::Gravity:
		for(size_t i = 0; i < indices.size(); ++i)
			 metaroom->m_gravity[indices[i]] = prev_values[i];
		metaroom->m_dirty = true;
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_music[indices[i]]  = prev_values[i];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_roomType[indices[i]] = prev_values[i];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_directionalShade[indices[i]]  = prev_values[i];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_ambientShade[indices[i]] = prev_values[i];
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->m_audio[indices[i]], &prev_values[i], 4);
		break;
	default:
		break;
	}

}

PermeabilityCommand::PermeabilityCommand(Document * document, std::vector<std::pair<int, int>> && doors, int perm_value) :
	metaroom(&document->m_metaroom),
	length(doors.size()),
	permeability(perm_value),
	heap(new uint8_t[(sizeof(uint64_t)+1) * doors.size()]),
	keys((uint64_t*)&heap[0]),
	values(&heap[sizeof(uint64_t)*doors.size()])
{
	for(uint i = 0; i < length; ++i)
		keys[i] = metaroom->GetDoorKey(doors[i].first, doors[i].second);

//fill array with previous values
	memset(&values[0], 100, length);

	uint32_t i{}, j{};

	while(i < length && j < metaroom->m_permeabilities.size())
	{
		if(metaroom->m_permeabilities[j].first < keys[i])
			++j;
		else if(metaroom->m_permeabilities[j].first > keys[i])
			++j;
		else
		{
			values[i] = metaroom->m_permeabilities[j].second;
			++i, ++j;
		}
	}

	PermeabilityCommand::RollForward();
}

void PermeabilityCommand::SetValue(int v)
{
	if(permeability == v)
		return;

	permeability = v;
	RollForward();
}

void  PermeabilityCommand::RollForward()
{
	InsertNewValues();
	RemoveDoubles();
	metaroom->m_tree.SetVBODirty();
}

void  PermeabilityCommand::RollBack()
{
	InsertOriginalValues();
	RemoveDoubles();
	metaroom->m_tree.SetVBODirty();
}

void PermeabilityCommand::InsertOriginalValues()
{
	auto & array = metaroom->m_permeabilities;

	array.reserve(array.size() + length);

	for(uint32_t i = 0; i < length; ++i)
		array.push_back({keys[i], values[i] + 101});

	std::sort(array.begin(), array.end(), [](auto const& a, auto const& b)
	{
		if(a.first != b.first)
			return a.first < b.first;

		return a.second < b.second;
	});
}

void PermeabilityCommand::InsertNewValues()
{
	auto & array = metaroom->m_permeabilities;

	array.reserve(array.size() + length);

	for(uint32_t i = 0; i < length; ++i)
		array.push_back({keys[i], permeability+101});

	std::sort(array.begin(), array.end(), [](auto const& a, auto const& b)
	{
		if(a.first != b.first)
			return a.first < b.first;

		return a.second < b.second;
	});
}

void PermeabilityCommand::RemoveDoubles()
{
	auto & array = metaroom->m_permeabilities;

	uint32_t write{0};
	for(uint32_t read=0, next = 0; read < array.size(); read = next, ++write)
	{
		for(next = read+1; next < array.size() && array[read].first == array[next].first; ++next) {}

		if(write != next-1)
			array[write] = array[next-1];

		array[write].second -= 101 * (array[write].second > 100);
	}

	array.resize(write);
}

bool PermeabilityCommand::IsSelection(std::vector<std::pair<int, int>> const& list) const
{
	if(length != list.size())
		return false;

	for(uint32_t i = 0; i < length; ++i)
	{
		if(keys[i] != metaroom->GetDoorKey(list[i].first, list[i].second))
			return false;
	}

	return true;
}

std::unique_ptr<CommandInterface> DifferentialSetCommmand::GravityCommand(Document * document, std::vector<int> && list, float value, Type type)
{
	assert(type == Type::GravityAngle || type == Type::GravityStrength || type == Type::ShadeAngle || type == Type::ShadeStrength);
	std::vector<uint32_t> values(list.size());

	bool unique = true;
	glm::vec2 first;

	if(type == Type::GravityAngle)
	{
		glm::vec2 direction(std::cos(value), std::sin(value));

		for(size_t i = 0; i < list.size(); ++i)
		{
			glm::vec2 grav = glm::unpackHalf2x16(document->m_metaroom.m_gravity[list[i]]);
			grav = direction * glm::length(grav);
			values[i] = glm::packHalf2x16(grav);

			if(i == 0) first = grav;
			else		unique &= (glm::dot(grav, -first) < .004);
		}
	}
	else if(type == Type::GravityStrength)
	{
		for(size_t i = 0; i < list.size(); ++i)
		{
			glm::vec2 grav = glm::unpackHalf2x16(document->m_metaroom.m_gravity[list[i]]);
			grav = value * glm::normalize(grav);
			values[i] = glm::packHalf2x16(grav);

			if(i == 0) first = grav;
			else		unique &= (glm::dot(grav, -first) < .004);
		}
	}

	if(unique == true)
	{
		return std::unique_ptr<CommandInterface>(new SettingCommand(document, std::move(list), values[0], SettingCommand::Type::Gravity));
	}

	return std::unique_ptr<CommandInterface>(new DifferentialSetCommmand(document, std::move(list), std::move(values), Type::Gravity));
}

std::unique_ptr<CommandInterface> DifferentialSetCommmand::ShadeCommand(Document * document, std::vector<int> && list, float value, Type type)
{
	assert(type == Type::ShadeAngle || type == Type::ShadeStrength);
	std::vector<uint32_t> values(list.size());

	bool unique = true;
	glm::vec2 first;

	if(type == Type::GravityAngle)
	{
		glm::vec2 direction(std::cos(value), std::sin(value));

		for(size_t i = 0; i < list.size(); ++i)
		{
			glm::vec2 grav = glm::unpackHalf2x16(document->m_metaroom.m_directionalShade[list[i]]);
			grav = direction * glm::length(grav);
			values[i] = glm::packHalf2x16(grav);

			if(i == 0) first = grav;
			else		unique &= (glm::dot(grav, -first) < .004);
		}
	}
	else if(type == Type::GravityStrength)
	{
		for(size_t i = 0; i < list.size(); ++i)
		{
			glm::vec2 grav = glm::unpackHalf2x16(document->m_metaroom.m_directionalShade[list[i]]);
			grav = value * glm::normalize(grav);
			values[i] = glm::packHalf2x16(grav);

			if(i == 0) first = grav;
			else		unique &= (glm::dot(grav, -first) < .004);
		}
	}

	if(unique == true)
	{
		return std::unique_ptr<CommandInterface>(new SettingCommand(document, std::move(list), values[0], SettingCommand::Type::Shade));
	}

	return std::unique_ptr<CommandInterface>(new DifferentialSetCommmand(document, std::move(list), std::move(values), Type::Shade));
}


DifferentialSetCommmand::DifferentialSetCommmand(Document * document, std::vector<int> && list, std::vector<uint32_t> && value, Type type) :
	metaroom(&document->m_metaroom),
	new_values(std::move(value)),
	indices(std::move(list)),
	type(type)
{
	prev_values.reset(new uint32_t[indices.size()]);

	switch(type)
	{
	case Type::Gravity:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_gravity[indices[i]];
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_music[indices[i]];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_roomType[indices[i]];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->m_directionalShade[indices[i]];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&prev_values[i], &metaroom->m_ambientShade[indices[i]], 4);
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&prev_values[i], &metaroom->m_audio[indices[i]], 4);
		break;
	default:
		break;
	}

	DifferentialSetCommmand::RollForward();

}

void DifferentialSetCommmand::RollForward()
{
	switch(type)
	{
	case Type::Gravity:
		for(size_t i = 0; i < indices.size(); ++i)
			 metaroom->m_gravity[indices[i]] = new_values[i];
		metaroom->m_dirty = true;
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_music[indices[i]]  = new_values[i];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_roomType[indices[i]] = new_values[i];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_directionalShade[indices[i]] = new_values[i];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->m_ambientShade[indices[i]], &new_values[i], 4);
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->m_audio[indices[i]], &new_values[i], 4);
		break;
	default:
		break;
	}

}

void DifferentialSetCommmand::RollBack()
{
	switch(type)
	{
	case Type::Gravity:
		for(size_t i = 0; i < indices.size(); ++i)
			 metaroom->m_gravity[indices[i]] = prev_values[i];
		metaroom->m_dirty = true;
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_music[indices[i]]  = prev_values[i];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_roomType[indices[i]] = prev_values[i];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->m_directionalShade[indices[i]]  = prev_values[i];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->m_ambientShade[indices[i]], &prev_values[i], 4);
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->m_audio[indices[i]], &prev_values[i], 4);
		break;
	default:
		break;
	}
}
