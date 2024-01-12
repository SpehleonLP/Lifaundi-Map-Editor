#ifndef COMPUTEHISTOGRAM_H
#define COMPUTEHISTOGRAM_H
#include "qt-gl/simpleshaderbase.h"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class ComputeHistogram  : public ComputeShaderBase
{
public:
	void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);
	void Destroy(QOpenGLFunctions * gl);

	struct TasksToHistogram  : public ComputeShaderBase
	{
	#ifndef PRODUCTION_BUILD
		static const char Source[];
	#endif

		struct Buffer
		{
			uint32_t buffer;
			uint16_t offset;
			uint16_t size;
		};

		struct Settings
		{
			uint32_t texture;	// 0
			uint32_t histogram;
			Buffer   quads;		// 1
			Buffer	 tasks;		// 2
			Buffer	 tiles;		// 3
		};

		void operator()(QOpenGLFunctions * gl, Settings const&);
		void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);
	} compute;


	struct HistogramBounds  : public ComputeShaderBase
	{
	#ifndef PRODUCTION_BUILD
		static const char Source[];
	#endif

		void operator()(QOpenGLFunctions * gl, uint32_t histogram, uint32_t output, glm::uvec2 task_range);
		void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);
	} computeBounds;


	struct DisplayHistogram : public ShaderBase
	{
	#ifndef PRODUCTION_BUILD
		static const char Vertex[];
		static const char Fragment[];
	#endif

		struct Settings
		{
			uint32_t vao;
			uint32_t histogramTexture;
			uint32_t histogramMaxValue;
			glm::uvec2 displayRange;
			glm::uvec2 highlightRange;
			uint32_t   outputWidth;
		};

		void operator()(QOpenGLFunctions * gl, Settings const& settings);

		void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);
		void Destory(QOpenGLFunctions * gl);

	private:
		uniform_t u_inputArea;
		uniform_t u_outputArea;
	} display;
};

#endif // COMPUTEHISTOGRAM_H
