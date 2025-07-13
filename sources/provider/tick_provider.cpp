#include "aptabase/provider/tick_provider.hpp"

AptabaseTickProvider::AptabaseTickProvider(std::unique_ptr<AptabaseHttpClient> &&client, const std::string &app_key, const std::string &url):
	m_Url(url),
	m_AppKey(app_key),
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
