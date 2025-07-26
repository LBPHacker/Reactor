#pragma once
#include "Common/HandlePtr.hpp"
#include "Defs.hpp"
#include <string>
#include <string_view>
#include <map>

namespace Reactor::Gl
{
	class Program : public Common::HandlePtr<Program, GLuint, 0>
	{
		static Program *&GetUsedInstance()
		{
			static Program *userInstance = nullptr;
			return userInstance;
		}

		std::map<std::string, GLint, std::less<>> uniformLocation;

		struct ScopedUsageTag
		{
		};

	public:
		static GLuint Ctor(std::in_place_t);
		static void Dtor(GLuint name);

		using HandlePtr::HandlePtr;

		struct ScopedUsage
		{
			Program *prev;

			ScopedUsage(ScopedUsageTag, Program *newPrev) : prev(newPrev)
			{
			}
			ScopedUsage(const ScopedUsage &) = delete;
			ScopedUsage &operator =(const ScopedUsage &) = delete;

			~ScopedUsage();
		};
		ScopedUsage Use();
		void AttachShader(GLuint shader);
		void Link();
		GLint GetUniformLocation(std::string_view name);

		template<class Value>
		void SetUniform(std::string_view name, const Value &value);
	};
}
