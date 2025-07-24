#pragma once

#include "provider.hpp"
#include <chrono>

namespace Aptabase{

	class TickProvider: public Provider{
		std::string m_Url;
		std::string m_AppKey;
		std::vector<Event> m_Pending;
		std::unique_ptr<HttpClient> m_HttpClient;
		std::int32_t m_RunningRequests = 0;
		LogFunctionType m_Logger = Provider::DefaultLogFunction;
	public:
		TickProvider(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &url);
	
		void RecordEvent(Event &&event)override;

		void Flush()override;

		bool AnyPending()const override{ return m_Pending.size(); }

		bool AnySending()const override{ return m_RunningRequests; }

		void SetLog(LogFunctionType &&log)override;

		void Tick()override;

		bool IsTickRequired()const override{ return true; }

		void Log(Verbosity verbosity, const std::string &message);
	};

}//namespace Aptabase::