#include "aptabase/provider/provider.hpp"
#include "aptabase/provider/tick_provider.hpp"
#include "aptabase/provider/worker_provider.hpp"
#include <iostream>

const char *ToString(AptabaseProviderVerbosity verbosity) {
	switch (verbosity) {
	case AptabaseProviderVerbosity::Verbose: return "Verbose";
	case AptabaseProviderVerbosity::Display: return "Display";
	case AptabaseProviderVerbosity::Info: return "Info";
	case AptabaseProviderVerbosity::Warning: return "Warning";
	case AptabaseProviderVerbosity::Error: return "Error";
	}

	return "Unknown";
}
void AptabaseProvider::DefaultLogFunction(AptabaseProviderVerbosity verbosity, const std::string& message) {
	std::cout << ToString(verbosity) << ":" << message << '\n';
}
