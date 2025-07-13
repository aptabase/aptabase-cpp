#include "aptabase/net/client.hpp"

void AptabaseHttpClient::PostEvents(const std::string& url, const std::string& app_key, const std::vector<AptabaseEventPayload>& events, CompleteCallbackType &&callback){
	std::map<std::string, std::string> headers = { 
		{"App-Key", app_key},
		{"Content-Type", "application/json"}
	};

	std::string body = nlohmann::json(events).dump();

	MakeRequest(AptabaseHttpClientMethod::Post, url, "/api/v0/events", headers, std::move(body), std::move(callback));
}

const char* ToString(AptabaseHttpClientMethod method){
	if(method == AptabaseHttpClientMethod::Post)
		return "POST";

	assert(false);
	return "<None>";
}
