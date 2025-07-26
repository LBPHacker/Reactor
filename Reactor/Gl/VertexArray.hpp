#pragma once
#include "Common/HandlePtr.hpp"
#include "Gl/Funcs.hpp"

namespace Reactor::Gl
{
	class Program;
	class Buffer;

	class VertexArray : public Common::HandlePtr<VertexArray, GLuint, 0>
	{
		static VertexArray *&GetBoundInstance()
		{
			static VertexArray *boundInstance = nullptr;
			return boundInstance;
		}

		struct ScopedBindingTag
		{
		};

	public:
		static GLuint Ctor(std::in_place_t);
		static void Dtor(GLuint name);

		using HandlePtr::HandlePtr;

		struct ScopedBinding
		{
			VertexArray *prev;

			ScopedBinding(ScopedBindingTag, VertexArray *newPrev) : prev(newPrev)
			{
			}
			ScopedBinding(const ScopedBinding &) = delete;
			ScopedBinding &operator =(const ScopedBinding &) = delete;

			~ScopedBinding();
		};
		ScopedBinding Bind();
	};
}
