#include "mainwindow.h"
#include "src/fmodmanager.h"
#include <QApplication>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
	FMODManager f_mod;

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
