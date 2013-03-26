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

/* mode */
#define CONTINUOUS_CONVERSION_MODE	0
#define SINGLE_CONVERSION_MODE	1
#define IDLE_MODE	2
#define SLEEP_MODE	3

/* register */
#define CIR	0x00	/* component id register */
#define UIR 0x01	/* unique id register */
#define ER	0x21	/* exposure register */
#define PCR	0x22	/* pixel clock register */
#define RR  0x30	/* result register */

namespace mavhub {

	class SenHuchLinSen : public I2cSensor {
		public:
			SenHuchLinSen(unsigned short _dev_id, unsigned short _comp_id, std::string _port, unsigned short _address, int _update_rate, int _debug, int _timings, int _exposure, int _pixelclock) throw(const char *);
			virtual ~SenHuchLinSen();
			void print_debug();

		protected:
			virtual void run();
			virtual void* get_data_pointer(unsigned int id) throw(const char *);
			/// copy data into datacenter - not supported
			//void publish_data(uint64_t time);
			
		private:
			#ifdef MAVLINK_ENABLED_HUCH
				mavlink_optical_flow_t oflow;
			#endif // MAVLINK_ENABLED_HUCH
		
			int exposure;
			int pixelclock;
			unsigned char address;

			/* frequenz */
			static const int waitFreq[8];

			/* gain factoy */
			static const int gainFactor[8];
	};

} // namespace mavhub

#endif
