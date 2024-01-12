#ifndef SHADERS_H
#define SHADERS_H
#include "src/Shaders/arrowshader.h"
#include "src/Shaders/blitshader.h"
#include "src/Shaders/defaulttextures.h"
#include "src/Shaders/defaultvaos.h"
#include "src/Shaders/doorshader.h"
#include "src/Shaders/mouseshader.h"
#include "src/Shaders/roomoutlineshader.h"
#include "src/Shaders/selectedroomshader.h"
#include "src/Shaders/transparencyshader.h"
#include "src/Shaders/uniformcolorshader.h"
#include "src/Shaders/computehistogram.h"

class QOpenGLFunctions_4_5_Core;

class Shaders
{
public:
	Shaders(QOpenGLFunctions_4_5_Core * gl);
	~Shaders();

	QOpenGLFunctions_4_5_Core *const gl;

	ArrowShader arrowShader;
	BlitShader  blitShader;
	DoorShader	doorShader;
	MouseShader mouseShader{defaultVaos};
	RoomOutlineShader roomOutlineShader;
	SelectedRoomShader selectedRoomShader;
	TransparencyShader transparencyShader;
	UniformColorShader uniformColorShader;

	ComputeHistogram	histogram;

	DefaultTextures		defaultTextures;
	DefaultVAOs			defaultVaos;

private:
};

#endif // SHADERS_H
