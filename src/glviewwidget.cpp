#include "document.h"
#include "glviewwidget.h"
#include "mainwindow.h"
#include "src/backgroundimage.h"
#include "ui_mainwindow.h"
#include "Shaders/shaders.h"
#include "lib/qt-gl/initialize_gl.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QPainter>
#include <QCursor>
#include <QHelpEvent>
#include <QToolTip>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QEvent>
#include <cmath>
#include <chrono>
#include <iostream>

struct MapParent : public gl::iParent
{
	static Bitwise GetFlags(gl::Modifiers mod)
	{
		switch(mod)
		{
		default: return Bitwise::SET;
		case gl::And: return Bitwise::AND;
		case gl::Or: return Bitwise::OR;
		case gl::Xor: return Bitwise::XOR;
		}
	}

	static bool alt(gl::Modifiers mod)
	{
		return mod & gl::AltModifier? 0 : 1;
	}


	MapParent(MainWindow * w) : w(w) {}
	~MapParent() = default;

	MainWindow * w{};

	QScrollBar * horizontalScrollBar() override { return w->ui->horizontalScrollBar; }
	QScrollBar * verticalScrollBar() override { return w->ui->verticalScrollBar;  }
	QString      getToolTip(glm::vec2) const override { return {}; }

	float GetZoom() const override { return w->GetZoom(); }
	float SetZoom(float v) override { return w->SetZoom(v); }

	glm::vec2 GetScroll() const override { return w->GetScroll(); }
	glm::vec2 SetScroll(glm::vec2 x) override { return w->SetScroll(x); }

	glm::vec2 GetScreenCenter() const override { return w->document? w->document->GetScreenCenter() : glm::vec2{64}; }
	glm::vec2 GetDocumentSize() const override { return w->document? w->document->GetDimensions() : glm::vec2{64}; }
	bool NeedTrackMouse() const override { return w->toolbox.HaveTool(); }
	void UpdateStatusBarMessage(glm::vec2 worldPosition) override { return w->SetStatusBarMessage(worldPosition); }

	bool OnMouseWheel(glm::vec2 angleDelta) override
	{
		return w->toolbox.wheelEvent(angleDelta);
	}
	bool OnMouseMove(glm::vec2 worldPosition, gl::Modifiers mod) override
	{
		return w->toolbox.OnMouseMove(worldPosition, GetFlags(mod));
	}

	bool OnMouseDown(glm::vec2 worldPosition, gl::Modifiers mod) override
	{
		if(mod & gl::LeftDown)
		{
			return w->toolbox.OnLeftDown(worldPosition, GetFlags(mod), alt(mod));
		}

		return false;
	}

	bool OnMouseUp(glm::vec2 worldPosition, gl::Modifiers mod) override
	{
		if(mod & gl::LeftDown)
		{
			return w->toolbox.OnLeftUp(worldPosition, GetFlags(mod), alt(mod));
		}

		return false;
	}
	bool OnDoubleClick(glm::vec2 worldPosition, gl::Modifiers mod) override
	{
		if(mod & gl::LeftDown)
		{
			return w->toolbox.OnDoubleClick(worldPosition, GetFlags(mod));
		}

		return false;
	}
	std::unique_ptr<QMenu> GetContextMenu(glm::vec2) override { return nullptr; }
};


struct Matrices
{
	glm::mat4 u_projection;
	glm::mat4 u_camera;
	glm::ivec4 u_screenSize;
	float      u_ctime;
	float      u_zoom;
	float	   u_pad[2];
};

GLViewWidget::GLViewWidget(QWidget * p) :
	gl::ViewWidget(p)
{
}

GLViewWidget::~GLViewWidget()
{
	glDeleteTextures(1, &permeabilities);
}

void GLViewWidget::SetMainWindow(MainWindow * w)
{
	this->w = w;
	_parent.reset(new MapParent(w));
}

void GLViewWidget::initializeGL()
{
	gl::ViewWidget::initializeGL();

	_shaders = std::make_shared<Shaders>(this);

    glClearColor(0, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenBuffers(1, &m_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrices), nullptr, GL_DYNAMIC_DRAW);

	glGenTextures(1, &permeabilities);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_1D, permeabilities);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);

	w->loadDefaultWalls();
}

void GLViewWidget::upload_permeabilitys(uint8_t * table, int size)
{
	makeCurrent();

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_1D, permeabilities);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, size, 0, GL_RED, GL_UNSIGNED_BYTE, &table[0]);
	glActiveTexture(GL_TEXTURE0);
}

void GLViewWidget::paintGL()
{
    if(w->document == nullptr)
		return;

    if(w->document->m_metaroom._selection.Changed()) w->OnSelectionChanged();

	w->SetStatusBarMessage(GetWorldPosition());

	makeCurrent();

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_1D, permeabilities);
	glActiveTexture(GL_TEXTURE0);

	int width = size().width();
	int height = size().height();

	glViewport(0, 0, width, height);

	Matrices mat;

	mat.u_projection = glm::ortho(
		(float)-width/2,
		(float) width/2,
		(float)-height/2,
		(float) height/2,
		(float)-1,
		(float)+1);

	auto window_pos = mapToGlobal(QPoint());
	
	mat.u_camera = glm::mat4(1);
    mat.u_camera = glm::scale(mat.u_camera, glm::vec3(w->GetZoom()));
	mat.u_camera[3] =   mat.u_camera * glm::vec4(-w->document->GetScreenCenter(), 0, 1);

	mat.u_screenSize = glm::ivec4(width, height, window_pos.x(), window_pos.y());

	long long time =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - current_time
				).count();

	mat.u_ctime = time / 1000;
	mat.u_zoom  = w->GetZoom();
	mat.u_pad[0]  = w->document->m_background?
				w->document->m_background->pxPerMeter() :
				256;
	mat.u_pad[1] = w->viewGrid();

	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrices), &mat);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);

	_shaders->transparencyShader.Bind(this);
	_shaders->defaultVaos.Bind(this);
	_shaders->defaultVaos.RenderSquare(this);

	auto displayRange = w->ui->backgroundDepthSlider;

	auto range = glm::uvec2(
		displayRange->GetLowerValue() * 65535 / 256,
		displayRange->GetUpperValue() * 65535 / 256
	);

	w->document->RenderBackground(_shaders.get(), range);
	w->document->m_metaroom.gl.Render(_shaders.get(), -1);

	w->toolbox.Render(_shaders.get());
	
//	auto center = glm::vec4(w->document->GetScreenCenter(), 0, 1);
//	auto transformed = mat.u_camera * center;
	
	_shaders->mouseShader(this, GetWorldPosition());
	_shaders->mouseShader(this, w->document->GetScreenCenter(), 5, glm::vec4(0, 1, 0, 1));
}
