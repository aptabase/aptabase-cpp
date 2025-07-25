#include "aptabase/provider/tick_provider.hpp"

Aptabase::TickProvider::TickProvider(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &url):
	Aptabase::ManualProvider(std::move(client), app_key, url)
{}

void Aptabase::TickProvider::Tick() {
	if (AnyPending()) {
		Flush();
	}
}
