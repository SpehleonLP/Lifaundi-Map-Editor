#include "defaulttextures.h"
#include <atomic>
#include <stdexcept>
#include <cstring>
#include "src/glviewwidget.h"
#include <iostream>

DefaultTextures::~DefaultTextures()
{
	if(textures[0] !=  0)
        std::cerr << "Default textures not cleaned up..." << std::endl;
}

uint32_t DefaultTextures::GetWhiteTexture(GLViewWidget* gl)
{
	if(textures[WhiteTexture] == 0)
        createTextures(gl);

	return textures[WhiteTexture];
}

uint32_t DefaultTextures::GetNormalTexture(GLViewWidget* gl)
{
	if(textures[NormalTexture] == 0)
        createTextures(gl);

	return textures[NormalTexture];
}

uint32_t DefaultTextures::GetNoiseTexture(GLViewWidget* gl)
{
	if(textures[NoiseTexture] == 0)
        createTextures(gl);

	return textures[NoiseTexture];

}

void DefaultTextures::createTextures(GLViewWidget* gl)
{
	if(refCount.load() == 0)
		throw std::runtime_error("Tried to create default textures without incing refcount...");

    gl->glGenTextures(TotalTextures, textures);

	uint32_t pixels[TotalTextures] =
	{
		0xFFFFFFFF,
		0x8080FFFF,
		0x7D80807D
	};

	for(int i = 0; i < TotalTextures; ++i)
	{
        gl->glBindTexture(GL_TEXTURE_2D, textures[i]);

        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        gl->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        gl->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        gl->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	}

	for(int i = 0; i < 2; ++i)
	{
		gl->glBindTexture(GL_TEXTURE_2D, textures[i]);

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

	gl->glBindTexture(GL_TEXTURE_2D, textures[2]);

	gl->glTexImage2D(
		GL_TEXTURE_2D,  //target
		0,	//level
		GL_RED, //internal format
		2, //width
		2, //height,
		0, //border must be 0
		GL_RED, //format of incoming source
		GL_UNSIGNED_BYTE, //specific format
		&textures[2]);


	std::unique_ptr<uint8_t[]> noise(new uint8_t[512*512]);

	for(int i = 0; i < 512*512; ++i)
		noise[i] = rand() & 0xFF;

	gl->glBindTexture(GL_TEXTURE_2D, textures[3]);

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

void DefaultTextures::destroyTextures(GLViewWidget* gl)
{
    gl->glDeleteTextures(TotalTextures, textures);
	memset(textures, 0, sizeof(textures));
}
