#include "sensor_factory.h"
#include "utility.h"

#include "core/logger.h"

#include <sstream> //stringstream
#include <iterator> //istream_iterator

#include <list>

using namespace std;

namespace mavhub {

	
	// ----------------------------------------------------------------------------
	// SensorFactory
	// ----------------------------------------------------------------------------
	void SensorFactory::build(const std::string& sensor_name, const std::map<std::string, std::string> args) {
		// standard sensor parameters
		unsigned short address = 0;
		unsigned short dev_id = 0;
		unsigned short comp_id = 0;
		std::string port;
		int update_rate = 0;
		int debug = 0;
		int timings = 0; 
		std::list< std::pair<int, int> > chanmap_pairs;
		// sensor specific paramterers
		// bmp085
		int update_rate_temp = 0;
		unsigned short comp_id1 = 0;
		unsigned short comp_id2 = 0;
		// sensor specific paramterers
		// hmc5843
		int gain = 0;
		int mode = 0;
		// sensor specific paramterers
		// huch-linSen
		unsigned short exposure = 0;
		unsigned short pixelclock = 0;		
		
		for(std::map<std::string, std::string>::const_iterator iter = args.begin(); iter != args.end(); ++iter) {
			// standard sensor parameters 
			if (iter->first == "dev_id") std::istringstream(iter->second) >> std::hex >> dev_id;
			else if (iter->first == "comp_id") std::istringstream(iter->second) >> std::hex >> comp_id;
			else if (iter->first == "func_id") std::istringstream(iter->second) >> std::hex >> comp_id;
			else if (iter->first == "address") std::istringstream(iter->second) >> std::hex >> address;
			else if (iter->first == "port") port = iter->second;
			else if (iter->first == "update_rate") std::istringstream(iter->second) >> update_rate;
			else if (iter->first == "debug") std::istringstream(iter->second) >> debug;
			else if (iter->first == "timings") std::istringstream(iter->second) >> timings;
			else if (iter->first == "chanmap") {
				istringstream s(iter->second);
				s >> chanmap_pairs;
				// Logger::debug("blub");
				// Logger::debug(s);
			}
			// sensor specific paramterers
			// bmp085
			else if (iter->first == "comp_id1") std::istringstream(iter->second) >> std::hex >> comp_id1;
			else if (iter->first == "comp_id2") std::istringstream(iter->second) >> std::hex >> comp_id2;
			else if (iter->first == "update_rate_temp") std::istringstream(iter->second) >> update_rate_temp;
			// sensor specific paramterers
			// hmc5843
			else if (iter->first == "gain") std::istringstream(iter->second) >> gain;
			else if (iter->first == "mode") std::istringstream(iter->second) >> mode;
			// sensor specific paramterers
			// huch-linSen
			else if (iter->first == "exposure") std::istringstream(iter->second) >> exposure;
			else if (iter->first == "pixelclock") std::istringstream(iter->second) >> pixelclock;

			// Logger::debug(iter->first);
			// Logger::debug(iter->second);
		}

		/* create instance */
		Sensor *s = NULL;
		try {
			if (sensor_name == "bmp085") 
				s = new SenBmp085(dev_id, comp_id, comp_id1, comp_id2, port, update_rate, debug, timings, update_rate_temp);
			else if (sensor_name == "hmc5843") 
				s = new SenHmc5843(dev_id, comp_id, port, update_rate, debug, timings, gain, mode);
			else if (sensor_name == "exp_ctrl")
				s = new SenExpCtrl(dev_id, comp_id, port, update_rate, debug, timings, chanmap_pairs);
			else if (sensor_name == "srf02")
				s = new SenSrf02(dev_id, comp_id, port, update_rate, debug, timings, chanmap_pairs);
			else if (sensor_name == "cmp02")
				s = new SenCmp02(dev_id, comp_id, port, update_rate, debug, timings, chanmap_pairs);
			else if (sensor_name == "huch-linSen")
				s = new SenHuchLinSen(dev_id, comp_id, port, address, update_rate, debug, timings, exposure, pixelclock);
		}
		catch (const char *message) {
			std::string s(message);
			s = "in sensor factory: " + s;
			Logger::error(s.c_str());
		}
		if (s != NULL) SensorManager::instance().add_sensor(s,dev_id);
	}
}
