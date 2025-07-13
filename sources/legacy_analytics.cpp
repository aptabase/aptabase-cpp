#include "aptabase/provider.hpp"

#ifdef APTABASE_USE_HTTPLIB
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#endif

#ifdef APTABASE_USE_BOOST
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#endif

#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <random>

AptabaseAnalyticsLegacy::AptabaseAnalyticsLegacy(std::string app_key, std::string api_url, bool is_debug_mode): 
	m_AppKey(std::move(app_key)), 
	m_ApiUrl(std::move(api_url)), 
	m_IsDebug(is_debug_mode)
{
	m_FlushInterval = m_IsDebug ? std::chrono::seconds(5) : std::chrono::seconds(15);

	StartSession();

	m_StopThread = false;
	m_FlushThread = std::thread(&AptabaseAnalyticsLegacy::FlushWorkerLoop, this);
}

AptabaseAnalyticsLegacy::~AptabaseAnalyticsLegacy(){
	EndSession();

	m_StopThread = true;
	m_FlushCV.notify_all();

	if (m_FlushThread.joinable())
	{
		m_FlushThread.join();
	}
}

void AptabaseAnalyticsLegacy::StartSession(){
	m_HasActiveSession = true;
	m_SessionId = GenerateSessionId();
}

void AptabaseAnalyticsLegacy::EndSession(){
	if (!m_HasActiveSession)
	{
		return;
	}

	FlushEvents();
	m_HasActiveSession = false;
}

void AptabaseAnalyticsLegacy::RecordEvent(const std::string& event_name, const std::vector<ExtendedAnalyticsEventAttribute>& attributes){
	if (!m_HasActiveSession)
	{
		std::cerr << "[Aptabase] Cannot record event: no active session.\n";
		return;
	}

	AptabaseEventPayload payload;
	payload.eventName = event_name;
	payload.sessionId = m_SessionId;
	payload.timeStamp = GetCurrentTimestamp();

	payload.systemProps.isDebug = m_IsDebug;
	payload.systemProps.locale = "en-US";
	payload.systemProps.appVersion = "1.0.0";
	payload.systemProps.sdkVersion = "aptabase-cpp@0.1.0";
	payload.systemProps.osName = "Linux";
	payload.systemProps.osVersion = "5.15";

	payload.eventAttributes = attributes;

	{
		std::lock_guard<std::mutex> lock(m_BatchMutex);
		m_BatchedEvents.push_back(payload);
	}

	m_FlushCV.notify_all();
}

void AptabaseAnalyticsLegacy::FlushWorkerLoop(){
	std::mutex cv_mutex;
	std::unique_lock<std::mutex> lock(cv_mutex);

	while (!m_StopThread)
	{
		m_FlushCV.wait_for(lock, m_FlushInterval, [this]() {
			return m_StopThread || !m_BatchedEvents.empty();
		});

		if (m_StopThread)
			break;

		FlushEvents();
	}
}

void AptabaseAnalyticsLegacy::FlushEvents(){
	std::vector<AptabaseEventPayload> batch;
	{
		std::lock_guard<std::mutex> lock(m_BatchMutex);
		if (m_BatchedEvents.empty())
			return;

		batch.swap(m_BatchedEvents);
	}
	SendEventsNow(batch);
}

void AptabaseAnalyticsLegacy::SendEventsNow(std::vector<AptabaseEventPayload> events){
	nlohmann::json json_payload = events;
	std::string body = json_payload.dump();
	
	auto status = MakePostRequest(m_ApiUrl, "/api/v0/events", {{"App-Key", m_AppKey}, {"Content-Type", "application/json"}}, body);

	if (status >= 200 && status < 300)
	{
		std::cout << "[Aptabase] ✅ " << events.size() << " event(s) recorded.\n";
	}
	else
	{
		std::cerr << "[Aptabase] ❌ Server returned HTTP " << status << "\n";
	}
}

std::int32_t AptabaseAnalyticsLegacy::MakePostRequest(const std::string& hostname, const std::string& path, const std::map<std::string, std::string>& headers, const std::string& body) {
#ifdef APTABASE_USE_HTTPLIB
	httplib::Client client(hostname);
	auto res = client.Post(path, headers, body, headers.count("Content-Type") ? headers.at("Content-Type") : "text/plain");

	if (!res)
	{
		return 0;
	}

	return res->status;
#endif
#ifdef APTABASE_USE_BOOST
    try {
        bool is_https = false;
        std::string clean_hostname = hostname;
        
        if (boost::starts_with(hostname, "https://")) {
            is_https = true;
            clean_hostname = hostname.substr(8);
        } 
        else if (boost::starts_with(hostname, "http://")) {
            is_https = false;
            clean_hostname = hostname.substr(7);
        }
        
        if (!clean_hostname.empty() && clean_hostname.back() == '/') {
            clean_hostname.pop_back();
        }
        
        boost::asio::io_context io_context;
        
        boost::asio::ip::tcp::resolver resolver(io_context);
        
        std::string port = is_https ? "443" : "80";
        
        boost::asio::ip::tcp::socket socket(io_context);
        
        auto endpoints = resolver.resolve(clean_hostname, port);
        boost::asio::connect(socket, endpoints);
        
        std::unique_ptr<boost::asio::ssl::context> ssl_context;
        std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>> ssl_stream;
        
        if (is_https) {
            ssl_context = std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12_client);
            ssl_context->set_default_verify_paths();
			ssl_context->set_options(
				boost::asio::ssl::context::default_workarounds |
				boost::asio::ssl::context::no_sslv2 |
				boost::asio::ssl::context::no_sslv3 |
				boost::asio::ssl::context::no_tlsv1 |
				boost::asio::ssl::context::single_dh_use
			);
            ssl_stream = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket&>>(socket, *ssl_context);
            ssl_stream->set_verify_mode(boost::asio::ssl::verify_none);
            ssl_stream->set_verify_callback(boost::asio::ssl::rfc2818_verification(clean_hostname));

			if (SSL_set_tlsext_host_name(ssl_stream->native_handle(), clean_hostname.c_str()) != 1) {
				throw boost::system::system_error(
					boost::system::error_code(
						static_cast<int>(ERR_get_error()),
						boost::asio::error::get_ssl_category()
					)
				);
			}

            ssl_stream->handshake(boost::asio::ssl::stream_base::client);
        }
        
        std::stringstream request_stream;
        request_stream << "POST " << path << " HTTP/1.1\r\n";
        request_stream << "Host: " << clean_hostname << "\r\n";
        request_stream << "Content-Length: " << body.length() << "\r\n";
        
        for (const auto& header : headers) {
            request_stream << header.first << ": " << header.second << "\r\n";
        }
        
        if (headers.find("Content-Type") == headers.end()) {
            request_stream << "Content-Type: text/plain\r\n";
        }
        
        request_stream << "\r\n" << body;
        
        std::string request = request_stream.str();
        
        if (is_https) {
            boost::asio::write(*ssl_stream, boost::asio::buffer(request));
        } else {
            boost::asio::write(socket, boost::asio::buffer(request));
        }
        
        boost::asio::streambuf response;
        
        if (is_https) {
            boost::asio::read_until(*ssl_stream, response, "\r\n");
        } else {
            boost::asio::read_until(socket, response, "\r\n");
        }
        
        std::istream response_stream(&response);
        std::string http_version;
        unsigned int status_code;
        std::string status_message;
        
        response_stream >> http_version;
        response_stream >> status_code;
        std::getline(response_stream, status_message);
        
        return static_cast<std::int32_t>(status_code);
    }
    catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
        return 0;
    }
#endif
}

std::string AptabaseAnalyticsLegacy::GetCurrentTimestamp() const{
	auto now = std::chrono::system_clock::now();
	auto itt = std::chrono::system_clock::to_time_t(now);
	std::ostringstream ss;
	ss << std::put_time(gmtime(&itt), "%FT%TZ");
	return ss.str();
}

std::string AptabaseAnalyticsLegacy::GenerateSessionId() const{
	auto now = std::chrono::system_clock::now().time_since_epoch();
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<> dist(10000000, 99999999);

	return std::to_string(seconds) + std::to_string(dist(rng));
}