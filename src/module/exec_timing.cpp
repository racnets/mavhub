#include "exec_timing.h"
#include "core/logger.h"

using namespace std;

namespace mavhub {
	// Exec_Timing::Exec_Timing(int order_, vector<double> &coeff) {
	Exec_Timing::Exec_Timing(int rate) {
		dt = 0;
		wait_freq = rate ? 1000000 / rate : 0;
		wait_time = wait_freq;
		frequency = wait_time;
		start = get_time_us(); // from utility
		usec = 0;
		gettimeofday(&tk, NULL);
		gettimeofday(&tkm1, NULL);
	}

	Exec_Timing::~Exec_Timing() {
		
	}

	int Exec_Timing::calcSleeptime() {
		usec = get_time_us();
		end = usec;
		wait_time = wait_freq - (end - start);

		// Logger::log("Exec_Timing: wait_freq", wait_freq, Logger::LOGLEVEL_INFO);
		// Logger::log("Exec_Timing: end, start", end, start, Logger::LOGLEVEL_INFO);
		// Logger::log("Exec_Timing: usec, wait_time", usec, wait_time, Logger::LOGLEVEL_INFO);

		// FIXME: adaptive timing on demand?
		if(wait_time < 0) {
			Logger::log("ALARM: time", Logger::LOGLEVEL_INFO);
			wait_time = 0;
		}
		return wait_time;
	}

	uint64_t Exec_Timing::updateExecStats() {
		/* calculate frequency */
		end = get_time_us();
		frequency = (15 * frequency + end - start) / 16;
		start = end;

		// Logger::log("Ctrl_Hover slept for", wait_time, Logger::LOGLEVEL_INFO);

		gettimeofday(&tk, NULL);
		//timediff(tdiff, tkm1, tk);
		dt = (tk.tv_sec - tkm1.tv_sec) * 1000000 + (tk.tv_usec - tkm1.tv_usec);
		tkm1 = tk; // save current time
		return dt;
	}

}
