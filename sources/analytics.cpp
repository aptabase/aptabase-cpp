#include "aptabase/analytics.hpp"
#include <random>
#include <sstream>

AptabaseAnalytics::AptabaseAnalytics(std::unique_ptr<AptabaseProvider> &&provider, bool is_debug):
	m_Provider(std::move(provider)),
	m_IsDebug(is_debug)
{}

void AptabaseAnalytics::Tick() {
	if(m_Provider && IsTickRequired())
		m_Provider->Tick();
}

void AptabaseAnalytics::StartSession() {
	if(IsInSession())
		EndSession();

	m_SessionId = GenerateSessionId();
}

void AptabaseAnalytics::EndSession() {
	m_SessionId = {};
}

bool AptabaseAnalytics::IsInSession()const {
	return m_SessionId.size();
}

void AptabaseAnalytics::RecordEvent(const std::string& event_name, const std::vector<ExtendedAnalyticsEventAttribute>& attributes) {
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

bool AptabaseAnalytics::IsTickRequired()const {
	return m_Provider ? m_Provider->IsTickRequired() : false;
}

void AptabaseAnalytics::SetDebug(bool is_debug) {
	m_IsDebug = is_debug;
}

bool AptabaseAnalytics::IsDebug()const {
	return m_IsDebug;
}

void AptabaseAnalytics::SetAppVersion(std::string&& app_version) {
	m_AppVersion = std::move(app_version);
}

void AptabaseAnalytics::SetLocale(std::string&& locale) {
	m_Locale = std::move(locale);
}

void AptabaseAnalytics::SetLog(AptabaseProvider::LogFunctionType&& log) {
	if(m_Provider)
		m_Provider->SetLog(std::move(log));
}

std::string AptabaseAnalytics::GetCurrentTimestamp() const{
	auto now = std::chrono::system_clock::now();
	auto itt = std::chrono::system_clock::to_time_t(now);
	std::ostringstream ss;
	ss << std::put_time(gmtime(&itt), "%FT%TZ");
	return ss.str();
}

std::string AptabaseAnalytics::GenerateSessionId() const{
	auto now = std::chrono::system_clock::now().time_since_epoch();
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<> dist(10000000, 99999999);

	return std::to_string(seconds) + std::to_string(dist(rng));
}
