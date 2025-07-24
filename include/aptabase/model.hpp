#pragma once

#include <string>
#include <vector>
#include <variant>
#include <nlohmann/json.hpp>

namespace Aptabase{

	struct EventAttribute {
		std::string key;
		std::variant<std::string, float, double> value;
	};

	struct EventProperties {
		bool isDebug = false;
		std::string locale;
		std::string appVersion;
		std::string sdkVersion;
		std::string osName;
		std::string osVersion;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EventProperties, isDebug, locale, appVersion, sdkVersion, osName, osVersion)
	};

	struct Event {
		std::string timeStamp;
		std::string sessionId;
		std::string eventName;
		EventProperties systemProps;
		std::vector<EventAttribute> eventAttributes;

		nlohmann::json ToJson()const;

		friend void to_json(nlohmann::json& json, const Event& payload) {
			json = payload.ToJson();
		}
	};

}//namespace Aptabase::