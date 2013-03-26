#ifndef _FPGA_CAM_TOOL_APP_H_
#define _FPGA_CAM_TOOL_APP_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_MAVLINK_H
#ifdef HAVE_GL_GLUT_H

#include "protocol/protocollayer.h"

#include <inttypes.h> //uint8_t
#include <vector>

namespace mavhub {

	class FPGACamToolApp : public AppLayer<mavlink_message_t>
	{
		public:
			static const int component_id = 200;

			FPGACamToolApp(const Logger::log_level_t loglevel = Logger::LOGLEVEL_WARN);
			virtual ~FPGACamToolApp();

			virtual void handle_input(const mavlink_message_t &msg);

		protected:
			virtual void print(std::ostream &os) const;
			virtual void run();

		private:
			/// TX buffer for mavlink messages
			mavlink_message_t tx_mav_msg;
			/// Mutex to protect tx_mav_msg
			pthread_mutex_t tx_mav_mutex;
			pthread_mutex_t buf_mutex;

			void send_heartbeat();
	};

} // namespace mavhub

#endif // HAVE_GL_GLUT_H
#endif // HAVE_MAVLINK_H
#endif // _FPGA_CAM_TOOL_APP_H_
