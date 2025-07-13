#pragma once

#include "provider.hpp"
#include <chrono>

class AptabaseTickProvider: public AptabaseProvider{
	std::string m_Url;
	std::string m_AppKey;
	std::vector<AptabaseEventPayload> m_Pending;
	std::unique_ptr<AptabaseHttpClient> m_HttpClient;
	std::int32_t m_RunningRequests = 0;
	LogFunctionType m_Logger = AptabaseProvider::DefaultLogFunction;
public:
	AptabaseTickProvider(std::string &&url, std::string &&app_key, std::unique_ptr<AptabaseHttpClient> &&client, float flush_interval_seconds = 5.f);
	
	void RecordEvent(AptabaseEventPayload &&event)override;

	void Flush()override;

	bool AnyPending()const override{ return m_Pending.size(); }

	bool AnySending()const override{ return m_RunningRequests; }

	void SetLog(LogFunctionType &&log)override;

	void Tick()override;

	bool IsTickRequired()const override{ return true; }

	void Log(AptabaseProviderVerbosity verbosity, const std::string &message);
};