#ifndef SIMPLESHADERBASE_H
#define SIMPLESHADERBASE_H
#include <string_view>
#include <cstdint>

class QOpenGLFunctions_4_5_Core;

#define VERSION(x) "#version "#x
#undef TEXT
#define TEXT(k)  #k "\n"
#define SHADER(v, k)  VERSION(v) "\n" #k "\n"
#define COMPUTE_HEADER_x(x) "#version 450\n" "layout(local_size_x = " #x ", local_size_y = 1, local_size_z = 1) in;\n"
#define COMPUTE_HEADER_xy(x, y) "#version 450\n" "layout(local_size_x = " #x ", local_size_y = " #y ", local_size_z = 1) in;\n"
#define COMPUTE_HEADER_xyz(x, y, z) "#version 450\n" "layout(local_size_x = " #x ", local_size_y = " #y ", local_size_z = " #z ") in;\n"

class CompressedShaderSource;
class GLSL_shader_source;

struct ShaderBase
{
	typedef QOpenGLFunctions_4_5_Core QOpenGLFunctions;

	typedef int16_t uniform_t;
	typedef ShaderBase super;
	~ShaderBase();

	uint32_t Program() const { return m_program; }
	void Destroy(QOpenGLFunctions *);

	struct ShaderSource
	{
		ShaderSource() = default;
		ShaderSource(std::nullptr_t) {}

		ShaderSource(const char * data)
		{
			buf_raw = data;
			length = 1;
			sources = &buf_raw;
			totals = nullptr;
		}

		ShaderSource(std::string_view const& str)
		{
			if(str.empty())
				return;

			buf_raw = str.data();
			int_data = str.size();

			length = 1;
			sources = &buf_raw;
			totals = &int_data;
		}

		const char * const* sources{};
		int const* totals{};
		int length{};

	private:
		int int_data{};
		char const* buf_raw{};
	};

	auto program() const { return m_program; }

protected:
	__always_inline bool Prepare(QOpenGLFunctions * gl, const char * class_name, ShaderSource && Vertex, ShaderSource && Geometry, ShaderSource && Fragment)
	{ return Prepare(gl, class_name, std::move(Vertex), nullptr, nullptr, std::move(Geometry), std::move(Fragment)); }
	__always_inline bool Prepare(QOpenGLFunctions * gl, const char * class_name, ShaderSource && Vertex) { return Prepare(gl, class_name, std::move(Vertex), nullptr, nullptr); };

	bool Prepare(QOpenGLFunctions * gl, const char * class_name, ShaderSource && Vertex, ShaderSource && TesselationControl, ShaderSource && Tesselation, ShaderSource && Geometry, ShaderSource && Fragment);

	void attribute(QOpenGLFunctions * gl, uint32_t index, const char * name);

	void uniform(QOpenGLFunctions * gl,int32_t & uniform, const char * name);
	void uniform(QOpenGLFunctions * gl,int16_t & uniform, const char * name);
	void uniform(QOpenGLFunctions * gl,int8_t & uniform, const char * name);

	uint32_t m_program{};
	uint32_t m_vertex{};
	uint32_t m_fragment{};
	uint32_t m_geometry{};
	uint32_t m_tesselation{};
	uint32_t m_tesselationControl{};
	
private:
	uint32_t PrepareShader(QOpenGLFunctions * gl, const char * class_name, ShaderSource & Source, const char * type, uint32_t mode,  bool & failed);
};

struct ComputeShaderBase
{
	typedef ShaderBase::QOpenGLFunctions QOpenGLFunctions;

	typedef int16_t uniform_t;
typedef ShaderBase::ShaderSource ShaderSource;
	typedef ComputeShaderBase super;
	~ComputeShaderBase();

	auto program() const { return m_program; }
	void Destroy(QOpenGLFunctions *);

protected:
	bool Prepare(QOpenGLFunctions *, const char * class_name, ShaderSource && Vertex);

	void uniform(QOpenGLFunctions *, int32_t & uniform, const char * name);
	void uniform(QOpenGLFunctions *, int16_t & uniform, const char * name);
	void uniform(QOpenGLFunctions *, int8_t & uniform, const char * name);

	uint32_t m_program{};
	uint32_t m_compute{};
};

bool OpenGL_LinkProgram(ShaderBase::QOpenGLFunctions * gl, const char * class_name, uint32_t program, bool delay_validate = false);

#endif

