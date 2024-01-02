#include "commandlist.h"
#include "document.h"
#include "metaroom.h"
#include <glm/glm.hpp>

TransformCommand::TransformCommand(Document * document) :
	metaroom(&document->m_metaroom),
	indices(metaroom->_selection.GetVertSelection())
{
	verts = shared_array<glm::ivec2>(indices.size());

	for(size_t i = 0; i < indices.size(); ++i)
	{
		verts[i] = metaroom->prev(i);
	}

	metaroom->CommitMove();
}

void TransformCommand::RollForward()
{
	metaroom->_selection.SetVertexSelection(indices);

	for(size_t i = 0; i < indices.size(); ++i)
	{
		std::swap(verts[i], metaroom->verts(indices[i]));
	}

	_permDelta = metaroom->GetInvalidPermeabilities();
	metaroom->CommitMove();
}

void TransformCommand::RollBack()
{
	metaroom->_selection.SetVertexSelection(indices);

	for(size_t i = 0; i < indices.size(); ++i)
	{
		std::swap(verts[i], metaroom->verts(indices[i]));
	}

	metaroom->AddPermeabilities(_permDelta);
	metaroom->CommitMove();
}

DeleteCommand::DeleteCommand(Document * document, std::vector<uint32_t> && selection) :
	metaroom(&document->m_metaroom),
	rooms(Clipboard::Extract(metaroom, selection)),
	indices(std::move(selection))
{
	_permDelta = metaroom->GetPermeabilities(indices);
	DeleteCommand::RollForward();
}

void DeleteCommand::RollForward()
{
	metaroom->RemoveFaces(indices);
	metaroom->RemovePermeabilities(_permDelta);
}

void DeleteCommand::RollBack()
{
	metaroom->Insert(rooms, indices);
	metaroom->AddPermeabilities(_permDelta);
}

InsertCommand::InsertCommand(Document * document, std::vector<Room> && insertion) :
	metaroom(&document->m_metaroom),
	rooms(std::move(insertion))
{
	InsertCommand::RollForward();
}

void InsertCommand::RollForward()
{
	indices = metaroom->Insert(rooms);
}

void InsertCommand::RollBack()
{
	metaroom->RemoveFaces(indices);
}



SliceCommand::SliceCommand(Document * document, std::vector<SliceInfo> && _slice) :
	metaroom(&document->m_metaroom),
	slice(std::move(_slice))
{
	SliceCommand::RollForward();
}

void SliceCommand::RollForward()
{
	indices = metaroom->Slice(slice);
	metaroom->CommitMove();

	metaroom->_selection.clear();

	for(size_t i = 0; i < slice.size(); i += 2)
	{
		int j = indices[i];

		int e = slice[i].edge % 4;

		metaroom->_selection.select_vertex(Metaroom::NextInEdge(slice[i].edge), Bitwise::OR);
		metaroom->_selection.select_vertex(slice[i+1].edge, Bitwise::OR);

		metaroom->_selection.select_vertex(j*4 + e, Bitwise::OR);
		metaroom->_selection.select_vertex(j*4 + (e+3)%4, Bitwise::OR);
	}
}

void SliceCommand::RollBack()
{
	for(size_t i = 0; i < slice.size(); i += 2)
	{
		std::swap(metaroom->verts(Metaroom::NextInEdge(slice[i].edge)), slice[i  ].vertex);
		std::swap(metaroom->verts(slice[i+1].edge), slice[i+1].vertex);
	}

	metaroom->RemoveFaces(indices);
	metaroom->CommitMove();
	metaroom->_selection.clear();

	for(uint i = 0; i < slice.size(); i += 2)
	{
		metaroom->_selection.select_face(slice[i].edge / 4, Bitwise::OR);
	}
}

ReorderCommand::ReorderCommand(Document * document, std::vector<uint32_t> && ordering) :
	metaroom(&document->m_metaroom),
	indices(std::move(ordering))
{
	ReorderCommand::RollForward();
}

void ReorderCommand::RollForward()
{
	std::vector<uint32_t> tmp = indices;
	std::sort(tmp.begin(), tmp.end());

	original_rooms = Clipboard::Extract(metaroom, tmp);

	metaroom->RemoveFaces(tmp);
	metaroom->Insert(original_rooms, tmp);
}

void ReorderCommand::RollBack()
{
	metaroom->RemoveFaces(indices);
	metaroom->Insert(original_rooms, indices);

	original_rooms.clear();
}

SettingCommand::SettingCommand(Document * document, std::vector<uint32_t> && list, uint32_t value, Type type) :
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
			prev_values[i] = metaroom->_gravity[indices[i]];
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->_music[indices[i]];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->_roomType[indices[i]];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->_directionalShade[indices[i]];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->_ambientShade[indices[i]];
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&prev_values[i], &metaroom->_audio[indices[i]], 4);
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
		metaroom->gl.SetDirty();

		for(auto i : indices)
			metaroom->_gravity[i] = value;
		break;
	case Type::Track:
		for(auto i : indices)
			metaroom->_music[i]   = value;
		break;
	case Type::RoomType:
		for(auto i : indices)
			metaroom->_roomType[i] = value;
		break;
	case Type::Shade:
		for(auto i : indices)
			metaroom->_directionalShade[i] = value;
		break;
	case Type::AmbientShade:
		for(auto i : indices)
			metaroom->_ambientShade[i] = value;
		break;
	case Type::Audio:
		for(auto i : indices)
			memcpy(&metaroom->_audio[i], &value, 4);
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
		metaroom->gl.SetDirty();

		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_gravity[indices[i]] = prev_values[i];
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_music[indices[i]]  = prev_values[i];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_roomType[indices[i]] = prev_values[i];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_directionalShade[indices[i]]  = prev_values[i];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_ambientShade[indices[i]] = prev_values[i];
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->_audio[indices[i]], &prev_values[i], 4);
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
//fill array with previous values
	memset(&values[0], 100, length);

	for(uint i = 0; i < length; ++i)
	{
		keys[i] = metaroom->GetDoorKey(doors[i].first, doors[i].second);

		auto itr = metaroom->_permeabilities.find(keys[i]);

		if(itr != metaroom->_permeabilities.end())
		{
			values[i] = itr->second;
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
	if(permeability == 100)
	{
		for(auto i = 0u; i < length; ++i)
		{
			auto itr = metaroom->_permeabilities.find(keys[i]);

			if(itr != metaroom->_permeabilities.end())
			{
				metaroom->_permeabilities.erase(itr);
			}
		}
	}
	else
	{
		for(auto i = 0u; i < length; ++i)
		{
			metaroom->_permeabilities[keys[i]] = permeability;
		}
	}

	metaroom->_tree.SetVBODirty();
}

void  PermeabilityCommand::RollBack()
{
	for(auto i = 0u; i < length; ++i)
	{
		if(values[i] != 100)
			metaroom->_permeabilities[keys[i]] = values[i];
		else
		{
			auto itr = metaroom->_permeabilities.find(keys[i]);

			if(itr != metaroom->_permeabilities.end())
			{
				metaroom->_permeabilities.erase(itr);
			}
		}
	}

	metaroom->_tree.SetVBODirty();
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

std::unique_ptr<CommandInterface> DifferentialSetCommmand::GravityCommand(Document * document, std::vector<uint32_t> && list, float value, Type type)
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
			glm::vec2 grav = glm::unpackHalf2x16(document->m_metaroom._gravity[list[i]]);
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
			glm::vec2 grav = glm::unpackHalf2x16(document->m_metaroom._gravity[list[i]]);
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

std::unique_ptr<CommandInterface> DifferentialSetCommmand::ShadeCommand(Document * document, std::vector<uint32_t> && list, float value, Type type)
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
			glm::vec2 grav = glm::unpackHalf2x16(document->m_metaroom._directionalShade[list[i]]);
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
			glm::vec2 grav = glm::unpackHalf2x16(document->m_metaroom._directionalShade[list[i]]);
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


DifferentialSetCommmand::DifferentialSetCommmand(Document * document, std::vector<uint32_t> && list, std::vector<uint32_t> && value, Type type) :
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
			prev_values[i] = metaroom->_gravity[indices[i]];
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->_music[indices[i]];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->_roomType[indices[i]];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			prev_values[i] = metaroom->_directionalShade[indices[i]];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&prev_values[i], &metaroom->_ambientShade[indices[i]], 4);
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&prev_values[i], &metaroom->_audio[indices[i]], 4);
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
		metaroom->gl.SetDirty();

		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_gravity[indices[i]] = new_values[i];
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_music[indices[i]]  = new_values[i];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_roomType[indices[i]] = new_values[i];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_directionalShade[indices[i]] = new_values[i];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->_ambientShade[indices[i]], &new_values[i], 4);
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->_audio[indices[i]], &new_values[i], 4);
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
		metaroom->gl.SetDirty();

		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_gravity[indices[i]] = prev_values[i];
		break;
	case Type::Track:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_music[indices[i]]  = prev_values[i];
		break;
	case Type::RoomType:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_roomType[indices[i]] = prev_values[i];
		break;
	case Type::Shade:
		for(size_t i = 0; i < indices.size(); ++i)
			metaroom->_directionalShade[indices[i]]  = prev_values[i];
		break;
	case Type::AmbientShade:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->_ambientShade[indices[i]], &prev_values[i], 4);
		break;
	case Type::Audio:
		for(size_t i = 0; i < indices.size(); ++i)
			memcpy(&metaroom->_audio[indices[i]], &prev_values[i], 4);
		break;
	default:
		break;
	}
}
