#include "ProgramBindings.hpp"
#include "Gl/Funcs.hpp"
#include "Common/Log.hpp"
#include "Buffer.hpp"
#include "Program.hpp"

namespace Reactor::Gl
{
	namespace
	{
		thread_local Common::LogRealmHandle lrh("programbindings");
	}

	void ProgramBindings::DeclareStorage(std::string name)
	{
		if (storageNames.find(name) != storageNames.end())
		{
			lrh.Die("storage named ", name, " already declared");
		}
		storageNames.insert({ std::string(name), GLuint(storageNames.size()) });
	}

	GLuint ProgramBindings::GetStorageBinding(std::string_view name) const
	{
		auto it = storageNames.find(name);
		if (it == storageNames.end())
		{
			lrh.Die("no storage named ", name, " declared");
		}
		return it->second;
	}

	void ProgramBindings::SetProgramBindings(Program &program)
	{
		for (auto &[ name, binding ] : storageNames)
		{
			auto index = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name.c_str());
			if (index != GL_INVALID_INDEX)
			{
				lrh(this, ": binding resource ", name, " of program ", &program, " to point ", binding);
				glShaderStorageBlockBinding(program, index, binding);
			}
		}
	}

	void ProgramBindings::BindStorageBuffer(std::string_view name, Buffer &buffer)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GetStorageBinding(name), buffer);
	}
}
