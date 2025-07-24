#include "aptabase/provider/tick_provider.hpp"

Aptabase::TickProvider::TickProvider(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &url):
	m_Url(url),
	m_AppKey(app_key),
	m_HttpClient(std::move(client))
{}

void Aptabase::TickProvider::RecordEvent(Event&& event){
	m_Pending.push_back(std::move(event));
}

void Aptabase::TickProvider::Flush(){
	auto OnComplete = [this](std::int32_t code) {
		//Handle retries
		m_RunningRequests--;

		Log(code == 200 ? Verbosity::Display : Verbosity::Error, "Request finished with code: " + std::to_string(code));
	};
	
	//Handle max 25 events
	m_HttpClient->PostEvents(m_Url, m_AppKey, m_Pending, OnComplete);
	m_RunningRequests++;

	m_Pending.clear();
}

void Aptabase::TickProvider::Tick() {
	if (AnyPending()) {
		Flush();
	}
}

void Aptabase::TickProvider::SetLog(LogFunctionType&& log){
	m_Logger = std::move(log);
}

void Aptabase::TickProvider::Log(Verbosity verbosity, const std::string& message) {
	if(m_Logger)
		m_Logger(verbosity, message);
}
