#ifndef _LIN_SEN_TOOL_APP_H_
#define _LIN_SEN_TOOL_APP_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_MAVLINK_H

#ifdef HAVE_GL_GLUT_H

#include "protocol/protocollayer.h"

#include <inttypes.h> //uint8_t
#include <vector>

namespace mavhub {

	class LinSenToolApp : public AppLayer<mavlink_message_t>
	{
		public:
			static const int component_id = 200;

			LinSenToolApp(const Logger::log_level_t loglevel = Logger::LOGLEVEL_WARN);
			virtual ~LinSenToolApp();

			virtual void handle_input(const mavlink_message_t &msg);

		protected:
			virtual void print(std::ostream &os) const;
			virtual void run();

		private:
			/// TX buffer for mavlink messages
			mavlink_message_t tx_mav_msg;
			/// Mutex to protect tx_mav_msg
			pthread_mutex_t tx_mav_mutex;
			unsigned int width;
			unsigned int height;
			char buffer[921600];
			pthread_mutex_t buf_mutex;
			
			void send_heartbeat();
			void processCtrl(mavlink_huch_lin_sen_ctrl_t *ctrl);
            void send_ctrl(HUCH_LIN_SEN_CTRL_CMD cmd, uint16_t arg);
            void analyseData(uint16_t *datap, uint8_t size);
	};

} // namespace mavhub

#endif // HAVE_GL_GLUT_H
#endif // HAVE_MAVLINK_H
#endif // _LIN_SEN_TOOL_APP_H_
