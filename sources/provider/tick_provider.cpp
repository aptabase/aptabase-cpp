#include "aptabase/provider/tick_provider.hpp"

AptabaseTickProvider::AptabaseTickProvider(std::string &&url, std::string &&app_key, std::unique_ptr<AptabaseHttpClient> &&client, float flush_interval_seconds):
	m_Url(std::move(url)),
	m_AppKey(std::move(app_key)),
	m_HttpClient(std::move(client))
{}

void AptabaseTickProvider::RecordEvent(AptabaseEventPayload&& event){
	m_Pending.push_back(std::move(event));
}

void AptabaseTickProvider::Flush(){
	auto OnComplete = [this](std::int32_t code) {
		//Handle retries
		m_RunningRequests--;

		Log(code == 200 ? AptabaseProviderVerbosity::Display : AptabaseProviderVerbosity::Error, "Request finished with code: " + std::to_string(code));
	};
	
	//Handle max 25 events
	m_HttpClient->PostEvents(m_Url, m_AppKey, m_Pending, OnComplete);
	m_RunningRequests++;

	m_Pending.clear();
}

void AptabaseTickProvider::Tick() {
	if (AnyPending()) {
		Flush();
	}
}

void AptabaseTickProvider::SetLog(LogFunctionType&& log){
	m_Logger = std::move(log);
}

void AptabaseTickProvider::Log(AptabaseProviderVerbosity verbosity, const std::string& message) {
	if(m_Logger)
		m_Logger(verbosity, message);
}
