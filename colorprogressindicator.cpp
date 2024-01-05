#include "colorprogressindicator.h"
#include "ui_colorprogressindicator.h"

ColorProgressIndicator::ColorProgressIndicator(QWidget *parent, std::shared_ptr<ColorProgress> progress) :
	QMainWindow(parent),
	ui(new Ui::ColorProgressIndicator),
	_progress(progress)
{
	ui->setupUi(this);
}

ColorProgressIndicator::~ColorProgressIndicator()
{
	delete ui;
}

void ColorProgressIndicator::Update()
{
	ui->IslandsCompleted->setText(QString("%1 / %2").arg(_progress->_processedIslands.load()).arg(_progress->_totalIslands));
	ui->CurrentProgress->setText(QString("%1 / %2").arg(_progress->_currentRoom.load()).arg(_progress->_totalRooms));
	ui->TimesBacktracked->setText(QString("%1 : %2").arg(_progress->_backtrackInIsland.load()).arg(_progress->_totalBacktrack));
	ui->CurrentColors->setText(QString("?"));
}
