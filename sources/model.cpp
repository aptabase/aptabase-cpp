#include "aptabase/model.hpp"

nlohmann::json AptabaseEventPayload::ToJson() const {
	nlohmann::json result = nlohmann::json::object();

	result["timeStamp"] = timeStamp;
	result["sessionId"] = sessionId;
	result["eventName"] = eventName;;
	result["systemProps"] = systemProps;
	result["props"] = nlohmann::json::object();

	for (const auto& attribute : eventAttributes) {
		if (const std::string *value = std::get_if<std::string>(&attribute.value)) {
			result["props"][attribute.key] = *value;
		}
		if (const float *value = std::get_if<float>(&attribute.value)) {
			result["props"][attribute.key] = *value;
		}
		if (const double *value = std::get_if<double>(&attribute.value)) {
			result["props"][attribute.key] = *value;
		}
	}
	return result;
}