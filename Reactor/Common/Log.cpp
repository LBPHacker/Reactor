#include "Log.hpp"
#include <iostream>
#include <map>
#include <memory>
#include <mutex>

namespace Reactor::Common
{
	namespace
	{
		LogRealm *GetRealm(const std::string &name)
		{
			static std::map<std::string, std::unique_ptr<LogRealm>> logRealms;
			static std::mutex logRealmsMx;
			std::lock_guard lk(logRealmsMx);
			auto &ptr = logRealms[name];
			if (!ptr)
			{
				ptr = std::make_unique<LogRealm>(name);
			}
			return ptr.get();
		}
	}

	void LogRealm::LogOne(std::string str)
	{
		std::cerr << str << std::flush;
	}

	void LogRealmHandle::EnsureLogRealm()
	{
		if (!logRealm)
		{
			logRealm = GetRealm(name);
		}
	}
}
