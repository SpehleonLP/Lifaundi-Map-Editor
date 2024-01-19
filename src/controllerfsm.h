#ifndef CONTROLLERFSM_H
#define CONTROLLERFSM_H
#include "metaroomselection.h"
#include "enums.hpp"
#include <glm/gtc/type_precision.hpp>
#include <glm/vec2.hpp>
#include <cstdint>

class MainWindow;
class QWheelEvent;

struct SliceEdge
{
	SliceEdge() = default;
	SliceEdge(int edge, glm::ivec2 vertex, float percentage, int uuid = -1) :
		edge(edge),
		vertex(vertex),
		type(0),
		percent(((edge & 0x03) < 2? percentage : 1.f - percentage) * 65535),
		uuid(uuid)
	{
	}

	int        edge;
	glm::ivec2 vertex;
	uint8_t    type;
	uint16_t   percent;
	int        uuid{-1};

	void setPercentage(float p)
	{
		percent = ((edge & 0x03) < 2? p : (1.f - p)) * 65535;
	}
};

class Shaders;

class ControllerFSM
{
public:
	typedef std::array<SliceEdge, 2> SliceRoom;
	typedef std::vector<std::array<SliceEdge, 2>> SliceArray;
	ControllerFSM(MainWindow * window);
	~ControllerFSM();

	bool wheelEvent(QWheelEvent*event);
    void Release(Shaders * shaders);

    void Prepare(Shaders * shaders);
    void Render(Shaders * shaders);
	void clear();
	void ClearTool();

	bool SetTool(Tool);
	bool HaveTool() const { return m_state != State::None; }

	void AddFace();
	void AddFace(glm::ivec2 min, glm::ivec2 max);

	bool OnDoubleClick(glm::ivec2, Bitwise);
	bool OnFinish();

	bool OnLeftDown(glm::vec2, Bitwise, bool alt);
	bool OnLeftUp(glm::vec2, Bitwise, bool alt);

	bool OnMouseMove(glm::vec2, Bitwise);

	static SliceArray SetUpSlices(Metaroom *, uint32_t edge, uint32_t noSlices, float percentage);
	static SliceArray CreateSlice(Metaroom & metaroom, SliceRoom const& slice, bool need_sort);
	static void CreateSlice(SliceArray & result, Metaroom & metaroom, int edge, glm::ivec2 position);

	bool AutoReseat();

	SliceArray m_slice;
	std::vector<uint32_t>  m_ordering;

	int slice_edge{-1};

	void lock(int x, int y)
	{
		glm::i16vec2 value(x, y);

		if(value == xy_filter && m_state == State::ScaleBegin)
			xy_filter += value;
		else
			xy_filter = value;
	}

private:
	MainWindow * m_parent{nullptr};

	glm::vec2 mouse_down_pos{0, 0};
	glm::vec2 mouse_current_pos{0, 0};
	glm::vec2 selection_center{0, 0};
	glm::i16vec2 xy_filter{1, 1};

	State  m_state{State::None};
	bool _sliceDirty{};

	uint32_t m_vao[2]{};
	uint32_t m_vbo[3]{};
    uint32_t m_sliceLength{};
	uint32_t _noSlices{1};
};

#endif // CONTROLLERFSM_H
