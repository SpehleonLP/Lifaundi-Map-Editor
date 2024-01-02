#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <fstream>
#include <loguru.hpp>
#include <QMessageBox>

static void LogHandler(void* , const loguru::Message& message);

int main(int argc, char *argv[])
{

	QApplication a(argc, argv);
	MainWindow w;

	// Put every log message in "everything.log":
	loguru::add_file("Log/status.log", loguru::Truncate, loguru::Verbosity_MAX);

	// Only log INFO, WARNING, ERROR and FATAL to "latest_readable.log":
	loguru::add_file("Log/error.log", loguru::Truncate, loguru::Verbosity_INFO);

	// Only show most relevant things on stderr:
	loguru::g_stderr_verbosity = 0;
	loguru::add_callback("WARNING",&LogHandler,&w, loguru::Verbosity_WARNING);
//	loguru::add_callback("ERROR",&LogHandler,nullptr, loguru::Verbosity_ERROR);
//	loguru::set_fatal_handler(&FatalHandler);

	w.show();

	return a.exec();
}

void LogHandler(void* w, const loguru::Message& message)
{
	if(message.verbosity <= loguru::Verbosity_ERROR)
	{
		QMessageBox::critical((MainWindow*)w, QMainWindow::tr("Error"),
									  message.message);
	}
	else if(message.verbosity == loguru::Verbosity_WARNING)
	{
		QMessageBox::warning((MainWindow*)w, QMainWindow::tr("Warning"),
									  message.message);
	}
	else
	{
		QMessageBox::information((MainWindow*)w, QMainWindow::tr("Warning"),
									  message.message);
	}

	if(message.verbosity == loguru::Verbosity_ERROR)
	{
		((MainWindow*)w)->close();
	}

	if(message.verbosity == loguru::Verbosity_FATAL)
	{
		exit(-1);
	}
}
