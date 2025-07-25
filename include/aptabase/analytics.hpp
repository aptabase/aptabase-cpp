#pragma once

#include "model.hpp"
#include "provider/provider.hpp"

namespace Aptabase{

	class Analytics {
		std::unique_ptr<Provider> m_Provider;

		std::string m_SessionId;
		bool m_IsDebug = false;

		std::string m_AppVersion = "1.0.0";
		std::string m_Locale = "en-US";
		std::string m_OsVersion = "0.0.0";
		std::string m_OsName = "Unknown";

		Provider::LogFunctionType m_LogFunction;
	public:
		Analytics(std::unique_ptr<Provider> &&provider, bool is_debug = false);

		Analytics(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &app_url, bool is_debug = false);

		void StartSession(const std::string &session_id = {});

		void EndSession();

		bool IsInSession()const;

		void RecordEvent(const std::string& event_name, const std::vector<EventAttribute>& attributes = {});

		void RecordEvent(Event &&event);

		void Flush();

		void SetDebug(bool is_debug);

		bool IsDebug()const;

		void SetAppVersion(std::string &&app_version);

		void SetLocale(std::string &&locale);

		void SetOsVersion(std::string &&os_version);

		void SetOsName(std::string &&os_name);

		void SetLog(Provider::LogFunctionType &&log);

	private:
		std::string GetCurrentTimestamp() const;

		std::string GenerateSessionId() const;

		void Log(Verbosity verbosity, const std::string &message);
	};

}//namespace Aptabase::