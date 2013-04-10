#include "senhuchLinSen.h"

#include <math.h> //pow
#include <iostream> //cout

#include "core/logger.h" //"printf"
#include "utility.h"
#include "core/datacenter.h"

using namespace std;

namespace mavhub {

	SenHuchLinSen::SenHuchLinSen(unsigned short _dev_id, unsigned short _comp_id, string _port, unsigned short _address, int _update_rate, int _debug, int _timings, int _exposure, int _pixel_clock) throw(const char *) {
		//FIXME Initialisierung
		dev_id = _dev_id;
		comp_id = _comp_id;
		update_rate = _update_rate;
		adr = _address;
		debug = _debug;
		timings = _timings; 

		// limit parameters to valid values
		// 36µs <= exposure <= 100ms
		if (_exposure < 36) exposure = 36;
		else if (_exposure > 100000) exposure = 100000;
		else exposure = _exposure;
		
		// 5kHz <= pixel clock <= 8Mhz
		if (_pixel_clock < 5) pixelclock = 5;
		else if (_pixel_clock > 8000) pixelclock = 8000;
		else pixelclock = _pixel_clock;
		
		uint8_t buffer[2];

		Logger::debug("huch-linSen: init...");

		status = RUNNING;	
		try {
			/* check config */
			if (get_data_pointer(comp_id << 16) == NULL) {
				throw "unsupported component id";
			}

			fd = i2c_init(_port);
			/* pixel clock */
			i2c_start_conversion(fd, adr);
			buffer[0] = PCR;
			buffer[1] = pixelclock;

			i2c_write_bytes(fd, buffer, 2);
			i2c_end_conversion(fd);

			/* exposure */
			i2c_start_conversion(fd, adr);
			buffer[0] = ER;
			buffer[1] = exposure;

			i2c_write_bytes(fd, buffer, 2);
			i2c_end_conversion(fd);

			/* get component and unique device id */
			i2c_start_conversion(fd, adr);
			i2c_read_bytes(fd, buffer, 2);
			i2c_end_conversion(fd);
						
			if (buffer[0] != comp_id) Logger::log("component id mismatch", Logger::LOGLEVEL_WARN);
			oflow.sensor_id = buffer[1];
			
			Logger::debug("done");
			status = STOPPED;
		}
		catch(const char *message) {
			status = UNINITIALIZED;
			i2c_end_conversion(fd);

			string s(message);
			throw ("SenHuchLinSen(): " + s).c_str();
		}
	}

	SenHuchLinSen::~SenHuchLinSen() {
		status = STOPPED;
		join();
	}

	void SenHuchLinSen::run() {
		uint8_t buffer[6];
		/* 1s / update rate in Hz */
		int wait_time = 1000000 / update_rate;

		/* start time */
		uint64_t start = get_time_us();
		/* needed for timing */
		uint64_t frequency = wait_time;
		/* needed for timing output: every second */
		uint64_t time_output = start + 1000000;

		Logger::debug("huch-linSen: running");
		try {
			status = RUNNING;

			while(status == RUNNING) {
				/* set address to read from */
				i2c_start_conversion(fd, adr);
				buffer[0] = RR;
				i2c_write_bytes(fd, buffer, 1);
				i2c_end_conversion(fd);
				
				/* read result */
				i2c_start_conversion(fd, adr);
				i2c_read_bytes(fd, buffer, 2);
				i2c_end_conversion(fd);
				
				/* wait time */
				uint64_t end = get_time_us();
				usleep(wait_time - (end - start));

				/* calculate frequency */
				end = get_time_us();
				frequency = (15 * frequency + end - start) / 16;
				start = end;

#ifdef MAVLINK_ENABLED_HUCH
				/* assign buffer to data */
				{ // begin of data mutex scope
					cpp_pthread::Lock ri_lock(data_mutex);
					oflow.time_usec = end;
					oflow.flow_x = (int16_t)((buffer[1] << 8) | buffer[0]);
				} // end of data mutex scope
				// not supported
				//publish_data(end);
#endif // MAVLINK_ENABLED_HUCH

				/* debug data */
				if (debug) print_debug();

				/* timings/benchmark output */
				if (timings) {
					if (time_output <= end) {
						Logger::log("huch-linSen frequency: ", (float)1000000/frequency, Logger::LOGLEVEL_DEBUG);
						//Logger::log("huch-linSen wait_time: ", wait_time, Logger::LOGLEVEL_DEBUG);
						time_output += 1000000;
					}
				}
			}
		}
		catch(const char *message) {
			i2c_end_conversion(fd);
			status = STRANGE;

			string s(message);
			throw ("SenHuchLinSen::run(): " + s).c_str();
		}
		/* unlock i2c */
		i2c_end_conversion(fd);

		Logger::debug("huch-linSen: stopped");
	}

	void SenHuchLinSen::print_debug() {
		//FIXME: remove variable send_stream (no extra allocation for debug messages)
		ostringstream send_stream;
#ifdef MAVLINK_ENABLED_HUCH
		send_stream << "huch-linSen: " << oflow.flow_x << " @:" << oflow.time_usec;
#else
		send_stream << "huch-linSen: mavlink missing";
#endif // MAVLINK_ENABLED_HUCH
		Logger::debug(send_stream.str());
	}

	void* SenHuchLinSen::get_data_pointer(unsigned int id) throw(const char *) {
		if (status == RUNNING) {
			switch ((0xFFFF0000 & id) >> 16) {
#ifdef MAVLINK_ENABLED_HUCH
			case OPTICAL_FLOW_SENSOR:
				return &oflow;
#endif // MAVLINK_ENABLED_HUCH
			default: 
				return NULL;
			}
		} throw "sensor huch-linSen isn't running";
	}

} // namespace mavh
