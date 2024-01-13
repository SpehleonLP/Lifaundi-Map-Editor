#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#include <cstdint>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class QOpenGLFunctions_4_5_Core;
class Shaders;
class Document;

class Histogram
{
public:
typedef class QOpenGLFunctions_4_5_Core QOpenGLFunctions;
	void Initialize(QOpenGLFunctions*);
	void Destroy(QOpenGLFunctions*);

	void clear(QOpenGLFunctions* gl);
	void operator()(Shaders * shaders, glm::uvec2 input_range, uint32_t output_width);

	void Update(Shaders * shaders, Document * document, glm::ivec4 AABB);
	void Update(Shaders * shaders, Document * document);

private:
	enum Buffers
	{
		HistogramBuffer,
		HistogramUBO,
		HistogramRawRange,
		Total,
	};

// render patches
	uint32_t _vao{};
	uint32_t _buffers[Total]{};
	uint32_t _texture{};

	bool	   _histogramDirty{};
	glm::uvec2 _inputRange{};
};

#endif // HISTOGRAM_H
