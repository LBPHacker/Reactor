#pragma once
#include <string>
#include <sstream>
#include <cstdlib>

namespace Reactor::Common
{
	class LogRealm
	{
		std::string name;

		void LogOne(std::string str);

		void LogMore(std::ostream &)
		{
		}

		template<class Thing, class ...Rest>
		void LogMore(std::ostream &os, Thing &&thing, Rest &&...rest)
		{
			os << thing;
			LogMore(os, std::forward<Rest>(rest)...);
		}

	public:
		LogRealm(std::string newName) : name(newName)
		{
		}

		LogRealm(const LogRealm &) = delete;
		LogRealm &operator =(const LogRealm &) = delete;

		template<class ...Things>
		void Log(Things &&...things)
		{
			std::ostringstream ss;
			ss << "[" << name << "] ";
			LogMore(ss, std::forward<Things>(things)...);
			ss << "\n";
			LogOne(ss.str());
		}
	};

	class LogRealmHandle
	{
		std::string name;
		LogRealm *logRealm = nullptr;
		void EnsureLogRealm();

	public:
		LogRealmHandle(std::string newName) : name(newName)
		{
		}

		LogRealmHandle(const LogRealmHandle &) = delete;
		LogRealmHandle &operator =(const LogRealmHandle &) = delete;

		template<class ...Things>
		void operator ()(Things &&...things)
		{
			EnsureLogRealm();
			logRealm->Log(std::forward<Things>(things)...);
		}

		template<class ...Things>
		void Die(Things &&...things)
		{
			this->operator ()(std::forward<Things>(things)...);
			abort();
		}
	};
}
