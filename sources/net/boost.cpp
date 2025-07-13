#include "aptabase/net/boost.hpp"

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>

void AptabaseBoostHttpClient::MakeRequest(AptabaseHttpClientMethod method, const std::string& hostname, const std::string& path, const std::map<std::string, std::string>& headers, std::string&& body, CompleteCallbackType&& complete){
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
        request_stream << ToString(method) << " " << path << " HTTP/1.1\r\n";
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
        
        if(complete) complete(static_cast<std::int32_t>(status_code));
    }
    catch (const std::exception& e) {
        if(complete) complete(0);
    }


}
