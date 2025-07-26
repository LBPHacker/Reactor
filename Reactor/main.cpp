#include "Scene.hpp"
#include "Movement.hpp"
#include "Common/Log.hpp"
#include "Common/Defer.hpp"
#include "Gl/Funcs.hpp"
#include <utility>
#include <iomanip>
#include <SDL.h>
#include <algorithm>

namespace
{
	using namespace Reactor;
	using namespace Reactor::Common;

	thread_local LogRealmHandle lrh("main");

	constexpr int64_t tickInterval = 1000 / 30;
	constexpr int64_t maxTicksDone = 10;

	constexpr int64_t windowTitleInterval = 1000;

	constexpr float panFactor  = 0.0005f;
	constexpr float rollFactor = 0.02f;
	constexpr float fovFactor  = 2.f;

	void SdlAssertTrue(SDL_bool val, const char *what)
	{
		if (!val)
		{
			lrh.Die(what, " failed: ", SDL_GetError());
		}
	}
#define SdlAssertTrue(x) SdlAssertTrue(x, #x)

	void SdlAssertZero(int val, const char *what)
	{
		if (val)
		{
			lrh.Die(what, " failed: ", SDL_GetError());
		}
	}
#define SdlAssertZero(x) SdlAssertZero(x, #x)

	template<class Thing>
	auto SdlAssertPtr(Thing *ptr, const char *what)
	{
		if (!ptr)
		{
			lrh.Die(what, " failed: ", SDL_GetError());
		}
		return ptr;
	}
#define SdlAssertPtr(x) SdlAssertPtr(x, #x)

	void GlDebugMessageHandler(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *
	)
	{
		lrh.Die(
			"GlDebugMessageHandler called: ",
			"source: "  , source  , ", ",
			"type: "    , type    , ", ",
			"id: "      , id      , ", ",
			"severity: ", severity, ", ",
			"length: "  , length  , ", ",
			"message: " , message 
		);
	}

	void Tick(Movement &movement)
	{
		auto *kbs = SDL_GetKeyboardState(nullptr);
		glm::ivec3 moveAccelWeight = { 0, 0, 0 };
		glm::ivec3 panAccelWeight = { 0, 0, 0 };
		if (kbs[SDL_SCANCODE_W])
		{
			moveAccelWeight.x += 1;
		}
		if (kbs[SDL_SCANCODE_S])
		{
			moveAccelWeight.x -= 1;
		}
		if (kbs[SDL_SCANCODE_SPACE] || kbs[SDL_SCANCODE_F])
		{
			moveAccelWeight.y += 1;
		}
		if (kbs[SDL_SCANCODE_LSHIFT] || kbs[SDL_SCANCODE_X])
		{
			moveAccelWeight.y -= 1;
		}
		if (kbs[SDL_SCANCODE_D])
		{
			moveAccelWeight.z += 1;
		}
		if (kbs[SDL_SCANCODE_A])
		{
			moveAccelWeight.z -= 1;
		}
		if (kbs[SDL_SCANCODE_Z])
		{
			panAccelWeight.x += 1;
		}
		if (kbs[SDL_SCANCODE_C])
		{
			panAccelWeight.x -= 1;
		}
		if (kbs[SDL_SCANCODE_V])
		{
			panAccelWeight.y += 1;
		}
		if (kbs[SDL_SCANCODE_R])
		{
			panAccelWeight.y -= 1;
		}
		if (kbs[SDL_SCANCODE_Q])
		{
			panAccelWeight.z += 1;
		}
		if (kbs[SDL_SCANCODE_E])
		{
			panAccelWeight.z -= 1;
		}
		movement.Tick(moveAccelWeight, panAccelWeight);
	}
}

int main()
{
	SdlAssertZero(SDL_Init(SDL_INIT_VIDEO));
	Defer sdlQuit([]() {
		SDL_Quit();
	});
	SdlAssertTrue(SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1"));
	SdlAssertZero(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS        , SDL_GL_CONTEXT_DEBUG_FLAG));
	SdlAssertZero(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK , SDL_GL_CONTEXT_PROFILE_CORE));
	SdlAssertZero(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4));
	SdlAssertZero(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5));
	SdlAssertZero(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
	SdlAssertZero(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE  , 24));
	SdlAssertZero(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8));
	glm::ivec2 windowSize = { 800, 600 };
	auto windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
	auto *sdlWindow = SdlAssertPtr(SDL_CreateWindow("reactor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowSize.x, windowSize.y, windowFlags));
	Defer destroyWindow([sdlWindow]() {
		SDL_DestroyWindow(sdlWindow);
	});
	auto *glContext = SdlAssertPtr(SDL_GL_CreateContext(sdlWindow));
	Defer destroyGlContext([glContext]() {
		SDL_GL_DeleteContext(glContext);
	});
	Gl::LoadGlFuncs();
	Defer inloadGlFuncs([]() {
		Gl::UnloadGlFuncs();
	});
	Gl::glEnable(GL_FRAMEBUFFER_SRGB);
	Gl::glDebugMessageCallback(GlDebugMessageHandler, nullptr);
	Gl::glDebugMessageControl(GL_DONT_CARE                   , GL_DONT_CARE       , GL_DONT_CARE, 0, nullptr, GL_FALSE);
	Gl::glDebugMessageControl(GL_DONT_CARE                   , GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, nullptr, GL_TRUE );
	Gl::glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DONT_CARE       , GL_DONT_CARE, 0, nullptr, GL_FALSE);
	SdlAssertZero(SDL_GL_MakeCurrent(sdlWindow, glContext));
	// SdlAssertZero(SDL_GL_SetSwapInterval(1));
	Scene scene;
	Movement movement;
	movement.Warp({ 200, 200, 200 }, { -1, -1, -1 }, { -1, 1, -1 });
	bool running = true;
	auto lastFrameAt = int64_t(SDL_GetTicks64());;
	auto nexTickAt = lastFrameAt;
	auto nextWindowTitleAt = lastFrameAt;
	float fps = 0;
	float fov = 60.f;
	bool paused = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;

			case SDL_MOUSEMOTION:
				if (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT))
				{
					movement.Pan(glm::vec3{ event.motion.xrel, -event.motion.yrel, 0 } * panFactor);
				}
				break;

			case SDL_MOUSEWHEEL:
				{
					auto x = event.wheel.preciseX * (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
					auto y = event.wheel.preciseY * (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
					movement.Pan(glm::vec3{ 0, 0, x } * rollFactor);
					fov = std::clamp(fov - y * fovFactor, 1.f, 180.f);
				}
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					if (event.window.windowID == SDL_GetWindowID(sdlWindow))
					{
						windowSize.x = event.window.data1;
						windowSize.y = event.window.data2;
					}
					break;
				}
				break;

			case SDL_KEYDOWN:
				if (event.key.repeat)
				{
					break;
				}
				switch (event.key.keysym.scancode)
				{
				case SDL_SCANCODE_P:
					paused = !paused;
					if (!paused)
					{
						nexTickAt = int64_t(SDL_GetTicks64());
					}
					break;

				default:
					break;
				}
				break;
			}
		}
		auto now = int64_t(SDL_GetTicks64());
		int64_t ticksDone = 0;
		Tick(movement);
		while (!paused && nexTickAt <= now)
		{
			scene.Tick();
			nexTickAt += tickInterval;
			ticksDone += 1;
			if (ticksDone >= maxTicksDone)
			{
				lrh("too many ticks to catch up on, skipping");
				nexTickAt = now + tickInterval;
				break;
			}
		}
		if (nextWindowTitleAt <= now)
		{
			lrh("fps: ", std::fixed, std::setprecision(2), fps);
			nextWindowTitleAt = now + windowTitleInterval;
		}
		auto frameTime = std::max(INT64_C(1), now - lastFrameAt);
		lastFrameAt = now;
		fps += ((1000.f / frameTime) - fps) * 0.1f;
		Gl::glViewport(0, 0, windowSize.x, windowSize.y);
		scene.Render(movement.GetCfu(), fov, float(windowSize.x) / float(windowSize.y));
		SDL_GL_SwapWindow(sdlWindow);
	}
	return 0;
}
