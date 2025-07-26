#pragma once
#include "Common/HandlePtr.hpp"
#include "Gl/Funcs.hpp"

namespace Reactor::Gl
{
	class Buffer : public Common::HandlePtr<Buffer, GLuint, 0>
	{
	public:
		static GLuint Ctor(std::in_place_t)
		{
			GLuint name;
			glCreateBuffers(1, &name);
			return name;
		}

		static void Dtor(GLuint name)
		{
			glDeleteBuffers(1, &name);
		}

		using HandlePtr::HandlePtr;
	};
}
