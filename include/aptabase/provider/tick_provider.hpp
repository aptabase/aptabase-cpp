#pragma once

#include "aptabase/provider/manual_provider.hpp"

namespace Aptabase{

	class TickProvider: public ManualProvider{
	public:
		TickProvider(std::unique_ptr<HttpClient> &&client, const std::string &app_key, const std::string &url);
	
		void Tick()override;

		bool IsTickRequired()const override{ return true; }
	};

}//namespace Aptabase::