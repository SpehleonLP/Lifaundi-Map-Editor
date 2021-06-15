#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "src/enums.hpp"
#include "src/controllerfsm.h"
#include <memory>
#include <QMainWindow>
#include <QShortcut>
#include <glm/vec2.hpp>

enum
{
	lf_NoBackgroundLayers = 3,
	//diffuse
	//depth
	//gloss + metal
	lf_NoMipMaps          = 3,
};

namespace MapData
{
struct WallTypeDB;
};

namespace Ui {
class MainWindow;
}

namespace FMOD
{
class Channel;
}

#define HAVE_WALL_TYPE 0
#define HAVE_FMOD 0

class Document;
class QPushButton;
class QComboBox;
class QCheckBox;
class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	std::vector<std::string> GetTrackList(int8_t * music, uint32_t length);
	void SetTrackList(std::vector<std::string> const&);

	void CreateDoorType();

	void documentNew();
    void documentOpen();

	void OnSelectionChanged();
	void loadDefaultWalls();

	void SetStatusBarMessage(const char * m = nullptr);
	void SetStatusBarMessage(glm::ivec2 coords);

	bool viewChecker() const;
    bool viewRooms() const;
    bool viewHalls() const;
    bool viewLinks() const;
	bool viewGravity() const;

	bool SetAsterisk(bool);

	bool musicSet();

	glm::ivec2 GetWorldPosition() const;
	void SetMouseTracking(bool);
	void need_repaint();

	glm::vec2 GetScroll();
	void      SetScroll(glm::vec2 scroll);
	glm::ivec2 WidgetSize();

	float SetZoom(float zoom);
	float GetZoom() const { return m_zoom; }

	void onMouseEvent(QMouseEvent*) {}
	void DisplayString(std::string && str);

	Ui::MainWindow *ui;

	ControllerFSM toolbox{this};
	std::unique_ptr<Document>  document;

private:
friend class ui_BackgroundsWindow;
friend class GLViewWidget;
friend class Document;

	bool aboutHelp() { return false; }

	bool fileClose();

	void fileLoadWalls();
	bool fileNew();
	void fileCAOS();
	bool fileOpen(bool load_rooms, bool load_background);
	bool fileSave();
	bool fileSaveAs();

	bool musicLoad();

	void SetDocument(std::unique_ptr<Document> && document);
	void SetEnabled();

	void SelectDoors();
	void DeselectDoors();
	void SetDoorType();
	void SelectDoorType();

	void UpdateTrackField();

	void closeEvent(QCloseEvent *event);

	float m_zoom{.5f};
	bool m_title_dirty{};
	bool did_release{false};
	bool updating_fields{false};
	bool m_asterisk{false};
	bool m_haveMessage{false};

	QShortcut escape;

	QShortcut create;
	QShortcut face;
	QShortcut duplicate;
	QShortcut translate;
	QShortcut rotate;
	QShortcut scale;
	QShortcut extrude;
	QShortcut slice;
	QShortcut lockX;
	QShortcut lockY;
	QShortcut enter;
};



#endif // MAINWINDOW_H
