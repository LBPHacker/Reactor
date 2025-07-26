#include "Shader.hpp"
#include "Common/Log.hpp"
#include "Gl/Funcs.hpp"
#include <string>

namespace Reactor::Gl
{
	namespace
	{
		thread_local Common::LogRealmHandle lrh("shader");
	}

	GLuint Shader::Ctor(GLenum shaderType)
	{
		return glCreateShader(shaderType);
	}

	void Shader::Dtor(GLuint name)
	{
		glDeleteShader(name);
	}

	void Shader::Source(std::string source)
	{
		const GLchar *data = source.data();
		GLint length = source.size();
		glShaderSource(ptr, 1, &data, &length);
	}

	void Shader::Compile()
	{
		glCompileShader(ptr);
		int status;
		glGetShaderiv(ptr, GL_COMPILE_STATUS, &status);
		{
			int infoLogLength;
			glGetShaderiv(ptr, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::string infoLog(infoLogLength, '?');
			glGetShaderInfoLog(ptr, infoLogLength, nullptr, infoLog.data());
			if (status != GL_TRUE)
			{
				lrh.Die("glCompileShader failed: ", infoLog);
			}
			lrh("info log: ", infoLog);
		}
	}
}
