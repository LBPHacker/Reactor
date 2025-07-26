#pragma once
#include <deque>
#include <cstdint>

namespace Reactor
{
	class Frequency
	{
		std::deque<int64_t> timestamps;
		int64_t maxHistoryMs = 1000;
		void Cull();

	public:
		void SetMaxHistory(int64_t newMaxHistoryMs)
		{
			maxHistoryMs = newMaxHistoryMs;
			Cull();
		}

		void Tick();
		float GetFrequency();
	};
}
