#ifndef DOCUMENT_H
#define DOCUMENT_H
#include <QString>
#include <QFileInfo>
#include <glm/vec2.hpp>
#include "commandlist.h"
#include "controllerfsm.h"
#include "mainwindow.h"
#include "metaroom.h"
#include <memory>
#include <vector>

class BackgroundImage;
class CommandInterface;
class MainWindow;
class DoorTypes;
class GLViewWidget;

class Document
{
public:
	Document(MainWindow * window);
	~Document();

    void Release(GLViewWidget * gl);

    void RenderBackground(GLViewWidget * gl);

    void OnSelectionChanged(GLViewWidget *gl);
	uint32_t noFacesSelected() const;
	uint32_t noEdgesSelected() const;

	void PushCommand(CommandInterface *);
	bool CanUndo() const { return m_command > 0; }
	bool CanRedo() const { return m_command < m_history.size(); }

	QString Name() const { return m_title; };

	bool SaveFile(QFileInfo const& path);
    bool LoadFile(GLViewWidget *gl, QFileInfo const& path, bool load_rooms, bool load_background);

	bool Undo();
	bool Redo();
	void Copy();
	void Delete();
	void Reseat();
	void Paste(glm::vec2 center);
	void Duplicate(glm::vec2 center);
	void SelectAll() ;
	void SelectNone() {}
	void RemoveLinks() {}

	void ReseatSelection();

	void SetRoomMusic(int);
	void SetRoomType(int);
	void SetDoorType(int);
	void SetRoomGravity(float, float);
	void SetDrawDistance(float);
	void PushSettingCommand(uint32_t value, SettingCommand::Type type);
	void SetPermeability(int);

	void setImage(int layer, int mip, QImage && image);
	void createMipMap(int i, int j);
	void clearMipMap(int i, int j);

	glm::vec2 GetScreenCenter() const;
	glm::vec2 GetDimensions() const;

	void toggleVisibility(int i, int state);

	void toggleVisibility(int i, int j, int state)
	{
		toggleVisibility(lf_NoBackgroundLayers + i * lf_NoMipMaps + j, state);
	}

	bool isCreaturesBackground() const;
	bool hasImage() const { return m_background != nullptr; }
	int width() const;
	int height() const;

	MainWindow * m_window{};

	QFileInfo  m_path;
	QString    m_title{"<untitled>"};

	uint32_t m_visibility{~0u};

	std::unique_ptr<BackgroundImage>               m_background;
	Metaroom                                       m_metaroom;

	std::vector<std::unique_ptr<CommandInterface>> m_history;
	size_t                                         m_command{};

	int       track{};
	uint32_t  gravity{};
	uint16_t  drawDistance{};
	int       room_type{};
	int       wall_type{};
	int       permeability{};
};

#endif // DOCUMENT_H
