#pragma once

#include "aptabase/model.hpp"
#include "aptabase/net/client.hpp"

enum class AptabaseProviderType {
	Worker,
	Tick
};

enum class AptabaseProviderVerbosity {
	Verbose,
	Display,
	Info,
	Warning,
	Error
};

extern const char *ToString(AptabaseProviderVerbosity verbosity);

class AptabaseProvider {
public:	
	using LogFunctionType = std::function<void(AptabaseProviderVerbosity, const std::string &)>;
public:
	static void DefaultLogFunction(AptabaseProviderVerbosity verbosity, const std::string &message);

	virtual ~AptabaseProvider() = default;

	virtual void RecordEvent(AptabaseEventPayload &&event) = 0;

	virtual void Flush() = 0;

	virtual bool AnyPending()const = 0;

	virtual bool AnySending()const = 0;

	virtual void Tick(){ (void)0; }

	virtual bool IsTickRequired()const = 0;

	virtual void SetLog(LogFunctionType &&log);
};