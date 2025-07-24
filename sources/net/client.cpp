#include "aptabase/net/client.hpp"

void Aptabase::HttpClient::PostEvents(const std::string& url, const std::string& app_key, const std::vector<Event>& events, CompleteCallbackType &&callback){
	std::map<std::string, std::string> headers = { 
		{"App-Key", app_key},
		{"Content-Type", "application/json"}
	};

	std::string body = nlohmann::json(events).dump();

	MakeRequest(HttpClientMethod::Post, url, "/api/v0/events", headers, std::move(body), std::move(callback));
}

const char* Aptabase::ToString(HttpClientMethod method){
	if(method == HttpClientMethod::Post)
		return "POST";

	assert(false);
	return "<None>";
}
