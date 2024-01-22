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
	gl->glNamedBufferStorage(_buffers[HistogramRawRange], sizeof(uint32_t)*4, &default_value,  GL_DYNAMIC_STORAGE_BIT);
	gl->glTextureBuffer(_texture, GL_R32UI, _buffers[HistogramBuffer]);
}

void Histogram::Destroy(QOpenGLFunctions* gl)
{
	gl->glDeleteVertexArrays(1, &_vao);
	gl->glDeleteTextures(1, &_texture);
	gl->glDeleteBuffers(Total, _buffers);

	_vao = 0;
	_texture = 0;
	memset(_buffers, 0, sizeof(_buffers));
}

void Histogram::clear(QOpenGLFunctions* gl)
{
	_histogramDirty = true;
	gl->glClearNamedBufferData(_buffers[HistogramBuffer], GL_R32UI, GL_RED, GL_UNSIGNED_INT, nullptr);
	gl->glClearNamedBufferData(_buffers[HistogramUBO], GL_R32UI, GL_RED, GL_UNSIGNED_INT, nullptr);
	gl->glClearNamedBufferData(_buffers[HistogramRawRange], GL_R32UI, GL_RED, GL_UNSIGNED_INT, nullptr);

}

void Histogram::Update(Shaders * shaders, Document * document, glm::ivec4 AABB)
{
//	RenderDocCaptureRAII raii("Compute Histogram", true);

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
	quad[0] = glm::ivec2(AABB.z, AABB.w);
	quad[1] = glm::ivec2(AABB.z, AABB.y);
	quad[2] = glm::ivec2(AABB.x, AABB.y);
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
		uint32_t tilesInBatch = std::min<int>(256, totalTiles - i*256);

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
			  .offset=i*256,
			  .size=256,
		   },
		});
	}

	gl->glDeleteBuffers(2, buffers);
}

void Histogram::Update(Shaders * shaders, Document * document)
{
//	RenderDocCaptureRAII raii("Compute Room Histogram", true);

	auto gl = shaders->gl;

	clear(gl);

	if(document->m_background == nullptr
	|| document->m_metaroom.empty()
	|| document->m_metaroom._selection.NoSelectedFaces() == 0)
		return;

	auto depth = document->m_background->depth();
	auto depthBuffer = document->m_background->depthTileBuffer();

	if(depth == nullptr)
		return;

	auto bgHalfSize = document->m_background->dimensions() / 2;
	auto idToTile =document->m_background->idToTile();

	std::vector<std::vector<glm::ivec2>> tasks(depth.size());

	const auto N =  document->m_metaroom._entitySystem.size();
	auto & selection = document->m_metaroom._selection;
	auto no_tiles = document->m_background->tiles();

	for(auto i = 0u; i < N; ++i)
	{
		if(selection.IsFaceSelected(i) == false)
			continue;

		glm::i16vec2  tl, br;
		document->m_metaroom.GetFaceAABB(i, tl, br);

		tl = glm::clamp((glm::ivec2(tl) + bgHalfSize) / 256, glm::ivec2(0), no_tiles);
		br = glm::clamp(((glm::ivec2(br) + bgHalfSize) + 255) / 256, glm::ivec2(0), no_tiles);

		for(auto y = tl.y; y < br.y; ++y)
		{
			for(auto x = tl.x; x < br.x; ++x)
			{
				int j = y * (no_tiles.x) + x;
				auto id = idToTile[j];

				if(id.x < tasks.size())
				{
					tasks[id.x].push_back({id.y, i});
				}
			}
		}
	}

	uint32_t taskBuffer;
	gl->glCreateBuffers(1, &taskBuffer);

	for(auto i = 0u; i < depth.size(); ++i)
	{
		if(tasks[i].empty()) continue;

		gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, taskBuffer);
		gl->glBufferData(GL_SHADER_STORAGE_BUFFER, tasks[i].size() * sizeof(glm::ivec2), tasks[i].data(), GL_STREAM_DRAW);

		uint32_t tasksInBatch =tasks[i].size();
		uint32_t noQuads = document->m_metaroom._verts.size();

		if(tasksInBatch != tasks[i].size())
			throw std::runtime_error("need more bits in buffer!!");

		shaders->histogram.compute(gl, ComputeHistogram::TasksToHistogram::Settings{
			.texture=depth[i],
			.histogram=_buffers[HistogramBuffer],
			.quads={
			   .buffer=document->m_metaroom.gl.GetVertVbo(),
			   .offset=0,
			   .size=noQuads,
			},
		   .tasks={
			  .buffer=taskBuffer,
			  .offset=0,
			  .size=tasksInBatch,
		   },
		   .tiles={
			  .buffer=depthBuffer,
			  .offset=i*256,
			  .size=256,
		   },
		});
	}

	gl->glDeleteBuffers(1, &taskBuffer);
}

void Histogram::operator()(Shaders * shaders, glm::uvec2 input_range, uint32_t output_width)
{
	//RenderDocCaptureRAII raii("Histogram", true);

	if(_histogramDirty)
	{
		shaders->histogram.computeBounds(shaders->gl, _buffers[HistogramBuffer], _buffers[HistogramRawRange], {0, 65536});
	}

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

	shaders->histogram.display(shaders->gl,
		ComputeHistogram::DisplayHistogram::Settings{
		   .vao = _vao,
		   .histogramTexture=_texture,
		   .histogramMaxValue=_buffers[HistogramRawRange],
		   .displayRange={0, 65535},
		   .highlightRange={0, 65535},
		   .outputWidth=output_width,
		   .color=glm::vec4(0.5, 0.5, 0.5, 1)
	   });

	shaders->histogram.display(shaders->gl,
		ComputeHistogram::DisplayHistogram::Settings{
		   .vao = _vao,
		   .histogramTexture=_texture,
		   .histogramMaxValue=_buffers[HistogramUBO],
		   .displayRange=input_range,
		   .highlightRange=input_range,
		   .outputWidth=output_width,
		   .color=glm::vec4(1, 1, 1, 1)
	   });
}
