#include "src/glviewwidget.h"
#include "controllerfsm.h"
#include "commandlist.h"
#include "mainwindow.h"
#include "lf_math.h"
#include "document.h"
#include "Shaders/uniformcolorshader.h"
#include "sort_unique.h"
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

static float GetAngle(glm::ivec2 center, glm::ivec2 point)
{
	glm::vec2 vec = glm::normalize(glm::vec2(point - center));
	return std::atan2(vec.y, vec.x);
}

static bool ArrangeCounterClockwise(glm::ivec2 * verts)
{
	glm::ivec2 center = ((verts[0]) + (verts[1]) + (verts[2]) + (verts[3]))/4;

	std::pair<int, float> order[4]{
		{0, GetAngle(center, verts[0])},
		{1, GetAngle(center, verts[1])},
		{2, GetAngle(center, verts[2])},
		{3, GetAngle(center, verts[3])}
	};

	std::sort(&order[0], &order[4], [](auto const& a, auto const& b)
	{
		return a.second > b.second;
	});

	glm::ivec2 cpy[4]{
		verts[order[0].first],
		verts[order[1].first],
		verts[order[2].first],
		verts[order[3].first]
	};

	memcpy(verts, cpy, sizeof(cpy));

	bool sign = math::cross(verts[1] - verts[0], verts[0] - verts[3]) <= 0;

	return sign == (math::cross(verts[2] - verts[1], verts[1] - verts[0]) <= 0)
		&& sign == (math::cross(verts[3] - verts[2], verts[2] - verts[1]) <= 0)
		&& sign == (math::cross(verts[0] - verts[3], verts[3] - verts[2]) <= 0);
}

bool GetVerticies(glm::ivec2 * dst, std::vector<int> const& selection, Metaroom * metaroom)
{
	std::vector<glm::ivec2> vec(selection.size());

	for(uint32_t i = 0; i < selection.size(); ++i)
		vec[i] = metaroom->GetVertex(selection[i]);

	if(vec.size() > 4)
	{
		std::sort(vec.begin(), vec.end(), [](auto const& a, auto const& b)
		{
			return math::GetZOrder(a) < math::GetZOrder(b);
		});

		Uniq(vec);
	}

	if(vec.size() != 4)
		return false;

	memcpy(dst, &vec[0], sizeof(dst[0])*4);
	return true;
}

template<typename T>
static T GetMode(T const* array, std::vector<int> const& selection)
{
	std::vector<T> vec(selection.size());

	for(size_t i = 0; i < selection.size(); ++i)
	{
		vec[i] = array[selection[i]];
	}

	std::sort(vec.begin(), vec.end());
	return *GetMode(vec);
}

static uint16_t GetAverage(uint16_t const* array, std::vector<int> const& selection)
{
	size_t accumulator=0;

	for(size_t i = 0; i < selection.size(); ++i)
	{
		accumulator += array[selection[i]];
	}

	return accumulator / selection.size();
}

static uint32_t GetGravity(uint32_t const* array, std::vector<int> const& selection)
{
	glm::vec2 accumulator{0,0};

	for(size_t i = 0; i < selection.size(); ++i)
	{
		accumulator += glm::unpackHalf2x16(array[selection[i]]);
	}

	return glm::packHalf2x16(accumulator /(float) selection.size());
}


bool ControllerFSM::SetTool(Tool tool)
{
	if(tool == Tool::Scale)
	{
		if(m_state == State::ScaleBegin)
			m_state = State::WeldBegin;
		else if(m_state == State::WeldBegin)
			m_state = State::ScaleBegin;
	}

	selection_center = m_parent->document->m_metaroom.GetSelectionCenter();
	mouse_down_pos   = m_parent->GetWorldPosition();

	if(m_state == State::Reorder && tool == Tool::Order)
	{
		m_state = State::None;
		m_parent->document->PushCommand(new ReorderCommand(m_parent->document.get(), std::move(m_ordering)));
		return false;
	}

	if(m_state != State::None && tool != Tool::None)
		return false;

	if(tool == Tool::Face)
	{
		auto& metaroom = m_parent->document->m_metaroom;
		auto selection = metaroom.m_selection.GetVertSelection();
		auto faces     = metaroom.m_selection.GetFaceSelection();

		glm::ivec2 verts[4];
		if(!GetVerticies(verts, selection, &metaroom))
		{
			m_parent->SetStatusBarMessage("Add face requires exactly 4 points selected");
			return false;
		}

		if(!ArrangeCounterClockwise(verts))
		{
			m_parent->SetStatusBarMessage("Selected points do not define a convex polygon");
			return false;
		}

		if(!metaroom.CanAddFace(verts))
		{
			m_parent->SetStatusBarMessage("Face addition would make graph non-planar");
			return false;
		}

		Room room;

		room.type		  = GetMode(&metaroom.m_roomType[0], faces);
		room.music_track  = GetMode(&metaroom.m_music[0], faces);
		room.gravity      = GetGravity(&metaroom.m_gravity[0], faces);
		room.drawDistance = GetAverage(&metaroom.m_drawDistance[0], faces);

		memset(&room.wall_types[0], 0, sizeof(room.wall_types));
		memcpy(&room.verts[0], verts, sizeof(verts));

		m_parent->document->PushCommand(new InsertCommand(m_parent->document.get(), {room}));
		return true;
	}

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
		m_parent->SetMouseTracking(true);
		xy_filter = glm::ivec2(1, 1);
		m_parent->SetStatusBarMessage(XY_FILTER);
		break;
	case Tool::Rotate:
		m_state = State::RotateBegin;
		m_parent->SetMouseTracking(true);
		xy_filter = glm::ivec2(1, 1);
		break;
	case Tool::Scale:
		m_state = State::ScaleBegin;
		m_parent->SetMouseTracking(true);
		xy_filter = glm::ivec2(1, 1);
		m_parent->SetStatusBarMessage(XY_FILTER);
		break;
	case Tool::Slice:
		m_state = State::SliceSet;
		m_parent->SetMouseTracking(true);
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
	case Tool::SliceGravity:
		m_state = State::SliceSetGravity;
		m_parent->SetMouseTracking(true);
		xy_filter = glm::ivec2(1, 1);
		break;
	case Tool::PropagateGravity:
		break;
	default:
		break;
	}

	return false;
}

void ControllerFSM::ClearTool()
{
	m_state = State::None;
	m_slice.clear();
	m_ordering.clear();
	m_parent->document->m_metaroom.CancelMove();
	m_parent->SetMouseTracking(false);
	m_parent->need_repaint();
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
	case State::RotateGravity:
	case State::DirectGravity:
	case State::ScaleGravity:
	case State::SliceGravity:
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
		m_parent->document->m_metaroom.ClickSelect(position, flags, alt, m_parent->GetZoom());
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
		if(m_slice.empty() || slice_edge < 0)
			m_state = State::None;
		else
		{
			m_state = State::SliceBegin;
			slice_edge = m_slice.front().edge;
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

	case State::SliceSetGravity:
		if(m_slice.empty() || slice_edge < 0)
			m_state = State::None;
		else
		{
			m_state = State::SliceGravity;
			slice_edge = m_slice.front().edge;
		}
		return true;
	case State::SliceGravity:
	{
		m_state = State::None;

		if(m_slice.empty())
			return false;

		std::vector<int> list;
		std::vector<uint32_t> values;

		list.resize(m_slice.size()/2);
		values.resize(m_slice.size()/2, 0);

		int id = -1;

		for(uint32_t i = 0; i < m_slice.size(); i += 2)
		{
			list[i/2] = (m_slice[i].edge/4);
			if(list[i/2] == slice_edge/4)	id = i;
		}

		assert(id != -1);

		auto const& mta = m_parent->document->m_metaroom;
		int edge = ((m_slice[id].percent <= m_slice[id+1].percent? m_slice[id].edge : m_slice[id+1].edge) + 5);
		id       = edge - m_slice[id].edge;

		glm::vec2 gravity		= mta.GetGravity(list[0]);
		glm::vec2 floor_normal  = math::OrthoNormal(mta.GetEdge(slice_edge/4, edge));

		for(uint32_t i = 0; i < list.size(); ++i)
		{
			glm::vec2 normal = math::OrthoNormal(mta.GetEdge(list[i], m_slice[i*2].edge+id));

			auto mat = math::GetRotation(floor_normal, normal);
			glm::vec2 grav = mat * glm::vec3(gravity, 1);
			values[i] = glm::packHalf2x16(grav);
		}

		m_slice.clear();
		m_parent->document->PushCommand(new DifferentialSetCommmand(m_parent->document.get(), std::move(list), std::move(values), DifferentialSetCommmand::Type::Gravity));
		return true;
	}
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
		if(m_slice.empty() || slice_edge < 0)
			m_state = State::None;
		else
		{
			m_state = State::SliceBegin;
			slice_edge = m_slice.front().edge;
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

	case State::SliceSetGravity:
		if(m_slice.empty() || slice_edge < 0)
			m_state = State::None;
		else
		{
			m_state = State::SliceGravity;
			slice_edge = m_slice.front().edge;
		}
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
		return true;
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
	int original_edge = edge;

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
		{
			if(original_edge == edge)
				slices[0].vertex = slices.back().vertex;
			else if(original_edge == Metaroom::GetOppositeEdge(edge))
				slices[1].vertex = slices.back().vertex;

			return;
		}

		slices.push_back({edge, position, mid});

		edge     = Metaroom::GetOppositeEdge(edge);
		position = metaroom.GetPointOnEdge(edge, mid);

		slices.push_back({edge, position, mid});
	}
}

void ControllerFSM::SetUpSlices(std::vector<SliceInfo> & slices, int edge, float percentage)
{
	slices.resize(2);
	slices[0].edge    = edge;
	slices[0].vertex = m_parent->document->m_metaroom.GetPointOnEdge(m_slice[0].edge, percentage);
	slices[0].setPercentage(percentage);
	slices[1].edge    = Metaroom::GetOppositeEdge(edge);
	slices[1].vertex = m_parent->document->m_metaroom.GetPointOnEdge(m_slice[1].edge, percentage);
	slices[1].setPercentage(percentage);
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

	if(m_state == State::SliceSet || m_state == State::SliceSetGravity)
	{
		int edge =  m_parent->document->m_metaroom.GetSliceEdge(mouse_current_pos);

		if(edge == slice_edge)
			return;

		if((slice_edge = edge) != -1)
			SetUpSlices(m_slice, slice_edge, .5);
	}
	else if(m_state == State::SliceBegin || m_state == State::SliceGravity)
	{
		assert(slice_edge >= 0);
		SetUpSlices(m_slice, slice_edge, m_parent->document->m_metaroom.ProjectOntoEdge(slice_edge, mouse_current_pos));
		m_parent->document->m_metaroom.m_selection.MarkFace(slice_edge/4);

		CreateSlice(m_slice, m_slice[0].edge, m_slice[0].vertex);
		CreateSlice(m_slice, m_slice[1].edge, m_slice[1].vertex);
		m_parent->document->m_metaroom.m_selection.ClearMarks();

		if(m_state != State::SliceGravity)
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
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(m_slice[0])*m_slice.capacity(), &m_slice[0], GL_DYNAMIC_DRAW);
	}


	if(m_state == State::BoxSelectBegin
	|| m_state == State::SliceGravity)
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

		if(m_state < State::SliceSetGravity)
		{
			gl->glLineWidth(1);
			UniformColorShader::Shader.Bind(gl, 1.f, 0.f, 1.f, 1.f);
		}
		else
		{
			gl->glLineWidth(2);
			UniformColorShader::Shader.Bind(gl, 0.f, 1.f, 1.f, 1.f);
		}
        gl->glDrawArrays(GL_LINES, 0, m_slice.size());
	}
}
