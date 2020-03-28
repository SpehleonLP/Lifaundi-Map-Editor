#include "backgroundwindow.h"
#include "ui_backgroundwindow.h"

BackgroundWindow::BackgroundWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::BackgroundWindow)
{
	ui->setupUi(this);
}

BackgroundWindow::~BackgroundWindow()
{
	delete ui;
}
