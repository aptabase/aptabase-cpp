#pragma once

#include "model.hpp"
#include "provider/provider.hpp"

class AptabaseAnalytics {
	std::unique_ptr<AptabaseProvider> m_Provider;

	std::string m_SessionId;
	bool m_IsDebug = false;

	std::string m_AppVersion = "1.0.0";
	std::string m_Locale = "en-US";

	AptabaseProvider::LogFunctionType m_LogFunction;
public:
	AptabaseAnalytics(std::unique_ptr<AptabaseProvider> &&provider, bool is_debug = false);

	void StartSession();

	void EndSession();

	bool IsInSession()const;

	void RecordEvent(const std::string& event_name, const std::vector<ExtendedAnalyticsEventAttribute>& attributes);

	void Tick();

	bool IsTickRequired()const;

	void SetDebug(bool is_debug);

	bool IsDebug()const;

	void SetAppVersion(std::string &&app_version);

	void SetLocale(std::string &&locale);

	void SetLog(AptabaseProvider::LogFunctionType &&log);

private:
	std::string GetCurrentTimestamp() const;

	std::string GenerateSessionId() const;
};