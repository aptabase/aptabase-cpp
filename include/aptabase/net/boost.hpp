#pragma once


#include "client.hpp"


class AptabaseBoostHttpClient : public AptabaseHttpClient {
public
	void MakeRequest(AptabaseHttpClientMethod method, const std::string &hostname, const std::string &path, const std::map<std::string, std::string> &headers, std::string&& body, CompleteCallbackType &&complete)override;
};