#pragma once

#include "aptabase/model.hpp"
#include "aptabase/net/client.hpp"

namespace Aptabase{

	enum class Verbosity {
		Verbose,
		Display,
		Info,
		Warning,
		Error
	};

	extern const char *ToString(Verbosity verbosity);

	class Provider {
	public:	
		using LogFunctionType = std::function<void(Verbosity, const std::string &)>;
	public:
		static void DefaultLogFunction(Verbosity verbosity, const std::string &message);

		virtual ~Provider() = default;

		virtual void RecordEvent(Event &&event) = 0;

		virtual void Flush() = 0;

		virtual bool AnyPending()const = 0;

		virtual bool AnySending()const = 0;

		virtual void SetLog(LogFunctionType &&log) = 0;
	};

}//namespace Aptabase::