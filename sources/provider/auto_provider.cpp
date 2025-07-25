#include "aptabase/provider/auto_provider.hpp"

Aptabase::AutoProvider::AutoProvider(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &url):
	Aptabase::ManualProvider(std::move(client), app_key, url)
{}

void Aptabase::AutoProvider::RecordEvent(Event&& event) {
	ManualProvider::RecordEvent(std::move(event));
	
	ManualProvider::Flush();
}
