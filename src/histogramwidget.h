#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H
#include "histogram.h"
#include "qtimer.h"
#include "src/document.h"
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>

class MainWindow;
class Shaders;

class HistogramWidget : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
typedef QOpenGLWidget super;
public:
	HistogramWidget(QWidget * p);
	virtual ~HistogramWidget();

	void need_repaint();

	MainWindow * w{};
	Histogram _histogram;

private:
	void mouseMoveEvent 		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mousePressEvent		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mouseReleaseEvent		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent	(QMouseEvent * event)	Q_DECL_OVERRIDE;

	void wheelEvent				(QWheelEvent * event)   Q_DECL_OVERRIDE;
	bool event					(QEvent *event)			Q_DECL_OVERRIDE;

	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;

	bool _initialized = false;
	QTimer	_timer;

	std::shared_ptr<Shaders> _shaders;

};

#endif // HISTOGRAMWIDGET_H
