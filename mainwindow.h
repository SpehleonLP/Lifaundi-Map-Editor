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

class Document;
class QPushButton;
class QComboBox;
class QCheckBox;
class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void CreateDoorType();

	void documentNew();
    void documentOpen();

	void OnSelectionChanged();
	void loadDefaultWalls();

	void SetStatusBarMessage(const char * m = nullptr);

	bool viewChecker() const;
    bool viewRooms() const;
    bool viewHalls() const;
    bool viewLinks() const;

	bool SetAsterisk(bool);

	void need_repaint();

	glm::vec2 GetScroll();
	void      SetScroll(glm::vec2 scroll);
	glm::ivec2 WidgetSize();

	float SetZoom(float zoom);
	float GetZoom() const { return m_zoom; }

	void onMouseEvent(QMouseEvent*) {}

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

	QShortcut escape;

	QShortcut create;
	//QShortcut duplicate;
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
