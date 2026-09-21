#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <fstream>
#include <loguru.hpp>
#include <QMessageBox>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef MAPEDITOR_TESTS
#include <gtest/gtest.h>
#endif

static void LogHandler(void* , const loguru::Message& message);

// Runs before any QApplication exists, so no window or GL context is created.
static int RunTests(int argc, char *argv[])
{
#ifdef MAPEDITOR_TESTS
	// An empty GTEST_FILTER selects nothing, and gtest reports "PASSED" over
	// zero tests with exit 0 -- a green that means nothing ran. Refuse it.
	if(const char* f = std::getenv("GTEST_FILTER"); f && *f == '\0')
	{
		fprintf(stderr, "GTEST_FILTER is set but empty: this runs zero tests "
		                "and exits 0. Unset it, or give it a pattern.\n");
		return -1;
	}

	// File tests need a (never shown) MainWindow, because the track list
	// lives in its widgets, and a hidden GL context for background uploads.
	// Without a display fall back to the offscreen platform, which has no GL
	// here, so the background tests skip.
	if(qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")
	&& qEnvironmentVariableIsEmpty("DISPLAY")
	&& qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY"))
		qputenv("QT_QPA_PLATFORM", "offscreen");

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication a(argc, argv);

	loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#else
	(void)argc; (void)argv;
	fprintf(stderr, "--gtest: tests are not compiled into this build (debug only).\n");
	return -1;
#endif
}

int main(int argc, char *argv[])
{
	for(int i = 1; i < argc; ++i)
	{
		if(strcmp(argv[i], "--gtest") == 0)
			return RunTests(argc, argv);
	}

	// The histogram widgets render using the shader programs owned by
	// viewWidget's GL context, so every QOpenGLWidget must share GL objects.
	// This attribute is the only supported way to do that in Qt6 (a per-widget
	// setShareContext() after context creation is a no-op); it must be set
	// before the QApplication (and thus any GL context) is created.
	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

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
