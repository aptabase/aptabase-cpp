#include "aptabase/analytics.hpp"
#include <random>
#include <sstream>

Analytics::Analytics(std::unique_ptr<AptabaseProvider> &&provider, bool is_debug):
	m_Provider(std::move(provider)),
	m_IsDebug(is_debug)
{}

void Analytics::Tick() {
	if(m_Provider && IsTickRequired())
		m_Provider->Tick();
}

void Analytics::StartSession() {
	if(IsInSession())
		EndSession();

	m_SessionId = GenerateSessionId();
}

void Analytics::EndSession() {
	m_SessionId = {};
}

bool Analytics::IsInSession()const {
	return m_SessionId.size();
}

void Analytics::RecordEvent(const std::string& event_name, const std::vector<ExtendedAnalyticsEventAttribute>& attributes) {
	AptabaseEventPayload payload;
	payload.eventName = event_name;
	payload.sessionId = m_SessionId;
	payload.timeStamp = GetCurrentTimestamp();

	payload.systemProps.isDebug = m_IsDebug;
	payload.systemProps.locale = m_Locale;
	payload.systemProps.appVersion = m_AppVersion;
	payload.systemProps.sdkVersion = "aptabase-cpp@0.1.0";
	payload.systemProps.osName = "Linux";
	payload.systemProps.osVersion = "5.15";

	payload.eventAttributes = attributes;

	if(m_Provider)
		m_Provider->RecordEvent(std::move(payload));
}

bool Analytics::IsTickRequired()const {
	return m_Provider ? m_Provider->IsTickRequired() : false;
}

void Analytics::SetDebug(bool is_debug) {
	m_IsDebug = is_debug;
}

bool Analytics::IsDebug()const {
	return m_IsDebug;
}

void Analytics::SetAppVersion(std::string&& app_version) {
	m_AppVersion = std::move(app_version);
}

void Analytics::SetLocale(std::string&& locale) {
	m_Locale = std::move(locale);
}

std::string Analytics::GetCurrentTimestamp() const{
	auto now = std::chrono::system_clock::now();
	auto itt = std::chrono::system_clock::to_time_t(now);
	std::ostringstream ss;
	ss << std::put_time(gmtime(&itt), "%FT%TZ");
	return ss.str();
}

std::string Analytics::GenerateSessionId() const{
	auto now = std::chrono::system_clock::now().time_since_epoch();
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<> dist(10000000, 99999999);

	return std::to_string(seconds) + std::to_string(dist(rng));
}
