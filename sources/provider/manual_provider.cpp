#include "aptabase/provider/manual_provider.hpp"

Aptabase::ManualProvider::ManualProvider(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &url):
	m_Url(url),
	m_AppKey(app_key),
	m_HttpClient(std::move(client))
{}

void Aptabase::ManualProvider::RecordEvent(Event&& event){
	if(m_Pending.size() >= HttpClient::MaxEventsPerRequest){
		Log(Verbosity::Error, "ManualProvider::RecordEvent: MaxEventsPerRequest of " + std::to_string(HttpClient::MaxEventsPerRequest) + " is reached, event '" + event.EventName+ "' was not recorded, please Flush and RecordEvent again");
		return;
	}

	m_Pending.push_back(std::move(event));
}

void Aptabase::ManualProvider::Flush(){
	auto OnComplete = [this](std::int32_t code) {
		//Handle retries
		m_RunningRequests--;
		
		Log(code == 200 ? Verbosity::Display: Verbosity::Error, "Request finished with code: " + std::to_string(code));
	};

	if(m_Pending.size() >= HttpClient::MaxEventsPerRequest){
		Log(Verbosity::Error, "ManualProvider::Flush: MaxEventsPerRequest of " + std::to_string(HttpClient::MaxEventsPerRequest) + " is reached, some events will be lost");
	}

	m_HttpClient->PostEvents(m_Url, m_AppKey, m_Pending, OnComplete);
	m_RunningRequests++;

	m_Pending.clear();
}

void Aptabase::ManualProvider::SetLog(LogFunctionType&& log){
	m_Logger = std::move(log);
}

void Aptabase::ManualProvider::Log(Verbosity verbosity, const std::string& message) {
	if(m_Logger)
		m_Logger(verbosity, message);
}
