#ifndef INDEXBUFFERS_H
#define INDEXBUFFERS_H
#include <cstdint>

class GLViewWidget;

class IndexBuffers
{
public:
	IndexBuffers() = default;
	~IndexBuffers();

    void Release(GLViewWidget * gl);
    void Prepare(GLViewWidget *gl, uint32_t size);

	uint32_t EdgeVBO() const { return m_vbo[0]; }
	uint32_t FaceVBO() const { return m_vbo[1]; }

private:
	uint32_t m_allocated{};
	uint32_t m_vbo[2]{};
};

#endif // INDEXBUFFERS_H
