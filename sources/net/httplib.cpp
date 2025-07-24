#include "aptabase/net/httplib.hpp"

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT 1
#endif

#include <httplib.h>

void Aptabase::HttplibHttpClient::MakeRequest(HttpClientMethod method, const std::string& hostname, const std::string& path, const std::map<std::string, std::string>& headers, std::string&& body, CompleteCallbackType&& complete){
	httplib::Client client(hostname);

	if (method == AptabaseHttpClientMethod::Post) {
		auto res = client.Post(path, headers, body, headers.count("Content-Type") ? headers.at("Content-Type") : "text/plain");

		if(complete) complete(res ? res->status : 0);
	}

	if(complete) complete(0);
}
