#include "computehistogram.h"
#include "gl/compressedshadersource.h"
#include "qt-gl/initialize_gl.h"
#include <QOpenGLFunctions_4_5_Core>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

void ComputeHistogram::Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source)
{
	compute.Initialize(gl, source);
	computeBounds.Initialize(gl, source);
	display.Initialize(gl, source);
}

void ComputeHistogram::Destroy(QOpenGLFunctions * gl)
{
	compute.Destroy(gl);
	computeBounds.Destroy(gl);
	display.Destroy(gl);
}

#define COMPUTE_HEADER_xy(x, y) "#version 450\n" "layout(local_size_x = " #x ", local_size_y = " #y ", local_size_z = 1) in;\n"
#define TEXT(k)  #k "\n"

void ComputeHistogram::TasksToHistogram::operator()(QOpenGLFunctions * gl, Settings const& settings)
{
	gl->glUseProgram(program());

	gl->glBindImageTexture(0, settings.texture, 2, GL_TRUE, 0, GL_READ_ONLY, GL_R16);
	gl->glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, settings.quads.buffer, sizeof(glm::ivec2) * 4 * settings.quads.offset, sizeof(glm::ivec2) * 4 * (settings.quads.size));
	gl->glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, settings.tasks.buffer, sizeof(glm::ivec2) * settings.tasks.offset, sizeof(glm::ivec2) * (settings.tasks.size));
	gl->glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, settings.tiles.buffer, sizeof(glm::ivec4) * settings.tiles.offset, sizeof(glm::ivec4) * (settings.tiles.size));
	gl->glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4, settings.histogram, 0, sizeof(uint32_t) * 65536);

	for(auto i = 0u; i < settings.tasks.size; )
	{
		int x = std::clamp<int>((settings.tasks.size - i), 1, OpenGL.MaxWorkGroupSize[0]);
		int y = std::clamp<int>((settings.tasks.size - i) / x, 1, OpenGL.MaxWorkGroupSize[1]);
		int z = std::clamp<int>((settings.tasks.size - i)/(y*x), 1, OpenGL.MaxWorkGroupSize[2]);

		gl->glUniform2ui(5, i, settings.tasks.size);
		i += x*y*z;

		gl->glDispatchCompute(x, y, z);
	}
}


// goal here is to get the histogram of the image that intersects the quad
// each tile is a layer of the image2D array
const char ComputeHistogram::TasksToHistogram::Source[] = COMPUTE_HEADER_xy(16, 16)
TEXT(
layout(r16, binding=0) readonly uniform highp image2DArray s_image;
// vertices of quads, clockwise winding
layout(std430, binding=1) readonly buffer quadBuffer { ivec2 u_vertex[]; };
// x is tile (image array layer); y is quad
layout(std430, binding=2) readonly buffer taskBuffer { ivec2 u_combinations[]; };
// where each tile is drawn in the world
layout(std430, binding=3) readonly buffer tileBuffer { ivec4 u_tilePosition[]; };

// can this be sped up with an intermediary shared buffer?
layout(std430, binding=4) buffer histogramBuffer { uint histogram[]; };

layout(location=5) uniform uvec2 u_taskRange;

bool IsInQuad(int quad, int tile, vec2 localPosition)
{
	vec2 position = vec2(u_tilePosition[tile].xy) + localPosition;

	for(int itr = 0; itr < 4; ++itr)
	{
		vec2 corner0 = u_vertex[quad*4 + itr];
		vec2 corner1 = u_vertex[quad*4 + ((itr+1) & 0x03)];

	// quad is always wound counter clockwise so can just check against each normal to determine if inside quad
		vec2 normal = vec2(corner0.y - corner1.y, corner1.x - corner0.x);
	// do either of these need to be normalized?
		float projection = dot(normal, position - corner0);

	// on wrong side of side ergo not in quad
		if(projection > 0)
			return false;
	}

	return true;
}


void main() {
// 	linearize Id
	uint WorkGroupID = u_taskRange.x + (gl_WorkGroupID.z * gl_NumWorkGroups.y + gl_WorkGroupID.y) * gl_NumWorkGroups.x + gl_WorkGroupID.x;

	if(WorkGroupID > u_taskRange.y)
		return;

	int tile = u_combinations[WorkGroupID].x;
	int quad = u_combinations[WorkGroupID].y;

	for(int i = 0; i < 16; ++i)
	{
		uvec2 coords = gl_LocalInvocationID.xy*4 + uvec2(i % 4, i / 4);
// coords are 16*4 = 64 per meter (b/c reading mip layer 2)
// coordinate space is in 256 per meter
		if(IsInQuad(quad, tile, vec2(coords*4)))
		{
			float depth = imageLoad(s_image, ivec3(coords, tile)).r;

			if(0 < depth && depth < 1)
				atomicAdd(histogram[int(depth * 8192)], 1);
		}
	}

});

void ComputeHistogram::TasksToHistogram::Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto text = source.pop();
#else
	auto text = source.Push(Source, sizeof(Source)-1);
#endif

	if(!Prepare(gl, "ComputeHistogram", text))
		return;

}


// goal here is to the biggest bar in the histogram so we can normalize it while rendering
const char ComputeHistogram::HistogramBounds::Source[] = COMPUTE_HEADER_x(256)
"#define GROUP_SIZE 256\n"
TEXT(
layout(std430, binding=0) readonly buffer histogramBuffer { uint histogram[]; };
layout(std430, binding=1) buffer outputBuffer { uint maxOut; };
layout(location=2) uniform uvec2 u_taskRange;

//shared uint histogramShared[GROUP_SIZE];

void main() {
// 	linearize Id
	const uint globalId =(gl_WorkGroupID.z * gl_NumWorkGroups.y + gl_WorkGroupID.y) * gl_NumWorkGroups.x + gl_WorkGroupID.x;
	const uint localIndex = gl_LocalInvocationID.x;
	const uint threadId = GROUP_SIZE * globalId + localIndex;

#if 1
	if(threadId + u_taskRange.x < u_taskRange.y)
	{
		atomicMax(maxOut, histogram[threadId + u_taskRange.x]);
	}

// would this be better?
#else
	if(threadId + u_taskRange.x < u_taskRange.y)
		histogramShared[localIndex] = histogram[threadId + u_taskRange.x];
	else
		histogramShared[localIndex] = 0;

	barrier();

	for (uint cutoff = (GROUP_SIZE >> 1); cutoff > 0; cutoff >>= 1)
	{
		if (uint(localIndex) < cutoff)
		{
			histogramShared[localIndex] = max(histogramShared[localIndex + cutoff], histogramShared[localIndex]);
		}

		barrier();
	}

	if (localIndex == 0)
	{
		atomicMax(maxOut, histogramShared[0]);
	}
#endif
});

void ComputeHistogram::HistogramBounds::operator()(QOpenGLFunctions * gl, uint32_t histogram, uint32_t output, glm::uvec2 range)
{
	gl->glClearNamedBufferData(output, GL_R32UI, GL_RED, GL_UNSIGNED_INT, nullptr);
	gl->glUseProgram(program());

	gl->glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, histogram, 0, sizeof(uint32_t) * 65536);
	gl->glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, output, 0, 16);
	gl->glUniform2ui(2, range.x/8, range.y/8);
	gl->glDispatchCompute(16, 2, 1);
}

void ComputeHistogram::HistogramBounds::Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto text = source.pop();
#else
	auto text = source.Push(Source, sizeof(Source)-1);
#endif

	if(!Prepare(gl, "HistogramBounds", text))
		return;

}

void ComputeHistogram::DisplayHistogram::operator()(QOpenGLFunctions * gl, Settings const& settings)
{
	gl->glUseProgram(program());

	gl->glBindTextureUnit(0, settings.histogramTexture);
	gl->glBindBufferRange(GL_UNIFORM_BUFFER, 1, settings.histogramMaxValue, 0, 16);

	auto range = settings.displayRange/8u;
	auto highlightRange = settings.highlightRange/8u;

	gl->glUniform1i(0, 0);
	gl->glUniform1f(1, float(range.y - range.x) / settings.outputWidth);
	gl->glUniform1ui(2, settings.outputWidth);
	gl->glUniform2f(3, highlightRange.x, highlightRange.y);
	gl->glUniform2f(4, range.x, range.y);
	gl->glUniform4fv(5, 1, &settings.color.x);

	gl->glBindVertexArray(settings.vao);
	gl->glDrawArrays(GL_LINES, 0, settings.outputWidth*2);
}

const char ComputeHistogram::DisplayHistogram::Vertex[] = SHADER(450,
	layout(std140) uniform HistogramHeight { uint u_maxHeight; };

	layout(location=0) uniform usamplerBuffer u_histogram;
	layout(location=1) uniform float		  u_barWidth;
	layout(location=2) uniform uint			  u_noLines;
	layout(location=3) uniform vec2		      u_highlightRange;
	layout(location=4) uniform vec2		      u_displayRange;
	layout(location=5) uniform vec4			u_color;

	out vec4 _color;

	void main()
	{
// get x coord
		int   line    = gl_VertexID / 2;
		float x_coord = (line / float(u_noLines)) * 2.0 - 1.0;

		_color      = u_color;

		if((gl_VertexID & 0x01) == 0)
		{
			gl_Position = vec4(x_coord, -1, 0, 1);
			return;
		}

		float offset = mix(u_displayRange.x, u_displayRange.y, line / float(u_noLines));
		float weightedSum = 0;

	//	weightedSum += float(texelFetch(u_histogram, int(offset)).r) * (1 - fract(offset));
	//	weightedSum += float(texelFetch(u_histogram, int(offset + u_barWidth)).r) * fract(offset + u_barWidth);

		for(float i = 0; i < u_barWidth; ++i)
		{
			weightedSum = max(weightedSum, texelFetch(u_histogram, int(offset + i)).r);
		//	weightedSum += float(texelFetch(u_histogram, int(offset + i)).r);
		}

		float y_coord = (weightedSum / (float(u_maxHeight)));
		y_coord = log2(y_coord+1);
		gl_Position = vec4(x_coord, y_coord * 2.0 - 1.0, 0, 1.0);
	});


const char ComputeHistogram::DisplayHistogram::Fragment[] = SHADER(450,
	in vec4 _color;
	out vec4 FragColor;

	void main()
	{
		FragColor = _color;
	});

void ComputeHistogram::DisplayHistogram::Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto vertex = source.Push(Vertex, sizeof(Vertex)-1);
	auto fragment = source.Push(Fragment, sizeof(Fragment)-1);
#endif

	if(!Prepare(gl, "DisplayHistogram", vertex, nullptr, fragment))
		return;

	OpenGL_LinkProgram(gl, "DisplayHistogram", program());
}



