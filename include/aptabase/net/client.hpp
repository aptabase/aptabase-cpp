#pragma once

#include <functional>
#include <string>
#include <map>
#include "aptabase/model.hpp"

namespace Aptabase{

	enum class HttpClientMethod {
		Post
	};

	extern const char *ToString(HttpClientMethod method);

	class HttpClient {
	public:
		using CompleteCallbackType = std::function<void(std::int32_t)>;
	public:

		virtual void MakeRequest(HttpClientMethod method, const std::string &hostname, const std::string &path, const std::map<std::string, std::string> &headers, std::string&& body, CompleteCallbackType &&complete) = 0;

		void PostEvents(const std::string &url, const std::string &app_key, const std::vector<Event> &events, CompleteCallbackType &&callback);
	}; 

}//namespace Aptabase::