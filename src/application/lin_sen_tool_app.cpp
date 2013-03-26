#include "lin_sen_tool_app.h"

#ifdef HAVE_MAVLINK_H

#ifdef HAVE_GL_GLUT_H

#include "core/logger.h"
#include "core/datacenter.h"
#include "utility.h"

#include "lib/opengl/array_1d.h"

typedef struct { int x, y; } xy;

using namespace cpp_pthread;
using namespace hub::opengl::array_1d;

namespace mavhub {

LinSenToolApp::LinSenToolApp(const Logger::log_level_t loglevel) :
	AppInterface("lin_sen_tool_app", loglevel),
	AppLayer<mavlink_message_t>("lin_sen_tool_app", loglevel)
	{
	    pthread_mutex_init(&tx_mav_mutex, NULL);
        
        /* send request to hardware */
	    send_ctrl(LIN_SEN_DEBUG_RAW_LINE, 1);
	    /* tell the openGL map that data is requested */
		Array1D::params[Array1D::SHOW_RAW].value = 1;
}

LinSenToolApp::~LinSenToolApp() {}

void LinSenToolApp::handle_input(const mavlink_message_t &msg) {
 	log("LinSenToolApp got mavlink_message", static_cast<int>(msg.msgid), Logger::LOGLEVEL_DEBUG);

	switch(msg.msgid) {
		case MAVLINK_MSG_ID_HUCH_LIN_SEN_RAW: {
			mavlink_huch_lin_sen_raw_t raw_data;
			mavlink_msg_huch_lin_sen_raw_decode(&msg, &raw_data);
	
			Lock buf_lock(buf_mutex);
			uint16_t buffer[128];
			int k = 0;
			for (int i = 0; i < MAVLINK_MSG_HUCH_LIN_SEN_RAW_FIELD_DATA_LEN;) {
				buffer[k++] = raw_data.data[i] >> 4;
				buffer[k++] = (raw_data.data[i] << 8 | raw_data.data[i+1] >> 8) & 0x0FFF;
				buffer[k++] = (raw_data.data[i+1] << 4 | raw_data.data[i+2] >> 12) & 0x0FFF;
				buffer[k++] = (raw_data.data[i+2]) & 0x0FFF;
				i += 3; 
			}
			Array1D::addData(buffer, k);
			log("respond raw data ", k, Logger::LOGLEVEL_DEBUG);

			/* update time */
			Array1D::params[Array1D::TIME].status = Array1D::COMPLETE;
			Array1D::params[Array1D::TIME].value = raw_data.msec;
			break;
		}
		case MAVLINK_MSG_ID_HUCH_LIN_SEN_CTRL: {
			mavlink_huch_lin_sen_ctrl_t ctrl;
			mavlink_msg_huch_lin_sen_ctrl_decode(&msg, &ctrl);
	
			Lock buf_lock(buf_mutex);
			switch (ctrl.cmd) {
				case LIN_SEN_AVG_GET: {
					Array1D::params[Array1D::AVG].status = Array1D::COMPLETE;
					Array1D::params[Array1D::AVG].value = ctrl.arg;
					log("respond averrage ", ctrl.arg, Logger::LOGLEVEL_DEBUG);
					break;
				}
				case LIN_SEN_EXPOSURE_GET: {
					Array1D::params[Array1D::EXPOSURE].status = Array1D::COMPLETE;
					Array1D::params[Array1D::EXPOSURE].value = ctrl.arg;
					log("respond exposure ", ctrl.arg, Logger::LOGLEVEL_DEBUG);
					break;
				}
				case LIN_SEN_PIX_CLK_GET: {
					Array1D::params[Array1D::PIX_CLK].status = Array1D::COMPLETE;
					Array1D::params[Array1D::PIX_CLK].value = ctrl.arg;
					log("respond pixel clock ", ctrl.arg, Logger::LOGLEVEL_DEBUG);
					break;
				}
				case LIN_SEN_DEBUG: {
					Array1D::params[Array1D::DEBUG].status = Array1D::COMPLETE;
					Array1D::params[Array1D::DEBUG].value = ctrl.arg;
					log("respond debug ", ctrl.arg, Logger::LOGLEVEL_DEBUG);
					break;
				}
				case LIN_SEN_GAIN_GET: {
					Array1D::params[Array1D::GAIN].status = Array1D::COMPLETE;
//					Array1D::params[Array1D::GAIN].value = ctrl.arg;
					log("respond gain ", ctrl.arg, Logger::LOGLEVEL_DEBUG);
					break;
				}
				case LIN_SEN_ALGO_GET: {
					Array1D::params[Array1D::ALGO].status = Array1D::COMPLETE;
					Array1D::params[Array1D::ALGO].value = ctrl.arg;
					log("respond algorithm ", ctrl.arg, Logger::LOGLEVEL_DEBUG);
					break;
				}
				case LIN_SEN_RESULT: {
					Array1D::params[Array1D::RESULT].status = Array1D::COMPLETE;
					Array1D::params[Array1D::RESULT].value = ctrl.arg;
					log("respond result ", ctrl.arg, Logger::LOGLEVEL_DEBUG);
					break;
				}
				default:;
			}
			/* update time */
			Array1D::params[Array1D::TIME].status = Array1D::COMPLETE;
			Array1D::params[Array1D::TIME].value = ctrl.msec;
			break;
		}
		case MAVLINK_MSG_ID_OPTICAL_FLOW: {
			mavlink_optical_flow_t oflow;
			mavlink_msg_optical_flow_decode(&msg, &oflow);
	
			Lock buf_lock(buf_mutex);

			log("got mavlink oflow packet ", oflow.flow_x, Logger::LOGLEVEL_DEBUG);
			Array1D::params[Array1D::RESULT].value = oflow.flow_x;
			if (Array1D::params[Array1D::RESULT].status == Array1D::WAITING) {
				Array1D::params[Array1D::RESULT].status = Array1D::COMPLETE;
			}			
			
			/* update time */
			Array1D::params[Array1D::TIME].status = Array1D::COMPLETE;
			Array1D::params[Array1D::TIME].value = oflow.time_usec;
			break;
		}
		default:;
	}
}

void LinSenToolApp::print(std::ostream &os) const {
	AppLayer<mavlink_message_t>::print(os);
}

void LinSenToolApp::run() {
	log("LinSenToolApp running", Logger::LOGLEVEL_DEBUG);
	Array1D array(Core::argc, Core::argv);

	while( !interrupted() ) {
		{
			Lock buf_lock(buf_mutex);
			
			// exposure 
			if (Array1D::params[Array1D::EXPOSURE].status == Array1D::PENDING) {
				Array1D::params[Array1D::EXPOSURE].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_EXPOSURE_GET, 0);
				log("send get exposure", Logger::LOGLEVEL_DEBUG);
			} else if (Array1D::params[Array1D::EXPOSURE].status == Array1D::SENDING) {
				Array1D::params[Array1D::EXPOSURE].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_EXPOSURE_SET, array.params[Array1D::EXPOSURE].value);				
			}
			// pixel clock 
			if (Array1D::params[Array1D::PIX_CLK].status == Array1D::PENDING) {
				Array1D::params[Array1D::PIX_CLK].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_PIX_CLK_GET, 0);
				log("send get pixel clock", Logger::LOGLEVEL_DEBUG);
			} else if (Array1D::params[Array1D::PIX_CLK].status == Array1D::SENDING) {
				Array1D::params[Array1D::PIX_CLK].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_PIX_CLK_SET, array.params[Array1D::PIX_CLK].value);				
			}
			// gain 
			if (Array1D::params[Array1D::GAIN].status == Array1D::PENDING) {
				Array1D::params[Array1D::GAIN].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_GAIN_GET, 0);
				log("send get gain", Logger::LOGLEVEL_DEBUG);
			} else if (Array1D::params[Array1D::GAIN].status == Array1D::SENDING) {
				Array1D::params[Array1D::GAIN].status = Array1D::COMPLETE;
				send_ctrl(LIN_SEN_GAIN_SET, array.params[Array1D::GAIN].value);				
			}
			// debug 
			if (Array1D::params[Array1D::DEBUG].status == Array1D::PENDING) {
				Array1D::params[Array1D::DEBUG].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_DEBUG, 0);
				log("send get debug", Logger::LOGLEVEL_DEBUG);
			}
			// result
			if (Array1D::params[Array1D::RESULT].status == Array1D::PENDING) {
				Array1D::params[Array1D::RESULT].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_RESULT, 1);
				log("send get result", Logger::LOGLEVEL_DEBUG);			
			} else if (Array1D::params[Array1D::RESULT].status == Array1D::SENDING) {
				Array1D::params[Array1D::RESULT].status = Array1D::COMPLETE;
				send_ctrl(LIN_SEN_RESULT, array.params[Array1D::RESULT].value);				
				log("send set result:", array.params[Array1D::RESULT].value, Logger::LOGLEVEL_DEBUG);			
			}
			// time - use result as standard communication
			if (Array1D::params[Array1D::TIME].status == Array1D::PENDING) {
				Array1D::params[Array1D::TIME].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_RESULT, 1);
				log("send get time", Logger::LOGLEVEL_DEBUG);
			}
			// avg
			if (Array1D::params[Array1D::AVG].status == Array1D::PENDING) {
				Array1D::params[Array1D::AVG].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_AVG_GET, 0);
				log("send get average", Logger::LOGLEVEL_DEBUG);
			} else if (Array1D::params[Array1D::AVG].status == Array1D::SENDING) {
				Array1D::params[Array1D::AVG].status = Array1D::COMPLETE;
				send_ctrl(LIN_SEN_AVG_SET, array.params[Array1D::AVG].value);				
			}

			// algo 
			if (Array1D::params[Array1D::ALGO].status == Array1D::PENDING) {
				Array1D::params[Array1D::ALGO].status = Array1D::WAITING;
				send_ctrl(LIN_SEN_ALGO_GET, 0);
				log("send get algorithm", Logger::LOGLEVEL_DEBUG);
			} else if (Array1D::params[Array1D::ALGO].status == Array1D::SENDING) {
				Array1D::params[Array1D::ALGO].status = Array1D::CLEAR;
				send_ctrl(LIN_SEN_ALGO_SET, array.params[Array1D::ALGO].value);				
				log("send set algorithm ", array.params[Array1D::ALGO].value, Logger::LOGLEVEL_DEBUG);
			}
			// show raw
			if (Array1D::params[Array1D::SHOW_RAW].status == Array1D::SENDING) {
				Array1D::params[Array1D::SHOW_RAW].status = Array1D::COMPLETE;
			    send_ctrl(LIN_SEN_DEBUG_RAW_LINE, array.params[Array1D::SHOW_RAW].value);
				log("show raw", Logger::LOGLEVEL_DEBUG);
			}
			Array1D::display();
		}
		usleep(1000);
	}

	log("LinSenToolApp stop running", Logger::LOGLEVEL_DEBUG);
}

void LinSenToolApp::send_ctrl(HUCH_LIN_SEN_CTRL_CMD cmd, uint16_t arg) {
    static mavlink_huch_lin_sen_ctrl_t huch_lin_sen_ctrl;
        huch_lin_sen_ctrl.msec = 0;
		huch_lin_sen_ctrl.cmd = cmd;
        huch_lin_sen_ctrl.arg = arg; 
		mavlink_msg_huch_lin_sen_ctrl_encode(system_id(),
			component_id,
			&tx_mav_msg,
			&huch_lin_sen_ctrl
		);
		AppLayer<mavlink_message_t>::send(tx_mav_msg);    
}

void LinSenToolApp::send_heartbeat() {
	Lock tx_lock(tx_mav_mutex);
	mavlink_msg_heartbeat_pack(system_id(), component_id, &tx_mav_msg, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_INVALID, MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 0, MAV_STATE_ACTIVE);
	AppLayer<mavlink_message_t>::send(tx_mav_msg);
}


} // namespace mavhub

#endif // HAVE_GL_GLUT_H
#endif // HAVE_MAVLINK_H
