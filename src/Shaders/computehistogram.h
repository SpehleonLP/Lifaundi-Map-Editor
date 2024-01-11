#ifndef COMPUTEHISTOGRAM_H
#define COMPUTEHISTOGRAM_H
#include "qt-gl/simpleshaderbase.h"


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
			Buffer   quads;		// 1
			Buffer	 tasks;		// 2
			Buffer	 tiles;		// 3
			uint32_t histogram;
		};

		void operator()(QOpenGLFunctions * gl, Settings const&);
		void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);
	} tasksToHistogram;


	struct HistogramBounds  : public ComputeShaderBase
	{
	#ifndef PRODUCTION_BUILD
		static const char Source[];
	#endif

		void operator()(QOpenGLFunctions * gl, uint32_t histogram, uint32_t output);
		void Initialize(QOpenGLFunctions * gl, CompressedShaderSource &);
	} histogramBounds;



};

#endif // COMPUTEHISTOGRAM_H
