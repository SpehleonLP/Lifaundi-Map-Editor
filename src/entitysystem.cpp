#include "entitysystem.h"
#include <QOpenGLFunctions_3_2_Core>
#include <loguru.hpp>

EntitySystem::EntitySystem(std::function<ReallocationFunction> && cb) :
	_onRealloc(cb)
{
}

EntitySystem::~EntitySystem()
{
	LOG_IF_F(WARNING, _freeList.size() + _usedList.size() != _totalSize, "memory leak in Entity System, %d items unaccounted for at destruction", (int)(_totalSize - _freeList.size()));

	if(_usedVBO)
	{
		_gl->glDeleteBuffers(1, &_usedVBO);
	}

}

void EntitySystem::clear()
{
	_freeList.insert(_freeList.begin(), _usedList.begin(), _usedList.end());
	_usedList.clear();
	std::sort(_freeList.rbegin(), _freeList.rend());
}

void EntitySystem::ReleaseGL(QOpenGLFunctions_3_2_Core * gl)
{
	if(_usedVBO)
	{
		gl->glDeleteBuffers(1, &_usedVBO);
		_usedVBO = 0;
		_gl = nullptr;
	}
}

bool EntitySystem::IsInUse(uint32_t item) const
{
	return std::find(_usedList.begin(), _usedList.end(), item) != _usedList.end();
}

uint32_t EntitySystem::GetEntity()
{
	if(_freeList.empty())
	{
		int64_t pSize = _totalSize;
		_totalSize = std::max<size_t>(16, _totalSize*2);

		LOG_IF_F(ERROR, _totalSize > 65535, "number of rooms allocated has exceeded maximum of: %u", USHRT_MAX);

		_onRealloc(_totalSize);

		_freeList.reserve(pSize);
		for(int64_t i = _totalSize-1; i >= pSize; --i)
		{
			_freeList.push_back(i);
		}
	}

	auto request = _freeList.back();
	_freeList.pop_back();

	_vboDirty = true;
	_dirty |= _usedList.size() && _usedList.back() > request;

	_usedList.push_back(request);
	return request;
}


void EntitySystem::GetEntity(uint32_t request)
{
	auto item = std::find(_freeList.begin(), _freeList.end(), request);
	LOG_IF_F(ERROR, item == _freeList.end(), "bookkeeping error in Entity System, %d should be unused but is not in free list", request);
	_freeList.erase(item);

	_vboDirty = true;
	_dirty |= _usedList.size() && _usedList.back() > request;

	_usedList.push_back(request);
}

void     EntitySystem::ReleaseEntity(uint32_t request)
{
	_vboDirty = true;

	if(_usedList.size() && _usedList.back() == request)
	{
		_usedList.pop_back();
		_freeList.push_back(request);
		return;
	}

	auto item = std::find(_usedList.begin(), _usedList.end(), request);
	LOG_IF_F(ERROR, item == _usedList.end(), "bookkeeping error in Entity System, %d should be used but is not in used list", request);

	_usedList.erase(item);
	_freeList.push_back(request);
	_dirty = true;
}

uint32_t EntitySystem::GetVBO(QOpenGLFunctions_3_2_Core * gl)
{
	if(!_vboDirty) return _usedVBO;

	if(_usedVBO == 0)
		gl->glGenBuffers(1, &_usedVBO);

	gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _usedVBO);
	gl->glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t)*_usedList.size(), _usedList.data(), GL_DYNAMIC_DRAW);
	gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	_vboDirty = false;

	return _usedVBO;
}

std::vector<uint32_t> const& EntitySystem::GetList() const
{
	if(!_dirty)
		return _usedList;

	_dirty = false;
	std::sort(_usedList.begin(), _usedList.end());
	return _usedList;
}

std::vector<int> EntitySystem::GetInversse() const
{
	std::vector<int> r(_totalSize, -1);

	uint32_t counter = 0;

	for(auto item : _usedList)
	{
		r[item] = counter++;
	}

	return r;

}
