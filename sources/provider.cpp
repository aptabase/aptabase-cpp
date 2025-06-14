#include "aptabase/provider.hpp"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <random>

AptabaseAnalyticsProvider::AptabaseAnalyticsProvider(std::string app_key, std::string api_url, bool is_debug_mode): 
	m_AppKey(std::move(app_key)), 
	m_ApiUrl(std::move(api_url)), 
	m_IsDebug(is_debug_mode)
{
	m_FlushInterval = m_IsDebug ? std::chrono::seconds(5) : std::chrono::seconds(15);

	StartSession();

	m_StopThread = false;
	m_FlushThread = std::thread(&AptabaseAnalyticsProvider::FlushWorkerLoop, this);
}

AptabaseAnalyticsProvider::~AptabaseAnalyticsProvider(){
	EndSession();

	m_StopThread = true;
	m_FlushCV.notify_all();

	if (m_FlushThread.joinable())
	{
		m_FlushThread.join();
	}
}

void AptabaseAnalyticsProvider::StartSession(){
	m_HasActiveSession = true;
	m_SessionId = GenerateSessionId();
}

void AptabaseAnalyticsProvider::EndSession(){
	if (!m_HasActiveSession)
	{
		return;
	}

	FlushEvents();
	m_HasActiveSession = false;
}

void AptabaseAnalyticsProvider::RecordEvent(const std::string& event_name, const std::vector<ExtendedAnalyticsEventAttribute>& attributes){
	if (!m_HasActiveSession)
	{
		std::cerr << "[Aptabase] Cannot record event: no active session.\n";
		return;
	}

	AptabaseEventPayload payload;
	payload.eventName = event_name;
	payload.sessionId = m_SessionId;
	payload.timeStamp = GetCurrentTimestamp();

	payload.systemProps.isDebug = m_IsDebug;
	payload.systemProps.locale = "en-US";
	payload.systemProps.appVersion = "1.0.0";
	payload.systemProps.sdkVersion = "aptabase-cpp@0.1.0";
	payload.systemProps.osName = "Linux";
	payload.systemProps.osVersion = "5.15";

	payload.eventAttributes = attributes;

	{
		std::lock_guard<std::mutex> lock(m_BatchMutex);
		m_BatchedEvents.push_back(payload);
	}

	m_FlushCV.notify_all();
}

void AptabaseAnalyticsProvider::FlushWorkerLoop(){
	std::mutex cv_mutex;
	std::unique_lock<std::mutex> lock(cv_mutex);

	while (!m_StopThread)
	{
		m_FlushCV.wait_for(lock, m_FlushInterval, [this]() {
			return m_StopThread || !m_BatchedEvents.empty();
		});

		if (m_StopThread)
			break;

		FlushEvents();
	}
}

void AptabaseAnalyticsProvider::FlushEvents(){
	std::vector<AptabaseEventPayload> batch;
	{
		std::lock_guard<std::mutex> lock(m_BatchMutex);
		if (m_BatchedEvents.empty())
			return;

		batch.swap(m_BatchedEvents);
	}
	SendEventsNow(batch);
}

void AptabaseAnalyticsProvider::SendEventsNow(std::vector<AptabaseEventPayload> events){
	nlohmann::json json_payload = events;
	std::string body = json_payload.dump();

	httplib::Client client(m_ApiUrl.c_str());
	auto res = client.Post("/api/v0/events", {
		{"App-Key", m_AppKey},
		{"Content-Type", "application/json"}
	}, body, "application/json");

	if (!res)
	{
		std::cerr << "[Aptabase] ❌ Failed to send event batch (no response)\n";
		return;
	}

	if (res->status >= 200 && res->status < 300)
	{
		std::cout << "[Aptabase] ✅ " << events.size() << " event(s) recorded.\n";
	}
	else
	{
		std::cerr << "[Aptabase] ❌ Server returned HTTP " << res->status << "\n";
	}
}

std::string AptabaseAnalyticsProvider::GetCurrentTimestamp() const{
	auto now = std::chrono::system_clock::now();
	auto itt = std::chrono::system_clock::to_time_t(now);
	std::ostringstream ss;
	ss << std::put_time(gmtime(&itt), "%FT%TZ");
	return ss.str();
}

std::string AptabaseAnalyticsProvider::GenerateSessionId() const{
	auto now = std::chrono::system_clock::now().time_since_epoch();
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<> dist(10000000, 99999999);

	return std::to_string(seconds) + std::to_string(dist(rng));
}