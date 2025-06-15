#pragma once

#include <string>
#include <vector>
#include <variant>
#include <nlohmann/json.hpp>

struct ExtendedAnalyticsEventAttribute {
	std::string key;
	std::variant<std::string, float, double> value;
};

struct AptabaseSystemProperties {
	bool isDebug = false;
	std::string locale;
	std::string appVersion;
	std::string sdkVersion;
	std::string osName;
	std::string osVersion;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AptabaseSystemProperties, isDebug, locale, appVersion, sdkVersion, osName, osVersion)
};

struct AptabaseEventPayload {
	std::string timeStamp;
	std::string sessionId;
	std::string eventName;
	AptabaseSystemProperties systemProps;
	std::vector<ExtendedAnalyticsEventAttribute> eventAttributes;

	nlohmann::json ToJson()const;

	friend void to_json(nlohmann::json& json, const AptabaseEventPayload& payload) {
		json = payload.ToJson();
	}
};