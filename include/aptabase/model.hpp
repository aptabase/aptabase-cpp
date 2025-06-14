#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct ExtendedAnalyticsEventAttribute {
	std::string key;
	std::string value;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ExtendedAnalyticsEventAttribute, key, value)
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

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AptabaseEventPayload, timeStamp, sessionId, eventName, systemProps, eventAttributes)
};