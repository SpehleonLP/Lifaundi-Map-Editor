#ifndef CONTROLLERFSM_H
#define CONTROLLERFSM_H
#include "metaroomselection.h"
#include "enums.hpp"
#include <glm/gtc/type_precision.hpp>
#include <glm/vec2.hpp>
#include <cstdint>

class MainWindow;

struct SliceInfo
{
	int        edge;
	glm::ivec2 vertex;
	uint8_t    type;
	int        uuid{-1};
};

class GLViewWidget;

class ControllerFSM
{
public:
	ControllerFSM(MainWindow * window);
	~ControllerFSM();

    void Release(GLViewWidget * gl);

    void Prepare(GLViewWidget * gl);
    void Render(GLViewWidget * gl);
	void clear();
	void ClearTool();

	void SetTool(Tool);
	bool HaveTool() const { return m_state != State::None; }

	bool OnDoubleClick(glm::ivec2, Bitwise);
	bool OnFinish();

	bool OnLeftDown(glm::vec2, Bitwise);
	bool OnLeftUp(glm::vec2, Bitwise);
	void AddFace();

	bool OnMouseMove(glm::vec2, Bitwise);

	void SetUpSlices(std::vector<SliceInfo> & slices, int face, float percentage);
	void CreateSlice(std::vector<SliceInfo> & slices, int edge, glm::ivec2 position);

	std::vector<SliceInfo> m_slice;
	std::vector<int>       m_ordering;

	int slice_face{-1};

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

	uint32_t m_vao[2]{};
	uint32_t m_vbo[3]{};
    uint32_t m_sliceLength{};
};

#endif // CONTROLLERFSM_H
