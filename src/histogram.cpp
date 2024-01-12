#include "histogram.h"
#include "src/backgroundimage.h"
#include "src/document.h"
#include <QOpenGLFunctions_4_5_Core>
#include "src/Shaders/shaders.h"
#include "gl/renderdoc.h"

void Histogram::Initialize(QOpenGLFunctions* gl)
{
	glm::uvec4 default_value{65536};
	std::vector<uint32_t> buffer;
	buffer.resize(65536);
	for(auto i = 0u; i < buffer.size(); ++i) buffer[i] = i;

	gl->glGenVertexArrays(1, &_vao);
	gl->glCreateBuffers(Total, _buffers);
	gl->glCreateTextures(GL_TEXTURE_BUFFER, 1, &_texture);

	gl->glNamedBufferStorage(_buffers[HistogramBuffer], sizeof(uint32_t)*65536, buffer.data(), GL_DYNAMIC_STORAGE_BIT);
	gl->glNamedBufferStorage(_buffers[HistogramUBO], sizeof(uint32_t)*4, &default_value,  GL_DYNAMIC_STORAGE_BIT);
	gl->glTextureBuffer(_texture, GL_R32UI, _buffers[HistogramBuffer]);
}

void Histogram::Destroy(QOpenGLFunctions* gl)
{
	gl->glDeleteVertexArrays(1, &_vao);
	gl->glDeleteTextures(1, &_texture);
	gl->glDeleteBuffers(Total, _buffers);
}

void Histogram::clear(QOpenGLFunctions* gl)
{
	_histogramDirty = true;
	gl->glClearNamedBufferData(_buffers[HistogramBuffer], GL_R32UI, GL_RED, GL_UNSIGNED_INT, nullptr);
	gl->glClearNamedBufferData(_buffers[HistogramUBO], GL_R32UI, GL_RED, GL_UNSIGNED_INT, nullptr);

}

void Histogram::Update(Shaders * shaders, Document * document, glm::ivec4 AABB)
{
	RenderDocCaptureRAII raii("Compute Histogram", true);

	auto gl = shaders->gl;

	clear(gl);

	if(document->m_background == nullptr)
		return;

	auto depth = document->m_background->depth();
	auto depthBuffer = document->m_background->depthTileBuffer();
	auto totalTiles = document->m_background->noTiles();

	if(depth == nullptr)
		return;

	assert(totalTiles != 0);

	glm::ivec2 quad[4];
	quad[0] = glm::ivec2(AABB.x, AABB.y);
	quad[1] = glm::ivec2(AABB.z, AABB.y);
	quad[2] = glm::ivec2(AABB.z, AABB.w);
	quad[3] = glm::ivec2(AABB.x, AABB.w);

	std::vector<glm::ivec2> tasks{256};
	for(auto i = 0u; i < tasks.size(); ++i)
		tasks[i] = {i, 0};

	uint32_t buffers[2];
	gl->glCreateBuffers(2, buffers);

	gl->glNamedBufferStorage(buffers[0], sizeof(quad), quad, 0);
	gl->glNamedBufferStorage(buffers[1], sizeof(tasks[0])*tasks.size(), tasks.data(), 0);

	for(auto i = 0u; i < depth.size(); ++i)
	{
		uint16_t tilesInBatch = std::min<int>(256, totalTiles - i*256);

		shaders->histogram.compute(gl, ComputeHistogram::TasksToHistogram::Settings{
			.texture=depth[i],
			.histogram=_buffers[HistogramBuffer],
			.quads={
			   .buffer=buffers[0],
			   .offset=0,
			   .size=1
			},
		   .tasks={
			  .buffer=buffers[1],
			  .offset=0,
			  .size=tilesInBatch
		   },
		   .tiles={
			  .buffer=depthBuffer,
			  .offset=0,
			  .size=256,
		   },
		});
	}

	gl->glDeleteBuffers(2, buffers);
}

void Histogram::Update(Shaders * shaders, Document * document)
{
	return;
}

void Histogram::operator()(Shaders * shaders, glm::uvec2 input_range, uint32_t output_width)
{
	RenderDocCaptureRAII raii("Histogram", true);

#if 1
	if(input_range != _inputRange)
	{
		_inputRange = input_range;
		_histogramDirty = true;
	}

	if(_histogramDirty)
	{
		_histogramDirty = false;
		shaders->histogram.computeBounds(shaders->gl, _buffers[HistogramBuffer], _buffers[HistogramUBO], _inputRange);
	}
#endif



	shaders->histogram.display(shaders->gl,
		ComputeHistogram::DisplayHistogram::Settings{
		   .vao = _vao,
		   .histogramTexture=_texture,
		   .histogramMaxValue=_buffers[HistogramUBO],
		   .displayRange=input_range,
		   .highlightRange=input_range,
		   .outputWidth=output_width,
	   });
}
