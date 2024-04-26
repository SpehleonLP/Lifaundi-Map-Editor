#include "defaulttextures.h"
#include <cstring>
#include <QOpenGLFunctions_4_5_Core>
#include <memory>

DefaultTextures::~DefaultTextures()
{
	if(_textures[0] !=  0)
		qFatal("Default textures not cleaned up...");
}

void DefaultTextures::Destroy(QOpenGLFunctions * gl)
{
	gl->glDeleteTextures(TotalTextures, _textures);
	memset(_textures, 0, sizeof(_textures));
}

void DefaultTextures::Initialize(QOpenGLFunctions * gl)
{
	gl->glGenTextures(TotalTextures, _textures);

	uint32_t pixels[TotalTextures] =
	{
		0xFFFFFFFF,
		0x8080FFFF,
		0x7D80807D
	};

	for(int i = 0; i < TotalTextures; ++i)
	{
		gl->glBindTexture(GL_TEXTURE_2D, _textures[i]);

        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        gl->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        gl->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	}

	for(int i = 0; i < 2; ++i)
	{
		gl->glBindTexture(GL_TEXTURE_2D, _textures[i]);

        gl->glTexImage2D(
			GL_TEXTURE_2D,  //target
			0,	//level
			GL_RGBA, //internal format
			1, //width
			1, //height,
			0, //border must be 0
			GL_RGBA, //format of incoming source
			GL_UNSIGNED_BYTE, //specific format
			&pixels[i]);
	}

	gl->glBindTexture(GL_TEXTURE_2D, _textures[2]);

	gl->glTexImage2D(
		GL_TEXTURE_2D,  //target
		0,	//level
		GL_RED, //internal format
		2, //width
		2, //height,
		0, //border must be 0
		GL_RED, //format of incoming source
		GL_UNSIGNED_BYTE, //specific format
		&pixels[2]);


	std::unique_ptr<uint8_t[]> noise(new uint8_t[512*512]);

	for(int i = 0; i < 512*512; ++i)
		noise[i] = rand() & 0xFF;

	gl->glBindTexture(GL_TEXTURE_2D, _textures[3]);

	gl->glTexImage2D(
		GL_TEXTURE_2D,  //target
		0,	//level
		GL_RED, //internal format
		512, //width
		512, //height,
		0, //border must be 0
		GL_RED, //format of incoming source
		GL_UNSIGNED_BYTE, //specific format
		&noise[0]);

}
