#ifndef GLVIEWWIDGET_H
#define GLVIEWWIDGET_H
#include "Support/counted_ptr.hpp"
#include "enums.hpp"
#include <glm/vec2.hpp>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>
#include <QTimer>
#include <chrono>

class MainWindow;
class Shaders;

class GLViewWidget : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
typedef QOpenGLWidget super;
//	Q_OBJECT
public:
	GLViewWidget(QWidget * p);
	virtual ~GLViewWidget();

	void need_repaint();

    MainWindow * w{};

	glm::vec2 GetWorldPosition(QMouseEvent * event);
	glm::vec2 GetScreenPosition(QMouseEvent * event);
	glm::vec2 GetWorldPosition();
	glm::vec2 GetScreenPosition();
	Bitwise   GetFlags(QMouseEvent * event);

	void upload_permeabilitys(uint8_t * table, int size);
	Shaders * shaders() const { return _shaders.get(); }

private:
friend class HistogramWidget;
	void mouseMoveEvent 		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mousePressEvent		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mouseReleaseEvent		(QMouseEvent * event)	Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent	(QMouseEvent * event)	Q_DECL_OVERRIDE;

	void wheelEvent				(QWheelEvent * event)   Q_DECL_OVERRIDE;
	bool event					(QEvent *event)			Q_DECL_OVERRIDE;

	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

	QTimer timer;
	std::shared_ptr<Shaders> _shaders;

	std::chrono::time_point<std::chrono::high_resolution_clock> current_time;

	uint32_t m_ubo{};
	uint32_t permeabilities{};

	bool		_initialized{false};
	bool       m_canvasDrag{false};
	glm::vec2  m_scrollPos{0,0};
	glm::vec2  m_screenPos{0,0};
	glm::vec2  m_mouseWorldPosition;
};


#endif // GLVIEWWIDGET_H
