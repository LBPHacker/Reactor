#pragma once
#include "Common/HandlePtr.hpp"
#include "Defs.hpp"

namespace Reactor::Gl
{
	class Shader : public Common::HandlePtr<Shader, GLuint, 0>
	{
	public:
		static GLuint Ctor(GLenum shaderType);
		static void Dtor(GLuint name);

		using HandlePtr::HandlePtr;

		void Source(std::string source);
		void Compile();
	};
}
