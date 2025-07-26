#include "Frequency.hpp"
#include <SDL.h>

namespace Reactor
{
	void Frequency::Cull()
	{
		auto now = int64_t(SDL_GetTicks64());
		while (!timestamps.empty() && timestamps.front() + maxHistoryMs < now)
		{
			timestamps.pop_front();
		}
	}

	void Frequency::Tick()
	{
		auto now = int64_t(SDL_GetTicks64());
		timestamps.push_back(now);
		Cull();
	}

	float Frequency::GetFrequency()
	{
		Cull();
		if (timestamps.size() < 2)
		{
			return 0;
		}
		auto diff = timestamps.back() - timestamps.front();
		if (diff < 1)
		{
			return 1000.f;
		}
		return 1000.f * (timestamps.size() - 1U) / diff;
	}
}
