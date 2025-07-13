#pragma once

#include "model.hpp"
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>
#include <atomic>
#include <chrono>

class AptabaseAnalyticsLegacy{
private:
	std::string m_SessionId;
	bool m_HasActiveSession = false;

	std::string m_AppKey;
	std::string m_ApiUrl;
	bool m_IsDebug = false;

	std::chrono::seconds m_FlushInterval;

	std::vector<AptabaseEventPayload> m_BatchedEvents;
	std::mutex m_BatchMutex;

	std::thread m_FlushThread;
	std::atomic<bool> m_StopThread = false;
	std::condition_variable m_FlushCV;
public:
	explicit AptabaseAnalyticsLegacy(std::string app_key, std::string api_url, bool is_debug_mode = false);

	~AptabaseAnalyticsLegacy();

	void StartSession();

	void EndSession();

	void RecordEvent(const std::string& event_name, const std::vector<ExtendedAnalyticsEventAttribute>& attributes);

private:
	void FlushWorkerLoop();

	void FlushEvents();

	void SendEventsNow(std::vector<AptabaseEventPayload> events);

	static std::int32_t MakePostRequest(const std::string &hostname, const std::string &path, const std::map<std::string, std::string> &headers, const std::string& body);

	std::string GetCurrentTimestamp() const;

	std::string GenerateSessionId() const;
};