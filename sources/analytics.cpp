#include "aptabase/analytics.hpp"
#include <random>
#include <chrono>
#include <sstream>

Aptabase::Analytics::Analytics(std::unique_ptr<Provider> &&provider, bool is_debug):
	m_Provider(std::move(provider)),
	m_IsDebug(is_debug)
{}

void Aptabase::Analytics::Tick() {
	if(m_Provider && IsTickRequired())
		m_Provider->Tick();
}

void Aptabase::Analytics::StartSession(const std::string &session_id) {
	if(IsInSession())
		EndSession();
	
	m_SessionId = session_id.size() ? session_id : GenerateSessionId();
}

void Aptabase::Analytics::EndSession() {
	m_SessionId = {};
}

bool Aptabase::Analytics::IsInSession()const {
	return m_SessionId.size();
}

void Aptabase::Analytics::RecordEvent(const std::string& event_name, const std::vector<EventAttribute>& attributes) {
	Event event;
	event.eventName = event_name;
	event.sessionId = m_SessionId;
	event.timeStamp = GetCurrentTimestamp();

	event.systemProps.isDebug = m_IsDebug;
	event.systemProps.locale = m_Locale;
	event.systemProps.appVersion = m_AppVersion;
	event.systemProps.sdkVersion = "aptabase-cpp@0.1.0";
	event.systemProps.osName = m_OsName;
	event.systemProps.osVersion = m_OsVersion;

	event.eventAttributes = attributes;
	
	RecordEvent(std::move(event));
}

void Aptabase::Analytics::RecordEvent(Event&& event) {
	if(m_Provider)
		m_Provider->RecordEvent(std::move(event));
}

bool Aptabase::Analytics::IsTickRequired()const {
	return m_Provider ? m_Provider->IsTickRequired() : false;
}

void Aptabase::Analytics::SetDebug(bool is_debug) {
	m_IsDebug = is_debug;
}

bool Aptabase::Analytics::IsDebug()const {
	return m_IsDebug;
}

void Aptabase::Analytics::SetAppVersion(std::string&& app_version) {
	m_AppVersion = std::move(app_version);
}

void Aptabase::Analytics::SetLocale(std::string&& locale) {
	m_Locale = std::move(locale);
}

void Aptabase::Analytics::SetOsVersion(std::string&& os_version) {
	m_OsVersion = std::move(os_version);
}

void Aptabase::Analytics::SetOsName(std::string&& os_name) {
	m_OsName = std::move(os_name);
}

void Aptabase::Analytics::SetLog(Provider::LogFunctionType&& log) {
	if(m_Provider)
		m_Provider->SetLog(std::move(log));
}

std::string Aptabase::Analytics::GetCurrentTimestamp() const{
	auto now = std::chrono::system_clock::now();
	auto itt = std::chrono::system_clock::to_time_t(now);
	std::ostringstream ss;
	ss << std::put_time(gmtime(&itt), "%FT%TZ");
	return ss.str();
}

std::string Aptabase::Analytics::GenerateSessionId() const{
	auto now = std::chrono::system_clock::now().time_since_epoch();
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<> dist(10000000, 99999999);

	return std::to_string(seconds) + std::to_string(dist(rng));
}
