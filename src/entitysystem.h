#ifndef ENTITYSYSTEM_H
#define ENTITYSYSTEM_H
#include <cstdint>
#include <functional>

class QOpenGLFunctions_3_2_Core;

class EntitySystem
{
public:
	typedef void ReallocationFunction(size_t);

	EntitySystem(std::function<ReallocationFunction> && cb);
	~EntitySystem();

	void clear();

	bool empty() const { return _totalSize == 0; }
	size_t size() const { return _totalSize; }
	size_t used() const { return _usedList.size(); }

	bool IsInUse(uint32_t id) const;
	void ReleaseGL(QOpenGLFunctions_3_2_Core * gl);

	uint32_t GetEntity();
	void	 GetEntity(uint32_t request);
	void     ReleaseEntity(uint32_t);

	uint32_t GetVBO(QOpenGLFunctions_3_2_Core * gl);
	std::vector<uint32_t> const& GetList() const;

	struct Range
	{
		auto begin() const { return _begin; }
		auto end() const { return _end; }
		auto size() const { return _end - _begin; }

		std::vector<uint32_t>::const_iterator _begin, _end;
	};

	operator Range() const
	{
		auto & list = GetList();
		return Range{._begin=list.begin(), ._end=list.end()};
	}

private:
	std::function<ReallocationFunction> _onRealloc;
	std::vector<uint32_t> _freeList;
	mutable std::vector<uint32_t> _usedList;

	size_t			 _totalSize{};
	uint32_t		 _usedVBO{};
	mutable bool	 _dirty{};
	mutable bool	 _vboDirty{};
	QOpenGLFunctions_3_2_Core   * _gl{};
};

#endif // ENTITYSYSTEM_H
