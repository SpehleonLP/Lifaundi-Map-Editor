#include "simpleshaderbase.h"
#include <QOpenGLFunctions_4_5_Core>
#include <loguru.hpp>

//---------------------------------------------------
//		SHADER BASE
// --------------------------------------------------
ShaderBase::~ShaderBase()
{
	assert(m_program == 0);
}

void ShaderBase::Destroy(QOpenGLFunctions * gl)
{
	if(m_program)
	{
		gl->glUseProgram(0);

		gl->glDetachShader(m_program, m_vertex);
		if(m_geometry) gl->glDetachShader(m_program, m_geometry);
		if(m_fragment) gl->glDetachShader(m_program, m_fragment);

		gl->glDeleteProgram(m_program);
		gl->glDeleteShader(m_vertex);
		if(m_geometry) gl->glDeleteShader(m_geometry);
		if(m_fragment) gl->glDeleteShader(m_fragment);

		m_program = 0;
		m_vertex   = 0;
		m_fragment = 0;
		m_geometry = 0;
	}
}

uint32_t ShaderBase::PrepareShader(QOpenGLFunctions * gl, const char * class_name, ShaderSource & Source, const char * type, uint32_t mode,  bool & failed)
{
	uint32_t r = 0;
	
	if(Source.sources)
	{
		r = gl->glCreateShader(mode);

		GLint success = 0;
		GLint length = 0;

		gl->glShaderSource(r, Source.length, Source.sources, Source.totals);
		gl->glCompileShader(r);

		gl->glGetShaderiv(r, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE)
		{
			std::string error;

			gl->glGetShaderiv(r, GL_INFO_LOG_LENGTH, &length);
			error.resize(length, 0);
			gl->glGetShaderInfoLog(r, length, &length, error.data());

			qWarning("%s.%s %.*s", class_name, type, length, error.data());
			failed = true;
		}
	}	
	
	return r;
};

bool ShaderBase::Prepare(QOpenGLFunctions * gl, const char * class_name, ShaderSource && Vertex, ShaderSource && TesselationControl, ShaderSource && Tesselation, ShaderSource && Geometry, ShaderSource && Fragment)
{
	m_vertex = gl->glCreateShader(GL_VERTEX_SHADER);
	
	bool failed = false;	
	if((TesselationControl.sources == nullptr) != (Tesselation.sources == nullptr))
	{
		LOG_F(ERROR, "%s failed: both tesselation sources must be provided if one is.", class_name);
		failed = true;
	}
	
	m_vertex = PrepareShader(gl, class_name, Vertex, "vertex", GL_VERTEX_SHADER, failed);
	m_tesselationControl = PrepareShader(gl, class_name, TesselationControl, "tesselation control", GL_TESS_CONTROL_SHADER, failed);
	m_tesselation = PrepareShader(gl, class_name, Tesselation, "tesselation", GL_TESS_EVALUATION_SHADER, failed);
	m_geometry = PrepareShader(gl, class_name, Geometry, "geometry", GL_GEOMETRY_SHADER, failed);
	m_fragment = PrepareShader(gl, class_name, Fragment, "fragment", GL_FRAGMENT_SHADER, failed);

	if(failed == true)
		return false;

	m_program = gl->glCreateProgram();

	gl->glAttachShader(m_program, m_vertex);
	
	if(TesselationControl.sources) gl->glAttachShader(m_program, m_tesselationControl);
	if(Tesselation.sources) gl->glAttachShader(m_program, m_tesselation);
	if(Geometry.sources) gl->glAttachShader(m_program, m_geometry);
	if(Fragment.sources) gl->glAttachShader(m_program, m_fragment);

	return true;
}

void ShaderBase::attribute(QOpenGLFunctions * gl, uint32_t index, const char * name)
{
	gl->glBindAttribLocation(m_program, index, name);
}

void ShaderBase::uniform(QOpenGLFunctions * gl, int32_t & index, const char * name)
{
	index = gl->glGetUniformLocation(m_program, name);
}

void ShaderBase::uniform(QOpenGLFunctions * gl, int16_t & index, const char * name)
{
	index = gl->glGetUniformLocation(m_program, name);
}

void ShaderBase::uniform(QOpenGLFunctions * gl, int8_t & index, const char * name)
{
	index = gl->glGetUniformLocation(m_program, name);
}

//---------------------------------------------------
//		COMPUTE SHADER BASE
// --------------------------------------------------

ComputeShaderBase::~ComputeShaderBase()
{
	assert(m_program == 0);
}

void ComputeShaderBase::Destroy(QOpenGLFunctions * gl)
{
	gl->glUseProgram(0);

	if(m_program)
	{
		gl->glDetachShader(m_program, m_compute);
		gl->glDeleteProgram(m_program);
		gl->glDeleteShader(m_compute);

		m_program = 0;
		m_compute  = 0;
	}
}

bool ComputeShaderBase::Prepare(QOpenGLFunctions * gl, const char * class_name, ShaderSource && Compute)
{
	m_compute = gl->glCreateShader(GL_COMPUTE_SHADER);

	GLint success = 0;
	GLint length = 0;

	gl->glShaderSource(m_compute, Compute.length, Compute.sources, Compute.totals);
	gl->glCompileShader(m_compute);

	gl->glGetShaderiv(m_compute, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		std::string error;

		gl->glGetShaderiv(m_compute, GL_INFO_LOG_LENGTH, &length);
		error.resize(length, 0);
		gl->glGetShaderInfoLog(m_compute, length, &length, error.data());

		qWarning("%s: %.*s", class_name, length, error.data());

		throw std::runtime_error(error);
	}

	m_program = gl->glCreateProgram();

	gl->glAttachShader(m_program, m_compute);
	return  OpenGL_LinkProgram(gl, class_name, m_program, false);
}


void ComputeShaderBase::uniform(QOpenGLFunctions * gl, int32_t & index, const char * name)
{
	index = gl->glGetUniformLocation(m_program, name);
}

void ComputeShaderBase::uniform(QOpenGLFunctions * gl, int16_t & index, const char * name)
{
	index = gl->glGetUniformLocation(m_program, name);
}

void ComputeShaderBase::uniform(QOpenGLFunctions * gl, int8_t & index, const char * name)
{
	index = gl->glGetUniformLocation(m_program, name);
}

void glBindUniformBlocks(QOpenGLFunctions_4_5_Core * gl, GLuint program);

bool OpenGL_LinkProgram(ShaderBase::QOpenGLFunctions * gl, const char * class_name, uint32_t program, bool delay_validate)
{
	auto print_log = [gl, class_name, program](GLint status)
	{
		if (status == GL_FALSE)
		{
			GLint length;
			gl->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

			if(length)
			{
				std::string log(length, 0);
				gl->glGetProgramInfoLog(program, length, &length, log.data());
				qWarning("%s: %.*s", class_name, length, log.data());
			}

			return false;
		}

		return true;
	};

	GLint status;

	gl->glLinkProgram(program);
	gl->glGetProgramiv(program, GL_LINK_STATUS, &status);

	if(!print_log(status))
		return false;

	glBindUniformBlocks(gl, program);

	if(delay_validate == false)
	{
		gl->glValidateProgram(program);
		gl->glGetProgramiv(program, GL_VALIDATE_STATUS, &status);

		if(!print_log(status))
			return false;
	}

	return true;
}




