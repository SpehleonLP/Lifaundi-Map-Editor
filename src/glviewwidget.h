#ifndef GLVIEWWIDGET_H
#define GLVIEWWIDGET_H
#include "qt-gl/gl_viewwidget.h"
#include "Support/counted_ptr.hpp"
#include "enums.hpp"
#include <glm/vec2.hpp>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>
#include <QTimer>
#include <chrono>
#include <memory>

class MainWindow;
class Shaders;

class GLViewWidget : public gl::ViewWidget
{
typedef QOpenGLWidget super;
//	Q_OBJECT
public:
	GLViewWidget(QWidget * p);
	virtual ~GLViewWidget();

	void SetMainWindow(MainWindow * w);

	void need_repaint();

	void upload_permeabilitys(uint8_t * table, int size);
	Shaders * shaders() const { return _shaders.get(); }

private:
friend class HistogramWidget;
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;

	std::shared_ptr<Shaders> _shaders;
	std::chrono::time_point<std::chrono::high_resolution_clock> current_time;

	MainWindow * w;
	uint32_t m_ubo{};
	uint32_t permeabilities{};
};


#endif // GLVIEWWIDGET_H
