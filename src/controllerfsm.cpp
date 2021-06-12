#include "src/glviewwidget.h"
#include "controllerfsm.h"
#include "commandlist.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lf_math.h"
#include "document.h"
#include "Shaders/uniformcolorshader.h"
#include <iostream>
#include <glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358
#endif

#define XY_FILTER "use 'x' or 'y' to affect only that axis"

ControllerFSM::ControllerFSM(MainWindow * window) :
    m_parent(window)
{
    UniformColorShader::Shader.AddRef();
}

ControllerFSM::~ControllerFSM()
{
}

void ControllerFSM::Release(GLViewWidget * gl)
{
	gl->glAssert();
     UniformColorShader::Shader.Release(gl);
     gl->glDeleteVertexArrays(2, m_vao);
     gl->glDeleteBuffers(3, m_vbo);
}

bool ControllerFSM::OnDoubleClick(glm::ivec2, Bitwise)
{
	return false;
}


void ControllerFSM::SetTool(Tool tool)
{
	if(tool == Tool::Scale)
	{
		if(m_state == State::ScaleBegin)
			m_state = State::WeldBegin;
		else if(m_state == State::WeldBegin)
			m_state = State::ScaleBegin;
	}

	selection_center = m_parent->document->m_metaroom.GetSelectionCenter();
	mouse_down_pos   = m_parent->ui->viewWidget->GetWorldPosition();

	if(m_state == State::Reorder && tool == Tool::Order)
	{
		m_state = State::None;
		m_parent->document->PushCommand(new ReorderCommand(m_parent->document.get(), std::move(m_ordering)));
		return;
	}

	if(m_state != State::None && tool != Tool::None)
		return;

	switch(tool)
	{
	case Tool::None:
		ClearTool();
		break;
	case Tool::Create:
		m_state = State::CreateSet;
		break;
	case Tool::Duplicate:
		break;
	case Tool::Select:
		m_state = State::BoxSelectSet;
		break;
	case Tool::Translate:
		m_state = State::TranslateSet;
		m_parent->ui->viewWidget->setMouseTracking(true);
		xy_filter = glm::ivec2(1, 1);
		m_parent->SetStatusBarMessage(XY_FILTER);
		break;
	case Tool::Rotate:
		m_state = State::RotateBegin;
		m_parent->ui->viewWidget->setMouseTracking(true);
		xy_filter = glm::ivec2(1, 1);
		break;
	case Tool::Scale:
		m_state = State::ScaleBegin;
		m_parent->ui->viewWidget->setMouseTracking(true);
		xy_filter = glm::ivec2(1, 1);
		m_parent->SetStatusBarMessage(XY_FILTER);
		break;
	case Tool::Slice:
		m_state = State::SliceSet;
		m_parent->ui->viewWidget->setMouseTracking(true);
		xy_filter = glm::ivec2(1, 1);
		break;
	case Tool::Extrude:
//		m_state = State::ExtrudeSet;
//		m_parent->ui->viewWidget->setMouseTracking(true);
		break;
	case Tool::Order:
		m_state = State::Reorder;
		m_parent->document->m_metaroom.m_selection.clear();
		break;
	case Tool::Finish:
		m_parent->SetStatusBarMessage();
		OnFinish();
		break;
	default:
		break;
	}
}

void ControllerFSM::ClearTool()
{
	m_state = State::None;
	m_slice.clear();
	m_ordering.clear();
	m_parent->document->m_metaroom.CancelMove();
	m_parent->ui->viewWidget->setMouseTracking(false);
	m_parent->ui->viewWidget->need_repaint();
}

bool ControllerFSM::OnLeftDown(glm::vec2 position, Bitwise , bool)
{
	mouse_current_pos = position;

	switch(m_state)
	{
	case State::CreateBegin:
	case State::BoxSelectBegin:
	case State::TranslateBegin:
	case State::RotateBegin:
	case State::ScaleBegin:
	case State::SliceBegin:
	case State::ExtrudeBegin:
	case State::WeldBegin:
		return false;

	default:
		break;
	}

	mouse_down_pos    = position;

	if(m_parent->document == nullptr)
		return false;

	switch(m_state)
	{
	case State::None:
	{
		int face = m_parent->document->m_metaroom.m_tree.GetFace(position);
//get best match for a face

		if(face == -1)
			m_state = State::MouseDownOnNothing;
		else if(m_parent->document->m_metaroom.m_selection.IsFaceSelected(face))
			m_state = State::MouseDownOnSelected;
		else
			m_state = State::MouseDownOnUnselected;
	}  return true;
	case State::MouseDownOnSelected:
	case State::MouseDownOnUnselected:
	case State::MouseDownOnNothing:		throw std::runtime_error("Bad Tool");
//tools set
	case State::CreateSet:
		m_state = State::CreateBegin;
		return true;
	case State::BoxSelectSet:
		m_state = State::BoxSelectBegin;
		return true;
	case State::TranslateSet:
		m_state = State::TranslateBegin;
		return false;
	case State::RotateSet:
		m_state = State::ScaleBegin;
		return false;
	case State::ScaleSet:
		m_state = State::ScaleBegin;
		return false;
//dealt with in mouse up...
	case State::SliceSet:
		return false;
	case State::ExtrudeSet:
//
		return false;

//these end on mouse up
	case State::CreateBegin:
	case State::BoxSelectBegin:
	case State::TranslateBegin:
	case State::RotateBegin:
	case State::ScaleBegin:
	case State::SliceBegin:
	case State::ExtrudeBegin:
	case State::WeldBegin:
		return false;
	default:
		return false;
	}

	return false;
}

bool ControllerFSM::OnLeftUp(glm::vec2 position, Bitwise flags, bool alt)
{
	mouse_current_pos = position;

	switch(m_state)
	{
	case State::None: break;
	case State::MouseDownOnSelected:
	{
		int face = m_parent->document->m_metaroom.m_tree.GetFace(mouse_down_pos);

		if(face >= 0)
		{
			m_parent->document->m_metaroom.m_selection.select_face(face, flags);
			m_parent->document->m_metaroom.update_selections();
		}

		m_state = State::None;
	}   return true;
	case State::MouseDownOnUnselected:
	case State::MouseDownOnNothing:
	{
		m_parent->document->m_metaroom.ClickSelect(position, flags, alt);
		m_state = State::None;
	}	return true;

//tools set, these all take action on mouse down.
	case State::CreateSet:
	case State::BoxSelectSet:
	case State::TranslateSet:
	case State::RotateSet:
	case State::ScaleSet:
		break;
//slice turns from face select to % select on mouse up...
	case State::SliceSet:
		if(m_slice.empty() || slice_face < 0)
			m_state = State::None;
		else
		{
			m_state = State::SliceBegin;
			slice_face = m_slice.front().edge;
		}
		return true;

	case State::CreateBegin:
	{
		AddFace();
		m_state = State::None;
	}   return true;
	case State::BoxSelectBegin:
		m_parent->document->m_metaroom.BoxSelect(
			glm::min(mouse_down_pos, mouse_current_pos),
			glm::max(mouse_down_pos, mouse_current_pos),
			flags);

		m_state = State::None;
		return true;
	case State::TranslateBegin:
	case State::RotateBegin:
	case State::ScaleBegin:
		m_parent->document->PushCommand(new TransformCommand(m_parent->document.get()));
		m_state = State::None;
		return true;
	case State::SliceBegin:
		m_state = State::None;
		m_parent->document->PushCommand(new SliceCommand(m_parent->document.get(), std::move(m_slice)));
		return true;
	case State::ExtrudeBegin:
		return false;
	case State::WeldBegin:
		m_parent->document->PushCommand(new TransformCommand(m_parent->document.get()));
		m_state = State::None;
		return true;
	case State::Reorder:
	{
		int face = m_parent->document->m_metaroom.m_tree.GetFace(mouse_down_pos);

		if(face < 0) return false;

		m_parent->document->m_metaroom.m_selection.select_face(face, Bitwise::XOR);

		if(m_parent->document->m_metaroom.m_selection.IsSelected(face))
		{
			m_ordering.push_back(face);
		}
		else
		{
			for(size_t i = 0; i < m_ordering.size(); ++i)
			{
				if(m_ordering[i] == face)
				{
					m_ordering.erase(m_ordering.begin()+i);
					break;
				}
			}
		}
	}	return true;
	default:
		return false;
	}

	return false;
}

bool ControllerFSM::OnFinish()
{
	switch(m_state)
	{
	case State::None:
		return true;
	case State::MouseDownOnSelected:
	case State::MouseDownOnUnselected:
	case State::MouseDownOnNothing:

	case State::CreateSet:
	case State::BoxSelectSet:
	case State::TranslateSet:
	case State::RotateSet:
	case State::ScaleSet:
		ClearTool();
		return true;

	case State::SliceSet:
		if(m_slice.empty() || slice_face < 0)
			m_state = State::None;
		else
		{
			m_state = State::SliceBegin;
			slice_face = m_slice.front().edge;
		}
		return true;

	case State::ExtrudeSet:
		m_state = State::None;
		return true;

	case State::CreateBegin:
	{
		AddFace();
	}   return true;
	case State::BoxSelectBegin:
		m_state = State::None;
		return true;
	case State::TranslateBegin:
	case State::RotateBegin:
	case State::ScaleBegin:
		m_parent->document->PushCommand(new TransformCommand(m_parent->document.get()));
		m_state = State::None;
		return true;
	case State::SliceBegin:
		m_state = State::None;
		m_parent->document->PushCommand(new SliceCommand(m_parent->document.get(), std::move(m_slice)));
		return true;

	case State::ExtrudeBegin:
		return true;
	case State::WeldBegin:
		m_parent->document->PushCommand(new TransformCommand(m_parent->document.get()));
		m_state = State::None;
		return true;

	case State::Reorder:
		m_state = State::None;
		m_parent->document->PushCommand(new ReorderCommand(m_parent->document.get(), std::move(m_ordering)));
		return true;
	default:
		break;
	}

	return false;
}

bool ControllerFSM::OnMouseMove(glm::vec2 p, Bitwise flags)
{
	mouse_current_pos = p;

	if(m_state == State::MouseDownOnSelected || m_state == State::MouseDownOnUnselected)
	{
		if(math::length2(p - mouse_down_pos)*m_parent->GetZoom() < 9)
			return false;
	}

	switch(m_state)
	{
	case State::None:
		return true;
	case State::MouseDownOnSelected:
		xy_filter = glm::ivec2(1, 1);
		m_state = State::TranslateBegin;
		return true;
	case State::MouseDownOnUnselected:
	case State::MouseDownOnNothing:
		m_state = State::BoxSelectBegin;
		return true;

//tools set, these all take action on mouse down.
	case State::CreateSet:
	case State::BoxSelectSet:
	case State::TranslateSet:
		mouse_down_pos    = p;
		m_state = State::TranslateBegin;
		break;
	case State::RotateSet:
	case State::ScaleSet:
		break;
	case State::SliceSet:
		return true;

	case State::CreateBegin:
	case State::BoxSelectBegin:
		return true;
	case State::TranslateBegin:
	{
		glm::ivec2 distance = mouse_current_pos - mouse_down_pos;

		if(flags == Bitwise::XOR)
		{
			glm::ivec2 sign = glm::sign(distance);
			distance = glm::abs(distance);
			distance.x -= distance.x % 5;
			distance.y -= distance.y % 5;
			distance *= sign;
		}

		m_parent->document->m_metaroom.Translate(distance * glm::ivec2(xy_filter), m_parent->document->GetDimensions() / 2.f);
		return true;
	}
	case State::RotateBegin:
	{
		glm::vec2 distance = mouse_current_pos - selection_center;

		if(flags == Bitwise::XOR)
		{
			float angle   = std::atan2(distance.y, distance.x);
			int degrees = (180 * angle / M_PI) + .5;
			degrees = degrees % 5;
			angle   = degrees * M_PI / 180;
			distance = glm::vec2(cos(angle), sin(angle));
		}
		else
		{
			distance = glm::normalize(distance);
		}

		m_parent->document->m_metaroom.Rotate(selection_center, distance);
		return true;
	}
	case State::ScaleBegin:
	{
		auto  dirn    = mouse_down_pos - selection_center;
		float length2 = glm::length(dirn);
		float dot1    = glm::dot(mouse_current_pos - selection_center, dirn) / (length2 * length2);

		float scale   = dot1;

		//get direction


		if(flags == Bitwise::XOR)
		{
			int s = scale * 100 + 2.5;
			s -= s % 5;
			s = std::max(5, s);
			scale = s / 100.f;
		}

		glm::vec2 scaling = glm::vec2(scale);

		if(xy_filter.x == 0)
		{
			if(xy_filter.y == 1) scaling.x = 1;
			else if(xy_filter.y == 2) scaling.x = 0;
		}
		else if(xy_filter.y == 0)
		{
			if(xy_filter.x == 1) scaling.y = 1;
			else if(xy_filter.x == 2) scaling.y = 0;
		}

		m_parent->document->m_metaroom.Scale(selection_center, scaling);
	} return true;


	case State::SliceBegin:
	case State::ExtrudeBegin:
		return true;
	case State::WeldBegin:
	{
		m_parent->document->m_metaroom.Scale(selection_center, glm::vec2(0, 0));
	}

	default:
		return false;
	}

	return false;
}

void ControllerFSM::CreateSlice(std::vector<SliceInfo> & slices, int edge, glm::ivec2 position)
{
	auto & selection = m_parent->document->m_metaroom.m_selection;
	bool selected = selection.IsFaceSelected(edge / 4);

	auto & metaroom = m_parent->document->m_metaroom;

	float mid;

	while(metaroom.m_tree.GetSliceFace(
		metaroom.GetVertex(edge),
		metaroom.GetVertex(Metaroom::NextInEdge(edge)),
		position,
		edge,
		mid))
	{
		if(selected && !selection.IsFaceSelected(edge / 4))
			return;

		if(selection.MarkFace(edge/4) == false)
			return;

		slices.push_back({edge, position, 0});

		edge     = Metaroom::GetOppositeEdge(edge);
		position = metaroom.GetPointOnEdge(edge, mid);

		slices.push_back({edge, position, 0});
	}
}

void ControllerFSM::SetUpSlices(std::vector<SliceInfo> & slices, int face, float percentage)
{
	slices.resize(2);
	slices[0].edge    = face;
	slices[0].vertex = m_parent->document->m_metaroom.GetPointOnEdge(m_slice[0].edge, percentage);
	slices[1].edge    = Metaroom::GetOppositeEdge(face);
	slices[1].vertex = m_parent->document->m_metaroom.GetPointOnEdge(m_slice[1].edge, percentage);
}

void ControllerFSM::AddFace()
{
	auto min = glm::min(mouse_down_pos, mouse_current_pos);
	auto max = glm::max(mouse_down_pos, mouse_current_pos);

	auto half_dimensions = m_parent->document->GetDimensions() / 2.f;

	min = glm::max(-half_dimensions, glm::min(min, half_dimensions));
	max = glm::max(-half_dimensions, glm::min(max, half_dimensions));

	m_state = State::None;

	if(math::length2(max - min) > 16
	&& !m_parent->document->m_metaroom.m_tree.DoesOverlap(min, max))
	{
		Room room;
		room.verts[0] = max;
		room.verts[1] = glm::ivec2(max.x, min.y);
		room.verts[2] = min;
		room.verts[3] = glm::ivec2(min.x, max.y);

		room.gravity     = glm::packHalf2x16(glm::vec2(0, 9.81));
		room.music_track = -1;
		room.type        = 0;
		memset(room.wall_types, 0, 4);

		m_parent->document->PushCommand(new InsertCommand(m_parent->document.get(), {room}));
	}
}

void ControllerFSM::Prepare(GLViewWidget *gl)
{
	const uint8_t tool_indices[] =
	{
//h line1
		0, 8, 8, 1,
//v line 1
		2, 8, 8, 3,
//h line 2
		4, 10, 10, 5,
//h line 3
		6, 10, 10, 7,
//box polygon
		8, 9, 10, 10, 11, 8,
//box lines
		8, 9, 9, 10, 10, 11, 11, 8
	};

	if(m_state == State::SliceSet)
	{
		int face =  m_parent->document->m_metaroom.GetSliceEdge(mouse_current_pos);

		if(face == slice_face)
			return;

		if((slice_face = face) != -1)
			SetUpSlices(m_slice, slice_face, .5);
	}
	else if(m_state == State::SliceBegin)
	{
		assert(slice_face >= 0);
		SetUpSlices(m_slice, slice_face, m_parent->document->m_metaroom.ProjectOntoEdge(slice_face, mouse_current_pos));
		m_parent->document->m_metaroom.m_selection.MarkFace(slice_face/4);

		CreateSlice(m_slice, m_slice[0].edge, m_slice[0].vertex);
		CreateSlice(m_slice, m_slice[1].edge, m_slice[1].vertex);
		m_parent->document->m_metaroom.m_selection.ClearMarks();

		std::sort(m_slice.begin(), m_slice.end(),
			[](SliceInfo const& a, SliceInfo const& b)
			{
				return a.edge < b.edge;
			});
	}

	if(m_vao[0] == 0)
	{
        gl->glGenVertexArrays(2, m_vao);
        gl->glGenBuffers(3, m_vbo);

        gl->glBindVertexArray(m_vao[0]);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::ivec2)*12, nullptr, GL_DYNAMIC_DRAW);

        gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(glm::ivec2), 0);
        gl->glEnableVertexAttribArray(0);

        gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[1]);
        gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tool_indices), tool_indices, GL_STATIC_DRAW);

        gl->glBindVertexArray(m_vao[1]);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo[2]);

        gl->glVertexAttribIPointer(0, 2, GL_INT, sizeof(SliceInfo), (void*) offsetof(SliceInfo, vertex));
        gl->glEnableVertexAttribArray(0);

	}


	if(m_slice.size())
	{
        gl->glBindVertexArray(0);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo[2]);

		if(m_sliceLength < m_slice.capacity())
		{
			m_sliceLength = m_slice.capacity();
            gl->glBufferData(GL_ARRAY_BUFFER, sizeof(m_slice[0])*m_slice.capacity(), &m_slice[0], GL_DYNAMIC_DRAW);
		}
		else
		{
            gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_slice[0])*m_slice.size(), &m_slice[0]);
		}
	}


	if(m_state == State::BoxSelectBegin
	|| m_state == State::CreateBegin)
	{
		glm::ivec2 min(glm::vec2(m_parent->WidgetSize()) / (2 * m_parent->GetZoom()));
		glm::ivec2 max = glm::ivec2(m_parent->document->GetDimensions()) - min;

		glm::ivec2 verts[12]
		{
			{mouse_down_pos.x,    min.y},
			{mouse_down_pos.x,    max.y},

			{min.x, mouse_down_pos.y},
			{max.x, mouse_down_pos.y},

			{mouse_current_pos.x,    min.y},
			{mouse_current_pos.x,    max.y},

			{min.x, mouse_current_pos.y},
			{max.x, mouse_current_pos.y},

			{mouse_down_pos.x,    mouse_down_pos.y},
			{mouse_current_pos.x, mouse_down_pos.y},
			{mouse_current_pos.x, mouse_current_pos.y},
			{mouse_down_pos.x,    mouse_current_pos.y},
		};

        gl->glBindVertexArray(0);
        gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
        gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::ivec2)*12, &verts[0]);
	}
}

void ControllerFSM::Render(GLViewWidget * gl)
{
    Prepare(gl);

	if(m_state == State::BoxSelectBegin)
	{
        gl->glBindVertexArray(m_vao[0]);

        gl->glLineWidth(2);
        UniformColorShader::Shader.Bind(gl, 0, 0, 1.f, .5f);
        gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*) (16L));
        UniformColorShader::Shader.Bind(gl, 0, 0.f, 1.f, .5f);
        gl->glDrawElements(GL_LINES, 16, GL_UNSIGNED_BYTE, (void*) (0));
	}

	if(m_state == State::CreateBegin)
	{
        gl->glBindVertexArray(m_vao[0]);

        gl->glLineWidth(2);
        UniformColorShader::Shader.Bind(gl, 0, 1.f, 1.f, 1.f);
        gl->glDrawElements(GL_LINES, 8, GL_UNSIGNED_BYTE, (void*) (22));
	}

	if(m_slice.size())
	{
        gl->glBindVertexArray(m_vao[1]);

        gl->glLineWidth(1);
        UniformColorShader::Shader.Bind(gl, 1.f, 0.f, 1.f, 1.f);
        gl->glDrawArrays(GL_LINES, 0, m_slice.size());
	}
}
