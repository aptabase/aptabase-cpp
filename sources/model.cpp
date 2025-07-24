#include "aptabase/model.hpp"
#include <sstream>
#include <iomanip>

namespace {
	std::string EscapeJsonString(const std::string& input) {
		std::ostringstream escaped;
		for (char c : input) {
			switch (c) {
				case '"':  escaped << "\\\""; break;
				case '\\': escaped << "\\\\"; break;
				case '\b': escaped << "\\b";  break;
				case '\f': escaped << "\\f";  break;
				case '\n': escaped << "\\n";  break;
				case '\r': escaped << "\\r";  break;
				case '\t': escaped << "\\t";  break;
				default:
					if (c >= 0 && c < 0x20) {
						escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<unsigned char>(c);
					} else {
						escaped << c;
					}
					break;
			}
		}
		return escaped.str();
	}

	std::string FormatNumber(double value) {
		std::ostringstream ss;
		ss << std::fixed << std::setprecision(15) << value;
		std::string result = ss.str();
		
		result.erase(result.find_last_not_of('0') + 1, std::string::npos);
		result.erase(result.find_last_not_of('.') + 1, std::string::npos);
		
		return result;
	}

	class JsonBuilder {
		std::ostringstream m_Stream;

	public:
		JsonBuilder& StartObject() {
			m_Stream << "{";
			return *this;
		}

		JsonBuilder& EndObject() {
			m_Stream << "}";
			return *this;
		}

		JsonBuilder& StartArray() {
			m_Stream << "[";
			return *this;
		}

		JsonBuilder& EndArray() {
			m_Stream << "]";
			return *this;
		}

		JsonBuilder& AddSeparator() {
			m_Stream << ",";
			return *this;
		}

		JsonBuilder& AddField(const std::string& key, const std::string& value) {
			m_Stream << "\"" << key << "\":\"" << EscapeJsonString(value) << "\"";
			return *this;
		}

		JsonBuilder& AddField(const std::string& key, bool value) {
			m_Stream << "\"" << key << "\":" << (value ? "true" : "false");
			return *this;
		}

		JsonBuilder& AddField(const std::string& key, double value) {
			m_Stream << "\"" << key << "\":" << FormatNumber(value);
			return *this;
		}

		JsonBuilder& AddKey(const std::string& key) {
			m_Stream << "\"" << key << "\":";
			return *this;
		}

		JsonBuilder& AddString(const std::string& value) {
			m_Stream << "\"" << EscapeJsonString(value) << "\"";
			return *this;
		}

		JsonBuilder& AddBool(bool value) {
			m_Stream << (value ? "true" : "false");
			return *this;
		}

		JsonBuilder& AddNumber(double value) {
			m_Stream << FormatNumber(value);
			return *this;
		}

		JsonBuilder& AddRaw(const std::string& raw_json) {
			m_Stream << raw_json;
			return *this;
		}

		std::string ToString() const {
			return m_Stream.str();
		}
	};
}

std::string Aptabase::Event::ToJson() const {
	JsonBuilder builder;
	
	builder.StartObject()
		.AddField("timeStamp", TimeStamp)
		.AddSeparator()
		.AddField("sessionId", SessionId)
		.AddSeparator()
		.AddField("eventName", EventName)
		.AddSeparator()
		.AddKey("systemProps")
		.StartObject()
			.AddField("isDebug", SystemProps.IsDebug)
			.AddSeparator()
			.AddField("locale", SystemProps.Locale)
			.AddSeparator()
			.AddField("appVersion", SystemProps.AppVersion)
			.AddSeparator()
			.AddField("sdkVersion", SystemProps.SdkVersion)
			.AddSeparator()
			.AddField("osName", SystemProps.OsName)
			.AddSeparator()
			.AddField("osVersion", SystemProps.OsVersion)
		.EndObject()
		.AddSeparator()
		.AddKey("props")
		.StartObject();
	
		for (size_t i = 0; i < EventAttributes.size(); ++i) {
			const auto& attribute = EventAttributes[i];
			const bool is_last = (i == EventAttributes.size() - 1);
		
			builder.AddKey(attribute.Key);
		
			if (const std::string* value = std::get_if<std::string>(&attribute.Value)) {
				builder.AddString(*value);
			}

			if (const float* value = std::get_if<float>(&attribute.Value)) {
				builder.AddNumber(static_cast<double>(*value));
			}

			if (const double* value = std::get_if<double>(&attribute.Value)) {
				builder.AddNumber(*value);
			}
		
			if (!is_last) builder.AddSeparator();
		}
	
		builder.EndObject()
	.EndObject();
	
	return builder.ToString();
}

std::string Aptabase::Event::ToJson(const std::vector<Event>& events) {
	JsonBuilder builder;
	builder.StartArray();
	
	for (std::size_t i = 0; i < events.size(); ++i) {
		builder.AddRaw(events[i].ToJson());

		if (i < events.size() - 1) builder.AddSeparator();
	}
	
	builder.EndArray();
	return builder.ToString();
}