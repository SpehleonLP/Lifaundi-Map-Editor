#include "computehistogram.h"
#include "gl/compressedshadersource.h"
#include "qt-gl/initialize_gl.h"
#include <QOpenGLFunctions_4_5_Core>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#define COMPUTE_HEADER(x, y) "#version 450\n" "layout(local_size_x = " #x ", local_size_y = " #y ") in;\n"
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
		int z = std::clamp<int>(std::pow(settings.tasks.size - i, 1/3.f), 1, OpenGL.MaxWorkGroupSize[2]);
		int y = std::clamp<int>(std::sqrt((settings.tasks.size - i) / z), 1, OpenGL.MaxWorkGroupSize[1]);
		int x = std::clamp<int>((settings.tasks.size - i)/(y*z), 1, OpenGL.MaxWorkGroupSize[0]);

		gl->glUniform2ui(5, i, settings.tasks.size);
		i += x*y*z;

		gl->glDispatchCompute(x, y, z);
	}
}


// goal here is to get the histogram of the image that intersects the quad
// each tile is a layer of the image2D array
const char ComputeHistogram::TasksToHistogram::Source[] = COMPUTE_HEADER(64, 64)
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


layout(binding=5) uniform uvec2 u_taskRange;

void main() {
// 	linearize Id
	uint WorkGroupID = u_taskRange.x + (gl_WorkGroupID.z * gl_NumWorkGroups.y + gl_WorkGroupID.y) * gl_NumWorkGroups.x + gl_WorkGroupID.x;

	if(WorkGroupID > u_taskRange.y)
		return;

	int tile = u_combinations[WorkGroupID].x;
	int quad = u_combinations[WorkGroupID].y;

	vec2 localPosition = vec2(gl_LocalInvocationID) / vec2(gl_WorkGroupSize);
	ivec2 globalPosition = mix(vec2(u_tilePosition[tile].xy), vec2(u_tilePosition[tile].zw), localPosition);

	for(int itr = 0; itr < 4; ++itr)
	{
		vec2 corner0 = u_vertex[quad*4 + itr];
		vec2 corner1 = u_vertex[quad*4 + ((itr+1) & 0x03)];

// quad is always wound clockwise so can just check against each normal to determine if inside quad
		vec2 normal = vec2(corner0.y - corner1.y, corner1.x - corner0.x);
// do either of these need to be normalized?
		float projection = dot(normal, globalPosition - corner0);
// on wrong side of side ergo not in quad
		if(projection < 0)
			return;
	}

// read depth information
	uint color = imageLoad(s_image, ivec3(gl_LocalInvocationID.xy, tile)).r * 65535;
	atomicAdd(histogram[color], 1);
});

void ComputeHistogram::TasksToHistogram::Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto text = source.Push(Source, sizeof(Source)-1);
#endif

	if(!Prepare(gl, "ComputeHistogram", text))
		return;

}


void ComputeHistogram::HistogramBounds::operator()(QOpenGLFunctions * gl, uint32_t histogram, uint32_t output)
{


}

void ComputeHistogram::HistogramBounds::Initialize(QOpenGLFunctions * gl, CompressedShaderSource & source)
{
#ifdef PRODUCTION_BUILD
	auto vertex = source.pop();
	auto fragment = source.pop();
#else
	auto text = source.Push(Source, sizeof(Source)-1);
#endif

	if(!Prepare(gl, "HistogramBounds", text))
		return;

}
