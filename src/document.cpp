#include "document.h"
#include "commandlist.h"
#include "backgroundimage.h"
#include "ui_mainwindow.h"
#include "clipboard.h"
#include "lf_math.h"
#include "src/walltype_db.h"
#include "Shaders/blitshader.h"
#include <fstream>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cassert>

Document::Document(MainWindow * window) :
	m_window(window),
	m_metaroom(this)
{
	BlitShader::Shader.AddRef();

}


Document::~Document()
{
}

void Document::Release(GLViewWidget * gl)
{
	gl->glAssert();
    m_metaroom.Release(gl);

    if(m_background)
      m_background->Release(gl);

    BlitShader::Shader.Release(gl);
	gl->glAssert();
}

void Document::Delete()
{
	std::vector<int> selection = m_metaroom.m_selection.GetFaceSelection();

	if(selection.size())
		PushCommand(new DeleteCommand(this, std::move(selection)));

	m_window->ui->viewWidget->need_repaint();
}

void Document::Copy()
{
	auto vec = Clipboard::Extract(&m_metaroom);

	if(vec.size())
	{
		Clipboard::Center(vec);
		Clipboard::SetClipBoard(std::move(vec));
	}
}

void Document::Paste(glm::vec2 center)
{
	auto vec = Clipboard::GetClipBoard();

	if(vec.size())
	{
		Clipboard::Translate(vec, center);
		PushCommand(new InsertCommand(this, std::move(vec)));
	}
}

void Document::OnSelectionChanged(GLViewWidget *gl)
{
	gravity      = m_metaroom.GetGravity();
	track        = m_metaroom.GetMusicTrack();
	room_type    = m_metaroom.GetRoomType();
	wall_type    = m_metaroom.GetWallType();
	permeability = m_metaroom.GetPermeability();

    m_metaroom.m_selection.Prepare(gl);
}

uint32_t Document::noFacesSelected() const
{
	return m_metaroom.m_selection.NoSelectedFaces();
}

uint32_t Document::noEdgesSelected() const
{
	return m_metaroom.m_selection.NoSelectedEdges();
}

void Document::SelectAll()
{
	m_metaroom.m_selection.ToggleSelectAll(m_metaroom.faces);
}

void Document::RenderBackground(GLViewWidget * gl)
{
	if(m_background == nullptr)
		return;

    m_background->Render(gl);
}

glm::vec2 Document::GetScreenCenter() const
{
	return m_window->GetScroll() * GetDimensions() - GetDimensions() / 2.f;
}

glm::vec2 Document::GetDimensions() const
{
	if(m_background == nullptr)
		return glm::vec2(1, 1);

	return glm::vec2(m_background->dimensions());
}

bool  Document::isCreaturesBackground() const
{
	return m_background != nullptr && m_background->GetType() != BackgroundImage::Type::Lifaundi;
}

int Document::width() const
{
	return m_background == nullptr? 0 : m_background->dimensions().x;
}

int Document::height() const
{
	return m_background == nullptr? 0 : m_background->dimensions().y;
}

void Document::PushCommand(CommandInterface * it)
{
	m_window->SetAsterisk(true);

	m_history.resize(m_command);
	m_history.push_back(std::unique_ptr<CommandInterface>(it));
	m_command = m_history.size();

	m_window->ui->editUndo->setEnabled(true);
	m_window->ui->editRedo->setEnabled(false);
}

bool Document::Undo()
{
	m_window->SetAsterisk(true);

	assert(m_command > 0 && m_history.size());
	m_history[--m_command]->RollBack();
	m_window->OnSelectionChanged();
	return m_command != 0;
}

bool Document::Redo()
{
	m_window->SetAsterisk(true);

	assert(m_command < m_history.size());
	m_history[m_command++]->RollForward();
	m_window->OnSelectionChanged();
	return m_command != m_history.size();
}


void Document::SetRoomMusic(int value)
{
	if(value == track)
		return;
#if HAVE_FMOD
	value          = m_metaroom.GetMusicId(FMODManager::Get()->GetTrackGUIDs()[value]);
#endif
	PushSettingCommand(value, SettingCommand::Type::Track);
}

void Document::SetDoorType(int value)
{
	if(value == wall_type)
		return;

	PushSettingCommand(value, SettingCommand::Type::DoorType);
}

void Document::SetRoomType(int value)
{
	if(value == wall_type)
		return;

	PushSettingCommand(value, SettingCommand::Type::RoomType);
}

void Document::SetRoomGravity(float x, float y)
{
	uint32_t value = glm::packHalf2x16(glm::vec2(x, y));
	if(value == gravity)
		return;

	PushSettingCommand(value, SettingCommand::Type::Gravity);
}

void Document::SetPermeability(int value)
{
	if(value == permeability || value < 0)
		return;

	auto doors = m_metaroom.GetSelectedDoors();

	if(doors.empty())
		return;

	if(!m_window->SetAsterisk(true))
	{
		if(m_command == m_history.size() && m_history.size())
		{
			auto command = dynamic_cast<PermeabilityCommand*>(m_history.back().get());

			if(command != nullptr
			&& command->IsSelection(doors))
			{
				command->SetValue(value);
				return;
			}
		}
	}

	PushCommand(new PermeabilityCommand(this, std::move(doors), value));
}


void Document::PushSettingCommand(uint32_t value, SettingCommand::Type type)
{
	std::vector<int> selection;

	switch(type)
	{
	case SettingCommand::Type::Gravity:
	case SettingCommand::Type::Track:
	case SettingCommand::Type::RoomType:
		selection = m_metaroom.m_selection.GetFaceSelection();
		break;
	case SettingCommand::Type::DoorType:
		selection = m_metaroom.m_selection.GetEdgeSelection();
		break;
	default:
		selection = m_metaroom.m_selection.GetSelection();
		break;
	}

	if(selection.empty())
		return;

	if(!m_window->SetAsterisk(true))
	{
		if(m_command == m_history.size() && m_history.size())
		{
			auto command = dynamic_cast<SettingCommand*>(m_history.back().get());

			if(command != nullptr
			&& command->IsType(type)
			&& command->IsSelection(selection))
			{
				command->SetValue(value);
				return;
			}
		}
	}

	m_history.resize(m_command);
	m_history.push_back(std::unique_ptr<CommandInterface>(new SettingCommand(this, std::move(selection), value, type)));
	m_command = m_history.size();

	m_window->ui->editUndo->setEnabled(true);
	m_window->ui->editRedo->setEnabled(false);
}

void Document::toggleVisibility(int i, int state)
{
	m_visibility &= ~(1 << i);
	m_visibility |= (state != 0) << i;

	m_window->need_repaint();
}

bool Document::SaveFile(QFileInfo const& Path)
{
	m_path = Path;
	m_title = Path.fileName();

	m_window->m_asterisk = true;
	m_window->SetAsterisk(false);

	std::string str = Path.filePath().toStdString();
	std::ofstream file(str.c_str(), std::ios_base::binary);

	if(!file.is_open())
		throw std::system_error(errno, std::system_category(), str);

	file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

	m_metaroom.Write(m_window, file);

	file.close();
	return true;
#if 0

	uint32_t version = 1;

	uint16_t w = width();
	uint16_t h = height();
	uint32_t image_offsets[32];
	memset(image_offsets, 0, 32*sizeof(uint32_t));

	fwrite(&version, sizeof(uint32_t), 1, fp);
	fwrite(&w , sizeof(uint16_t), 1, fp);
	fwrite(&h, sizeof(uint16_t), 1, fp);

	size_t pos = ftell(fp);
	fseek(fp, 4*32, SEEK_CUR);

	if(save_rooms)
	{
		image_offsets[1] = ftell(fp);
		m_metaroom.Write(fp, w, h);
	}

	if(save_background && m_background)
	{
		image_offsets[0] = ftell(fp);
		m_background->WriteFile(fp);
	}

	fseek(fp, pos, SEEK_SET);
	fwrite(image_offsets, sizeof(uint32_t), 32, fp);
	fclose(fp);

	return true;
#endif
}

bool Document::LoadFile(GLViewWidget * gl, QFileInfo const& Path, bool load_rooms, bool load_background)
{
	std::string str = Path.filePath().toStdString();

	if(load_rooms)
	{
		m_path = Path;
		m_title = Path.fileName();

		std::ifstream file(str, std::ios::binary);

		if(!file.is_open())
			throw std::system_error(errno, std::system_category(), str);

		file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );

		m_metaroom.faces = 0;
		m_metaroom.Read(m_window, file, 0);
		m_metaroom.CommitMove(false);

		file.close();

		m_window->SetAsterisk(false);
	}
	else if(load_background)
	{
        if(m_background)
          m_background->Release(gl);

		m_background.reset();

        m_background = std::make_unique<BackgroundImage>(gl, str);
	}


	return true;
}
