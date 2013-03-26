#ifndef _HUCHLINSEN_H_
#define _HUCHLINSEN_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_MAVLINK_H
#include <mavlink.h>
#endif // HAVE_MAVLINK_H

#include <inttypes.h>

#include "i2csensor.h"

#define	HUCHLINSEN_ADR	0x18

#define CRA	0x00
#define CRB	0x01
#define MR	0x02
#define DATA	0x03
#define SR	0x09

/* data rates */
#define DR05HZ	0
#define DR1HZ	1
#define DR2HZ	2
#define DR5HZ	3
#define DR10HZ	4
#define DR20HZ	5
#define DR50HZ	6
#define DR100HZ	7
#define DRDEFAULT DR10HZ

/* sensor input range */
#define GN07A	0
#define GN10A	1
#define GN15A	2
#define GN20A	3
#define GN32A	4
#define GN38A	5
#define GN45A	6
#define GN65A	7
#define GNDEFAULT GN10A

/* mode */
#define CONTINUOUS_CONVERSION_MODE	0
#define SINGLE_CONVERSION_MODE	1
#define IDLE_MODE	2
#define SLEEP_MODE	3


namespace mavhub {

	class HuchLinSen : public I2cSensor {
		public:
			HuchLinSen(unsigned short _dev_id, unsigned short _func_id, std::string _port, int _update_rate, int _debug, int _timings, int _gain, int _mode) throw(const char *);
			virtual ~HuchLinSen();
			void print_debug();

		protected:
			virtual void run();
			virtual void* get_data_pointer(unsigned int id) throw(const char *);
			/// copy data into datacenter
			void publish_data(uint64_t time);
			
		private:
#ifdef MAVLINK_ENABLED_HUCH
			mavlink_huch_magnetic_kompass_t kompass_data;
#endif // MAVLINK_ENABLED_HUCH

			int gain;
			int mode;

			/* frequenz */
			static const int waitFreq[8];

			/* gain factoy */
			static const int gainFactor[8];
	};

} // namespace mavhub

#endif
