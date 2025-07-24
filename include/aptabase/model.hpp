#pragma once

#include <string>
#include <vector>
#include <variant>

namespace Aptabase{

	struct EventAttribute {
		std::string Key;
		std::variant<std::string, float, double> Value;
	};

	struct EventProperties {
		bool IsDebug = false;
		std::string Locale;
		std::string AppVersion;
		std::string SdkVersion;
		std::string OsName;
		std::string OsVersion;
	};

	struct Event {
		std::string TimeStamp;
		std::string SessionId;
		std::string EventName;
		EventProperties SystemProps;
		std::vector<EventAttribute> EventAttributes;

		std::string ToJson()const;

		static std::string ToJson(const std::vector<Event> &events);
	};

}//namespace Aptabase::