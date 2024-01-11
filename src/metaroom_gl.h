#ifndef METAROOM_GL_H
#define METAROOM_GL_H
#include "src/indexbuffers.h"
#include <cstdint>

class Shaders;
class Metaroom;
struct Arrow;

class MetaroomGL
{
public:
	MetaroomGL(Metaroom & me);
	~MetaroomGL();

	void Release(Shaders * shaders);
	void Render(Shaders * shaders, int selected_door_type);
	void Prepare(Shaders * gl);

	void SetDirty() const { _dirty = true; }
	void SetIndicesDirty() const { _dirty = true; _indicesDirty = true; }

private:
	mutable bool _dirty{true};
	mutable bool _indicesDirty{true};

	Metaroom & me;

	IndexBuffers     _indices;

	uint32_t gl_vert_texture{};
	uint32_t gl_vert_vbo{};

	uint32_t _vao[2]{};
	uint32_t _buffers{};

	uint32_t _glAlloced{};

	std::vector<Arrow>	_arrows;
};

#endif // METAROOM_GL_H
