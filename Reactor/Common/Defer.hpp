#pragma once
#include <utility>

namespace Reactor::Common
{
	template<class Func>
	class Defer
	{
		Func func;

	public:
		Defer(Func &&newFunc) : func(std::forward<Func>(newFunc))
		{
		}

		Defer(const Defer &) = delete;
		Defer &operator =(const Defer &) = delete;

		~Defer()
		{
			func();
		}
	};
}
