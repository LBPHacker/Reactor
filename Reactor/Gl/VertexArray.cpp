#include "VertexArray.hpp"
#include "Gl/Funcs.hpp"

namespace Reactor::Gl
{
	GLuint VertexArray::Ctor(std::in_place_t)
	{
		GLuint name;
		glCreateVertexArrays(1, &name);
		return name;
	}

	void VertexArray::Dtor(GLuint name)
	{
		glDeleteVertexArrays(1, &name);
	}

	VertexArray::ScopedBinding VertexArray::Bind()
	{
		auto &boundInstance = GetBoundInstance();
		auto *prev = boundInstance;
		glBindVertexArray(ptr);
		boundInstance = this;
		return { ScopedBindingTag{}, prev };
	}

	VertexArray::ScopedBinding::~ScopedBinding()
	{
		auto &boundInstance = GetBoundInstance();
		glBindVertexArray(prev ? prev->ptr : 0);
		boundInstance = prev;
	}
}
