#include "Funcs.hpp"
#include "Common/Log.hpp"
#include <SDL.h>

namespace Reactor::Gl
{
	namespace
	{
		thread_local Common::LogRealmHandle lrh("main");
	}

#define REACTOR_GLFUNC_DEFINE(ret, name, ...) ret (*name)(__VA_ARGS__) = nullptr;
	REACTOR_GLFUNC_LIST(REACTOR_GLFUNC_DEFINE)
#undef REACTOR_GLFUNC_DEFINE

	void LoadGlFuncs()
	{
#define REACTOR_GLFUNC_LOAD(ret, name, ...) if (!(name = reinterpret_cast<ret (*)(__VA_ARGS__)>(SDL_GL_GetProcAddress(#name)))) lrh.Die("SDL_GL_GetProcAddress(\"" #name "\") failed");
		REACTOR_GLFUNC_LIST(REACTOR_GLFUNC_LOAD)
#undef REACTOR_GLFUNC_LOAD
	}

	void UnloadGlFuncs()
	{
#define REACTOR_GLFUNC_UNLOAD(ret, name, ...) name = nullptr;
		REACTOR_GLFUNC_LIST(REACTOR_GLFUNC_UNLOAD)
#undef REACTOR_GLFUNC_UNLOAD
	}
}
