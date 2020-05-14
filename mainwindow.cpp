#include "src/glviewwidget.h"
#include "mainwindow.h"
#include "exportoptions.h"
#include "ui_mainwindow.h"
#include "src/document.h"
#include "src/walltype_db.h"
#include <QScreen>
#include <QMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <fstream>
#include <glm/glm.hpp>
#include <QStatusBar>

static QString g_imagePath;
static QDir g_blkPath = QDir::home();

extern const char * lf_BackgroundLayers[];


MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow),
escape(Qt::Key_Escape, this),
create(Qt::Key_N, this),
//duplicate(QKeySequence("Shift+Ctrl+D"), this),
translate(Qt::Key_G, this),
rotate(Qt::Key_R, this),
scale(Qt::Key_S, this),
extrude(Qt::Key_E, this),
slice(QKeySequence("Ctrl+R"), this),
lockX(Qt::Key_X, this),
lockY(Qt::Key_Y, this),
enter(Qt::Key_Z, this)
{
	ui->setupUi(this);

	escape.setContext(Qt::ApplicationShortcut);
	create.setContext(Qt::ApplicationShortcut);
//	duplicate.setContext(Qt::ApplicationShortcut);
	translate.setContext(Qt::ApplicationShortcut);
	rotate.setContext(Qt::ApplicationShortcut);
	scale.setContext(Qt::ApplicationShortcut);
	extrude.setContext(Qt::ApplicationShortcut);
	slice.setContext(Qt::ApplicationShortcut);
	lockX.setContext(Qt::ApplicationShortcut);
	lockY.setContext(Qt::ApplicationShortcut);
	enter.setContext(Qt::ApplicationShortcut);

	ui->viewWidget->w = this;

	connect(ui->aboutAbout,      &QAction::triggered, &QApplication::aboutQt);

    connect(ui->aboutHelp,       &QAction::triggered, this, &MainWindow::aboutHelp);
	connect(ui->aboutQt,         &QAction::triggered, &QApplication::aboutQt);
	connect(ui->appExit,         &QAction::triggered, &QApplication::closeAllWindows);
	connect(ui->editCopy,        &QAction::triggered, [this]() { document->Copy(); } );
	connect(ui->editCut,         &QAction::triggered, [this]() { document->Copy(); document->Delete(); });
    connect(ui->editDelete,      &QAction::triggered, [this]() { document->Delete(); } );
    connect(ui->editPaste,       &QAction::triggered, [this]() {

		document->Paste(ui->viewWidget->GetWorldPosition());

	});
    connect(ui->editRedo,        &QAction::triggered, [this]() {
		document->Redo();
		ui->editUndo->setEnabled(document->CanUndo());
		ui->editRedo->setEnabled(document->CanRedo());
		ui->viewWidget->need_repaint();
	});
    connect(ui->editSelectAll,   &QAction::triggered, [this]() { document->SelectAll(); ui->viewWidget->need_repaint(); });
//    connect(ui->editSelectNone,  &QAction::triggered, [this]() { document->SelectNone(); });
    connect(ui->editUndo,        &QAction::triggered, [this]()
	{
		document->Undo();
		ui->editUndo->setEnabled(document->CanUndo());
		ui->editRedo->setEnabled(document->CanRedo());
		ui->viewWidget->need_repaint();
	});

	connect(ui->fileLoadBackground,   &QAction::triggered, [this]() { fileOpen(false, true); });
    connect(ui->fileOpen,           &QAction::triggered, [this]() { fileOpen(true, false); });
	connect(ui->fileCAOS,           &QAction::triggered, this, &MainWindow::fileCAOS );
    connect(ui->fileImportWalls,   &QAction::triggered, this, &MainWindow::fileLoadWalls);
	connect(ui->fileNew,         &QAction::triggered, this, &MainWindow::fileNew);
    connect(ui->fileSave,        &QAction::triggered, this, &MainWindow::fileSave);
    connect(ui->fileSaveAs,      &QAction::triggered, this, &MainWindow::fileSaveAs);
//    connect(ui->linkAdd,         &QAction::triggered, [this]() { document->SetTool(Tool::AddLink); });
    connect(ui->linkRemove,      &QAction::triggered, [this]() { document->RemoveLinks(); });
   // connect(ui->musicLoad,       &QAction::triggered, this, &MainWindow::musicLoad);
  //  connect(ui->musicMute,       &QAction::toggled, this, &MainWindow::musicMute);

	connect(ui->toolNew,         &QAction::triggered,[this]() { toolbox.SetTool(Tool::Create); } );
//    connect(ui->toolDuplicate,   &QAction::triggered,[this]() { toolbox.SetTool(Tool::Duplicate); } );
    connect(ui->toolTranslate,   &QAction::triggered,[this]() { toolbox.SetTool(Tool::Translate); } );
    connect(ui->toolRotate,      &QAction::triggered,[this]() { toolbox.SetTool(Tool::Rotate); } );
	connect(ui->toolScale,       &QAction::triggered,[this]() { toolbox.SetTool(Tool::Scale); } );
//	connect(ui->toolExtrude,     &QAction::triggered,[this]() { toolbox.SetTool(Tool::Extrude); } );
	connect(ui->toolSlice,       &QAction::triggered,[this]() { toolbox.SetTool(Tool::Slice); } );
	connect(ui->toolReseat,      &QAction::triggered,[this]() { toolbox.SetTool(Tool::Order); } );

	connect(&escape,             &QShortcut::activated, [this]() { toolbox.SetTool(Tool::None); } );
	connect(&create,             &QShortcut::activated, [this]() { toolbox.SetTool(Tool::Create); } );
//	connect(&duplicate,          &QShortcut::activated, [this]() { toolbox.SetTool(Tool::Duplicate); } );
	connect(&translate,          &QShortcut::activated, [this]() { toolbox.SetTool(Tool::Translate); } );
	connect(&rotate,             &QShortcut::activated, [this]() { toolbox.SetTool(Tool::Rotate); } );
	connect(&scale,              &QShortcut::activated, [this]() { toolbox.SetTool(Tool::Scale); } );
	connect(&extrude,            &QShortcut::activated, [this]() { toolbox.SetTool(Tool::Extrude); } );
	connect(&slice,              &QShortcut::activated, [this]() { toolbox.SetTool(Tool::Slice); } );
	connect(&lockX,              &QShortcut::activated, [this]() { toolbox.lock(1, 0); } );
	connect(&lockY,              &QShortcut::activated, [this]() { toolbox.lock(0, 1); } );
	connect(&enter,              &QShortcut::activated, [this]() {
		toolbox.SetTool(Tool::Finish);
	} );

    connect(ui->viewChecker,     &QAction::triggered, ui->viewWidget, &GLViewWidget::need_repaint);
    connect(ui->viewHalls,       &QAction::triggered, ui->viewWidget, &GLViewWidget::need_repaint);
    connect(ui->viewLinks,       &QAction::triggered, ui->viewWidget, &GLViewWidget::need_repaint);
    connect(ui->viewRooms,       &QAction::triggered, ui->viewWidget, &GLViewWidget::need_repaint);
    connect(ui->zoomActual,      &QAction::triggered, [this]()
	{
		SetZoom(.5f);
	});
    connect(ui->zoomIn,          &QAction::triggered, [this]()
	{
		SetZoom(m_zoom * 9.f/8.f);
	});
    connect(ui->zoomOut,         &QAction::triggered, [this]()
	{
		SetZoom(m_zoom * 7.f/8);
	});

#define QDoubleSpinBoxChanged()  (void (QDoubleSpinBox::*)(double)) &QDoubleSpinBox::editingFinished
#define QSpinBoxChanged()  (void (QSpinBox::*)(int)) &QSpinBox::editingFinished
#define QComboBoxChanged()  (void (QComboBox::*)(int)) &QComboBox::currentIndexChanged

	connect(ui->room_music,         QComboBoxChanged(),      [this](int   value) { if(!updating_fields && value >= 0) document->SetRoomMusic(value); });
//	connect(ui->room_music->lineEdit(), &QLineEdit::editingFinished, this, &MainWindow::musicSet);

	ui->room_music->lineEdit()->setMaxLength(24);

#if HAVE_WALL_TYPE
	connect(ui->door_type,          QComboBoxChanged(),      [this](int   value) { if(!updating_fields && value >= 0) document->SetDoorType(value); });
#endif
	connect(ui->room_type,          QComboBoxChanged(),      [this](int   value) { if(!updating_fields && value >= 0) document->SetRoomType(value); });

    connect(ui->room_grav_x,        QDoubleSpinBoxChanged(), [this]() { if(!updating_fields) document->SetRoomGravity(ui->room_grav_x->value(), ui->room_grav_y->value()); } );
    connect(ui->room_grav_y,        QDoubleSpinBoxChanged(), [this]() { if(!updating_fields) document->SetRoomGravity(ui->room_grav_x->value(), ui->room_grav_y->value()); } );
	connect(ui->permeability,       QSpinBoxChanged(),       [this]() { if(!updating_fields) document->SetPermeability(ui->permeability->value()); } );


	/*
	connect(ui->permeability,       QSpinBoxChanged(),       [this](int   value) { document->SetDoorValue(&DoorTypes::permeability, value); } );
	connect(ui->friction,           QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::friction, value); } );
	connect(ui->elasticity,         QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::elasticity, value); } );
	connect(ui->dampening,          QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::dampening, value); } );

    connect(ui->thermal_baseline,   QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::thermal_baseline, value); } );
	connect(ui->thermal_absorbtion, QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::thermal_absorbtion, value); } );
	connect(ui->thermal_capacity,   QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::thermal_capacity, value); } );
	connect(ui->thermal_loss_rate,  QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::thermal_loss_rate, value); } );

	connect(ui->water_baseline,     QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::water_baseline, value); } );
	connect(ui->water_absorbtion,   QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::water_absorbtion, value); } );
    connect(ui->water_capacity,     QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::water_capacity, value); } );
    connect(ui->water_loss_rate,    QDoubleSpinBoxChanged(), [this](float value) { document->SetDoorValue(&DoorTypes::water_loss_rate, value); } );
	*/

/*
	connect(ui->add_wall, &QPushButton::released, this, &MainWindow::AddDoorType);
    connect(ui->dup_wall, &QPushButton::released, this, &MainWindow::CopyDoorType);
    connect(ui->select_wall, &QPushButton::released,this, &MainWindow::SelectDoorType);
    connect(ui->remove_wall, &QPushButton::released, this, &MainWindow::RemoveDoorType);*/

   // connect(ui->set_type, &QPushButton::released, this, &MainWindow::SetDoorType);
   // connect(ui->select_type, &QPushButton::released, this, &MainWindow::SelectDoorType);

	connect(ui->horizontalScrollBar, &QScrollBar::valueChanged, ui->viewWidget, &GLViewWidget::need_repaint);
	connect(ui->verticalScrollBar, &QScrollBar::valueChanged, ui->viewWidget, &GLViewWidget::need_repaint);

/*


	//	connect(ui->horizontalScrollBar, &QScrollBar::valueChanged, ui->openGLWidget, &QOpenGLWidget::needRepaint);
	//	connect(ui->verticalScrollBar, &QScrollBar::valueChanged, ui->openGLWidget, &QOpenGLWidget::needRepaint);


*/
//	QTreeView *textures;
//	QListView *tasks;
//	QListView *wall_list;

	setGeometry(
	    QStyle::alignedRect(
	        Qt::LeftToRight,
	        Qt::AlignCenter,
	        size(),
	        qApp->primaryScreen()->availableGeometry()
	    )
	);

	this->setFocus();

	SetDocument(std::unique_ptr<Document>(new Document(this)));
	setWindowTitle(tr("%1 - Map Editor").arg(document->Name()));

// Load default walls
}

MainWindow::~MainWindow()
{
	if(!did_release) {
		did_release = true;

        toolbox.Release(ui->viewWidget);
	}
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if(!fileClose())
		event->ignore();
	else
	{
		QMainWindow::closeEvent(event);
		document.reset();

		if(!did_release) {
			did_release = true;
		}
	}
}

void MainWindow::OnSelectionChanged()
{
	ui->viewWidget->makeCurrent();

    document->OnSelectionChanged(ui->viewWidget);
	bool selected = document->noFacesSelected() != 0;

	auto gravity = glm::unpackHalf2x16(document->gravity);

	updating_fields = true;

	UpdateTrackField();
	ui->room_grav_x->setValue(gravity.x);
	ui->room_grav_y->setValue(gravity.y);
	ui->room_grav_x->setEnabled(selected);
	ui->room_grav_y->setEnabled(selected);

	ui->room_type->setCurrentIndex(document->room_type);
	ui->room_type->setEnabled(selected);

	ui->permeability->setValue(document->permeability);
	ui->permeability->setEnabled(document->noEdgesSelected() != 0);

	ui->room_music->setEnabled(selected);

#if HAVE_WALL_TYPE
	ui->door_type->setCurrentIndex(document->wall_type);
	ui->door_type->setEnabled(document->noEdgesSelected() != 0);
#endif

	//ui->editCut->setEnabled(selected);
	//ui->editCopy->setEnabled(selected);

	updating_fields = false;
}

std::vector<std::string> MainWindow::GetTrackList(int8_t * music, uint32_t length)
{
	std::vector<std::string> r;

	if(ui->room_music->count() == 0)
		return r;

	std::vector<uint32_t> refCount(ui->room_music->count(), 0);

	for(uint32_t i = 0; i < length; ++i)
		if(music[i] >= 0) refCount[music[i]] += 1;

	std::vector<std::pair<QString, uint32_t>> sort_vector;

	for(uint32_t i = 0; i < refCount.size(); ++i)
	{
		sort_vector.push_back({ui->room_music->itemText(i), i});
	}

	std::sort(sort_vector.begin(), sort_vector.end(), [](auto const& a, auto const& b)
	{
		return a.first < b.first;
	});

	r.reserve(ui->room_music->count());
	for(uint32_t i = 0; i < refCount.size(); ++i)
	{
		if(refCount[sort_vector[i].second])
		{
			refCount[sort_vector[i].second] = r.size();
			r.push_back(sort_vector[i].first.toStdString());
		}
	}

	for(uint32_t i = 0; i < length; ++i)
		if(music[i] >= 0) music[i] = refCount[music[i]];

	SetTrackList(r);
	return r;
}

void MainWindow::SetTrackList(std::vector<std::string> const& tracks)
{
	QStringList list;
	for(auto & str : tracks)
		list << QString::fromStdString(str);

	updating_fields = true;
	ui->room_music->clear();
	ui->room_music->addItems(list);
	updating_fields = false;
}

void MainWindow::SetStatusBarMessage(const char * m)
{
	statusBar()->showMessage(m);
}

bool MainWindow::fileClose()
{
	if(m_asterisk == true)
	{
		QMessageBox::StandardButton button = QMessageBox::question(
			this,
			QGuiApplication::applicationDisplayName(),
			tr("Save changes to %1?").arg(document->Name()),
			QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
			QMessageBox::Cancel);

		if(button == QMessageBox::Cancel
		||(button == QMessageBox::Yes && fileSave() == false))
			return false;
	}

	return true;
}

bool MainWindow::SetAsterisk(bool value)
{
	if(value == m_asterisk)
		return false;

	m_asterisk = value;
	setWindowTitle(tr("%1%2 - Map Editor").arg(value? "*" : "").arg(document->Name()));
	return true;
}

bool MainWindow::fileNew()
{
	if(fileClose() == false)
		return false;

	SetDocument(std::unique_ptr<Document>(new Document(this)));
	OnSelectionChanged();
	ui->viewWidget->need_repaint();
	SetAsterisk(false);

	ui->fileCAOS->setEnabled(false);

	return true;
}

bool MainWindow::fileSave()
{
	if(!document->m_path.isFile())
		return fileSaveAs();

	return document->SaveFile(document->m_path);
}

bool MainWindow::fileSaveAs()
{
	QFileDialog dialog(this, tr("Save Background File"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setNameFilter("Room System (*.lf_mta)");
	dialog.setDirectory(g_blkPath);

	bool accepted;
	while ((accepted = (dialog.exec() == QDialog::Accepted)) && !document->SaveFile(dialog.selectedFiles().first())) {}

	if(!accepted)
		return false;

	g_blkPath = dialog.directory();

	return true;
}

void MainWindow::fileCAOS()
{
	QFileDialog dialog(this, tr("Save Cos File"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setNameFilter("Creatures Object Source (*.cos)");
	dialog.setDirectory(g_blkPath);

	bool accepted;
	while ((accepted = (dialog.exec() == QDialog::Accepted)))
	{
		g_blkPath = dialog.directory();

		ExportOptions options(this);
		options.configure(document->m_background.get());

		if(QDialog::Rejected == options.exec())
			break;

		try
		{
			auto music = GetTrackList(&document->m_metaroom.m_music[0], document->m_metaroom.size());
			options.save(&document->m_metaroom, dialog.selectedFiles().first().toStdString(), std::move(music));
			break;
		}
		catch(std::exception &e)
		{
			if(QMessageBox::Ok == QMessageBox::warning(this, "Failed to open file.", e.what() + tr("\n try again?")))
				return;
		}
	}
}

bool MainWindow::fileOpen(bool load_rooms, bool load_background)
{
	if(load_rooms == false && load_background == false)
		return false;

	ui->viewWidget->makeCurrent();

	if(load_rooms == true && load_background == true)
	{
		if(fileNew() == false)
			return false;
	}

	QFileDialog dialog(this, load_background? tr("Load Background") : tr("Load Room System"));
	dialog.setAcceptMode(QFileDialog::AcceptOpen);

	if(load_background == true)
		dialog.setNameFilter("Background (*.spr *.s16 *.blk *.lf_bck)");
	else if(load_rooms == true)
		dialog.setNameFilter("Room System (*.lf_mta)");

	dialog.setDirectory(g_blkPath);

	bool accepted;
	while ((accepted = (dialog.exec() == QDialog::Accepted)))
	{
		try
		{
            if(document->LoadFile(ui->viewWidget, dialog.selectedFiles().first(), load_rooms, load_background))
				break;
		}
		catch(std::exception & e)
		{
			if(QMessageBox::Ok == QMessageBox::warning(this, "Failed to open file.", e.what() + tr("\n try again?")))
				return false;
		}
	}

	if(!accepted)
		return false;

	g_blkPath = dialog.directory();
	m_asterisk = true;
	SetAsterisk(false);

	ui->viewWidget->need_repaint();

	ui->fileCAOS->setEnabled(document->isCreaturesBackground());
	return true;
}

void MainWindow::SetDocument(std::unique_ptr<Document> && doc)
{
    if(document != nullptr)
      document->Release(ui->viewWidget);

	document = std::move(doc);
	SetEnabled();
}

glm::ivec2 MainWindow::WidgetSize()
{
	auto size = ui->viewWidget->size();
	return glm::ivec2(size.width(), size.height());
}

float MainWindow::SetZoom(float zoom)
{
const float g_MinZoom = .125f;
const float g_MaxZoom = 2.f;

	float p_zoom = m_zoom;

	m_zoom = std::max(g_MinZoom, std::min(g_MaxZoom, zoom));

	ui->zoomIn->setEnabled(m_zoom < g_MaxZoom);
	ui->zoomOut->setEnabled(m_zoom > g_MinZoom);

	ui->horizontalScrollBar->setPageStep(m_zoom*64);
	ui->verticalScrollBar->setPageStep(m_zoom*64);

	ui->viewWidget->need_repaint();

	return p_zoom / m_zoom;
}

void MainWindow::fileLoadWalls()
{
	QFileDialog dialog(this, tr("Load Walls"));
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setNameFilter("Json (*.json)");
	dialog.setDirectory(g_blkPath);

	while(dialog.exec() == QDialog::Accepted)
	{
		g_blkPath = dialog.directory();
		std::string path = dialog.selectedFiles().first().toStdString();

		try
		{
			std::ifstream file(path);

			if (!file.is_open())
			{
				throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory), path);
			}

			MapData::WallTypeDB db = nlohmann::json::parse(file);

			updating_fields = true;
			db.UpdateTypeLists(ui);
			updating_fields = false;

			std::unique_ptr<uint8_t[]> table(new uint8_t[db.door_types.size()]);
			for(uint i = 0; i < db.door_types.size(); ++i)
			{
				table[i] = db.door_types[i].permeability;
			}

			ui->viewWidget->upload_permeabilitys(&table[0], db.door_types.size());
			break;
		}
		catch(std::exception & e)
		{
			if(QMessageBox::Ok != QMessageBox::warning(this, windowTitle(), "Failed to open file:\n" + QString::fromStdString(path) + "\n" + e.what() + tr("\n try again?")))
				return;
		}
	}

}

void MainWindow::loadDefaultWalls()
{
	MapData::WallTypeDB db;

	updating_fields = true;
	db.UpdateTypeLists(ui);
	updating_fields = false;

	std::unique_ptr<uint8_t[]> table(new uint8_t[16]);
	memset(&table[0], 0, 16);
	table[0] = 255;
	table[1] = 128;

	ui->viewWidget->upload_permeabilitys(&table[0], 16);
}

#if HAVE_FMOD
bool MainWindow::musicLoad()
{
	QFileDialog dialog(this, tr("Load Music Bank"));
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setNameFilter("FMOD bank (*.bank)");
	dialog.setDirectory(g_blkPath);

	while(dialog.exec() == QDialog::Accepted)
	{
		std::string path = dialog.selectedFiles().first().toStdString();

		try
		{
			FMODManager::Get()->LoadBank(path.c_str());
			g_blkPath = dialog.directory();
			break;
		}
		catch(std::exception & e)
		{
			if(QMessageBox::Ok != QMessageBox::warning(this, windowTitle(), "Failed to open file:\n" + QString::fromStdString(path) + "\n" + e.what() + tr("\n try again?")))
				return false;
		}
	}

	updating_fields = true;

	ui->room_music->clear();
	ui->room_music->addItems(FMODManager::Get()->GetTrackTitles());

	UpdateTrackField();
	updating_fields = false;

	return true;
}
#endif


void MainWindow::UpdateTrackField()
{
	int track = 	document->m_metaroom.GetMusicTrack();
#if HAVE_FMOD
	if(track > 0)
	{
		FMOD_GUID id;
		memcpy(&id, &document->m_metaroom.m_musicDB[track].guid, sizeof(FMOD_GUID));
		auto const& tracks = FMODManager::Get()->GetTrackGUIDs();

		if((uint)track >= tracks.size() || memcmp(&id, &tracks[track], sizeof(FMOD_GUID)))
		{
			track = -1;

			for(uint i = 0; i < tracks.size(); ++i)
			{
				if(memcmp(&id, &tracks[i], sizeof(FMOD_GUID)) == 0)
				{
					track = i;
					break;
				}
			}
		}
	}
#endif

	ui->room_music->setCurrentIndex(track);
}
/*
void MainWindow::SetDoorType()
{
	document->m_metaroom.SetDoorType(ui->WallTypes->currentRow());
}

void MainWindow::SelectDoorType()
{
	int r = document->m_metaroom.GetDoorType();

	if(r < 0)
	{
		QMessageBox::warning(this, windowTitle(), tr("Rooms of multiple types selected."));
		return;
	}

	if(ui->WallTypes->count() < r)
	{
		QMessageBox::warning(this, windowTitle(), tr("Room type greater than number of room types loaded."));
		return;
	}

	ui->WallTypes->setCurrentRow(r);
}
*/

void MainWindow::SetEnabled()
{
//	ui->editCut->setEnabled(false);
//	ui->editCopy->setEnabled(false);
//	ui->editDelete->setEnabled(false);

	ui->editUndo->setEnabled(document->CanUndo());
	ui->editRedo->setEnabled(document->CanRedo());


}

void MainWindow::need_repaint()
{
	ui->viewWidget->need_repaint();
}


bool MainWindow::viewChecker() const { return ui->viewChecker->isChecked(); }
bool MainWindow::viewRooms() const { return ui->viewRooms->isChecked(); }
bool MainWindow::viewHalls() const { return ui->viewHalls->isChecked(); }
bool MainWindow::viewLinks() const { return ui->viewLinks->isChecked(); }


glm::vec2  MainWindow::GetScroll()
{
	return glm::vec2(
		(ui->horizontalScrollBar->value() - ui->horizontalScrollBar->minimum())
			/ (double) (ui->horizontalScrollBar->maximum() - ui->horizontalScrollBar->minimum()),
		 1 - (ui->verticalScrollBar->value() - ui->verticalScrollBar->minimum())
			/ (double) (ui->verticalScrollBar->maximum() - ui->verticalScrollBar->minimum()));
}

void  MainWindow::SetScroll(glm::vec2 scroll)
{
	scroll = glm::max(glm::vec2(0), glm::min(scroll, glm::vec2(1)));
	int scroll_x = glm::mix(ui->horizontalScrollBar->minimum(), ui->horizontalScrollBar->maximum(), scroll.x);
	int scroll_y = glm::mix(ui->verticalScrollBar->minimum(), ui->verticalScrollBar->maximum(), 1 - scroll.y);

	ui->horizontalScrollBar->setValue(scroll_x);
	ui->verticalScrollBar->setValue(scroll_y);
}
