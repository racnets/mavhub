#include "factory_app.h"

#include "protocollayer.h"

#include "module/coremod.h"
#include "module/testcore.h"
#include "module/fc_mpkg.h"
#include "module/ctrl_hover.h"
#include "app/mk_app.h"

#include <iostream>

using namespace std;

namespace mavhub {

	// ----------------------------------------------------------------------------
	// AppFactory
	// ----------------------------------------------------------------------------
	AppLayer* AppFactory::build(const std::string& app_name, const std::map<std::string, std::string>& args) {
		//transform application name to lower case
		std::string lowercase_name(app_name);
		transform(lowercase_name.begin(), lowercase_name.end(), lowercase_name.begin(), ::tolower);

		//get loglevel
		std::map<std::string,std::string>::const_iterator find_iter = args.find("loglevel");
		Logger::log_level_t loglevel;
		if( find_iter != args.end() ) {
			istringstream istream(find_iter->second);
			istream >> loglevel;
		} else { //no loglevel given, set default
			Logger::log("No loglevel given for", app_name, Logger::LOGLEVEL_DEBUG);
			loglevel = Logger::LOGLEVEL_WARN;
		}

		if(lowercase_name == "test_app") {
			return new TestCore();
		} else if(lowercase_name == "core_app") {
			return new CoreModule();
		} else if(lowercase_name == "fc_mpkg_app") {
			int component_id = 0;
			std::map<std::string,std::string>::const_iterator iter = args.find("component_id");
			if( iter != args.end() ) {
				istringstream s(iter->second);
				s >> component_id;
			}
			return new FC_Mpkg(component_id);
		} else if(lowercase_name == "ctrl_hover_app") {
			// pass only configuration map into constructor
			return new Ctrl_Hover(args);
		} else if(lowercase_name == "mk_app") {
			std::string dev_name;
			find_iter = args.find("device");
			if(find_iter != args.end()) {
				dev_name = find_iter->second;
			} else {
				dev_name = "/dev/ttyS0";
			}
			unsigned int baudrate(57600);
			find_iter = args.find("baudrate");
			if(find_iter != args.end()) {
				istringstream istream(find_iter->second);
				istream >> baudrate;
			}
			return new MKApp(loglevel, dev_name, baudrate);
		}
	
		return NULL;
	}
}
