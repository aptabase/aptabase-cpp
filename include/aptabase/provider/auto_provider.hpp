#pragma once

#include "aptabase/provider/manual_provider.hpp"

namespace Aptabase{

	class AutoProvider: public ManualProvider{
	public:
		AutoProvider(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &url);

		void RecordEvent(Event &&event)override;
	};

}//namespace Aptabase::