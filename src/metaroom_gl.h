#ifndef METAROOM_GL_H
#define METAROOM_GL_H
#include "src/indexbuffers.h"
#include <cstdint>

class GLViewWidget;
class Metaroom;

class MetaroomGL
{
public:
	MetaroomGL(Metaroom & me);
	~MetaroomGL();

	void Release(GLViewWidget *gl);
	void Render(GLViewWidget *gl, int selected_door_type);
	void Prepare(GLViewWidget* gl);

	void SetDirty() const { _dirty = true; }
	void SetIndicesDirty() const { _dirty = true; _indicesDirty = true; }

	enum Buffers
	{
		bVertices=0,
		bHalls=2,
		bGravity = 3,
		NoBuffers,

	};

	enum Arrays
	{
		aRooms = 0,
		aHalls = 2,
		aGravity,
		NoArrays,

	};

private:
	mutable bool _dirty{true};
	mutable bool _indicesDirty{true};

	Metaroom & me;

	IndexBuffers      _indices;

	uint32_t gl_vert_texture{};
	uint32_t gl_vert_vbo{};

	uint32_t _vao[NoArrays]{};
	uint32_t _buffers[NoBuffers]{};

	uint32_t _glAlloced{};
	uint32_t _noHalls{};
};

#endif // METAROOM_GL_H
