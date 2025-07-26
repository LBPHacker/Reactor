#include "Program.hpp"
#include "Common/Log.hpp"
#include "Gl/Funcs.hpp"
#include <glm/mat3x3.hpp>

namespace Reactor::Gl
{
	namespace
	{
		thread_local Common::LogRealmHandle lrh("program");
	}

	GLuint Program::Ctor(std::in_place_t)
	{
		return glCreateProgram();
	}

	void Program::Dtor(GLuint name)
	{
		glDeleteProgram(name);
	}

	Program::ScopedUsage Program::Use()
	{
		auto &usedInstance = GetUsedInstance();
		auto *prev = usedInstance;
		glUseProgram(ptr);
		usedInstance = this;
		return { ScopedUsageTag{}, prev };
	}

	Program::ScopedUsage::~ScopedUsage()
	{
		auto &usedInstance = GetUsedInstance();
		glUseProgram(prev ? prev->ptr : 0);
		usedInstance = prev;
	}

	void Program::AttachShader(GLuint shader)
	{
		glAttachShader(ptr, shader);
	}

	void Program::Link()
	{
		glLinkProgram(ptr);
		int status;
		glGetProgramiv(ptr, GL_LINK_STATUS, &status);
		{
			int infoLogLength;
			glGetProgramiv(ptr, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::string infoLog(infoLogLength, '?');
			glGetProgramInfoLog(ptr, infoLogLength, nullptr, infoLog.data());
			if (status != GL_TRUE)
			{
				lrh.Die("glLinkProgram failed: ", infoLog);
			}
			lrh("info log: ", infoLog);
		}
	}

	GLint Program::GetUniformLocation(std::string_view name)
	{
		auto it = uniformLocation.find(name);
		if (it == uniformLocation.end())
		{
			std::string nameStr(name);
			auto location = glGetUniformLocation(ptr, nameStr.c_str());
			if (location == -1)
			{
				lrh.Die("glGetUniformLocation couldn't find ", nameStr);
			}
			it = uniformLocation.insert({ nameStr, location }).first;
		}
		return it->second;
	}

	template<> void Program::SetUniform<uint32_t   >(std::string_view name, const uint32_t    &value) { glProgramUniform1ui      (ptr, GetUniformLocation(name), value);                     }
	template<> void Program::SetUniform<glm::ivec3 >(std::string_view name, const glm::ivec3  &value) { glProgramUniform3i       (ptr, GetUniformLocation(name), value.x, value.y, value.z); }
	template<> void Program::SetUniform<float      >(std::string_view name, const float       &value) { glProgramUniform1f       (ptr, GetUniformLocation(name), value);                     }
	template<> void Program::SetUniform<glm::mat3x3>(std::string_view name, const glm::mat3x3 &value) { glProgramUniformMatrix3fv(ptr, GetUniformLocation(name), 1, GL_FALSE, &value[0][0]); }
}
