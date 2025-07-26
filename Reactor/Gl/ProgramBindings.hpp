#pragma once
#include "Gl/Funcs.hpp"
#include <map>
#include <string>
#include <string_view>

namespace Reactor::Gl
{
	class Program;
	class Buffer;

	class ProgramBindings
	{
		std::map<std::string, GLuint, std::less<>> storageNames;
		GLuint GetStorageBinding(std::string_view name) const;

	public:
		void DeclareStorage(std::string name);
		void SetProgramBindings(Program &program);
		void BindStorageBuffer(std::string_view name, Buffer &buffer);
	};
}
