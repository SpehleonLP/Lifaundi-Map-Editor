#include "histogramwidget.h"
#include "mainwindow.h"
#include "qtimer.h"
#include "ui_mainwindow.h"
#include "widget_rangeslider.h"
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
	_timer.setSingleShot(true);
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

	// NOTE: QOpenGLWidget init order is undefined — this may run before
	// viewWidget::initializeGL(), so we must NOT touch viewWidget->_shaders
	// here. Cross-context GL object sharing is set up globally via
	// Qt::AA_ShareOpenGLContexts in main(). Only this widget's own GL
	// resources are created here; _shaders is fetched lazily in paintGL().
	makeCurrent();

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

	// viewWidget owns the shared shader programs. Its initializeGL() may not
	// have run yet (QOpenGLWidget init order is undefined), so wait for it
	// rather than dereferencing a null _shaders.
	if(!w->ui->viewWidget->initialized())
	{
		need_repaint();
		return;
	}

	if(!_shaders)
		_shaders = w->ui->viewWidget->_shaders;

	auto range = glm::uvec2(
		_displayRange->GetLowerValue() * 65535 / 256,
		_displayRange->GetUpperValue() * 65535 / 256
	);

	_histogram(_shaders.get(), range, width);
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

