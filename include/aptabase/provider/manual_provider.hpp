#pragma once

#include "provider.hpp"

namespace Aptabase{

	class ManualProvider: public Provider{
		std::string m_Url;
		std::string m_AppKey;
		std::vector<Event> m_Pending;
		std::unique_ptr<HttpClient> m_HttpClient;
		std::int32_t m_RunningRequests = 0;
		LogFunctionType m_Logger = Provider::DefaultLogFunction;
	public:
		ManualProvider(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &url);
	
		void RecordEvent(Event &&event)override;

		void Flush()override;

		bool AnyPending()const override{ return m_Pending.size(); }

		bool AnySending()const override{ return m_RunningRequests; }

		bool IsTickRequired()const override{ return false; }

		void SetLog(LogFunctionType &&log)override;

		void Log(Verbosity verbosity, const std::string &message);
	};

}//namespace Aptabase::