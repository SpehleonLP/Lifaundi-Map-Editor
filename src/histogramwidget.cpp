#include "histogramwidget.h"
#include "mainwindow.h"
#include "qtimer.h"
#include "ui_mainwindow.h"
#include <QHelpEvent>
#include <QToolTip>

QColor getParentBackgroundColor(QWidget *widget) {

	// Default color if no parent is available
	if (widget == nullptr)
		return QColor(); // Returns an invalid color

	if(widget->parentWidget())
		widget = widget->parentWidget();

	QPalette palette = widget->palette();
	QColor bgColor = palette.color(widget->backgroundRole());

	return bgColor;
}

HistogramWidget::HistogramWidget(QWidget * p) :
	QOpenGLWidget(p),
	_timer(this)
{
	_timer.setSingleShot(false);
	_timer.setInterval(20);
	connect(&_timer, &QTimer::timeout, this, [this]() { repaint(); } );
}

HistogramWidget::~HistogramWidget()
{
	_histogram.Destroy(this);
}

void HistogramWidget::need_repaint()
{
	if(!_timer.isActive())
		_timer.start();
}

void HistogramWidget::initializeGL()
{
	if(_initialized) return;
	_initialized = true;

	w->ui->viewWidget->initializeGL();

	makeCurrent();
	_shaders = w->ui->viewWidget->_shaders;
	context()->setShareContext(w->ui->viewWidget->context());


	QOpenGLFunctions_4_5_Core::initializeOpenGLFunctions();
	super::initializeGL();

	_histogram.Initialize(this);
}

void HistogramWidget::paintGL()
{
	int width = size().width();
	int height = size().height();

	glViewport(0, 0, width, height);

	auto color = getParentBackgroundColor(this);

	glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	glClear(GL_COLOR_BUFFER_BIT);

	_histogram(_shaders.get(), glm::uvec2{0, 15000}, width);
}

void HistogramWidget::resizeGL(int w, int h)
{
	QOpenGLWidget::resizeGL(w, h);
}


void HistogramWidget::mouseMoveEvent 		(QMouseEvent * event)
{
	super::mouseMoveEvent(event);
}

void HistogramWidget::mousePressEvent		(QMouseEvent * event)
{
	super::mousePressEvent(event);
}

void HistogramWidget::mouseReleaseEvent		(QMouseEvent * event)
{
	super::mouseReleaseEvent(event);
}

void HistogramWidget::mouseDoubleClickEvent	(QMouseEvent * event)
{
	super::mouseDoubleClickEvent(event);
}


void HistogramWidget::wheelEvent				(QWheelEvent * event)
{
	super::wheelEvent(event);
}

bool HistogramWidget::event					(QEvent *event)
{
	if(event->type() != QEvent::ToolTip)
		return super::event(event);

   QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

   QString string; // = window->getToolTip(helpEvent->pos());

   if(!string.isEmpty())
   {
	   QToolTip::showText(helpEvent->globalPos(), string);
   }
   else
   {
	   QToolTip::hideText();
	   event->ignore();
   }

   return true;
}

