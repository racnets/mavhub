#include "fpga_cam_tool_app.h"

#ifdef HAVE_MAVLINK_H

#ifdef HAVE_GL_GLUT_H

#include "core/logger.h"
#include "core/datacenter.h"
#include "utility.h"

#include "lib/opengl/array_2d.h"

typedef struct { int x, y; } xy;

using namespace cpp_pthread;
using namespace hub::opengl::array_2d;

namespace mavhub {

FPGACamToolApp::FPGACamToolApp(const Logger::log_level_t loglevel) :
	AppInterface("fpga_cam_tool_app", loglevel),
	AppLayer<mavlink_message_t>("fpga_cam_tool_app", loglevel)
	{
	    pthread_mutex_init(&tx_mav_mutex, NULL);        
}

FPGACamToolApp::~FPGACamToolApp() {}

void FPGACamToolApp::handle_input(const mavlink_message_t &msg) {
	static int data_transmission_size;
 	log("FPGACamToolApp got mavlink_message", static_cast<int>(msg.msgid), Logger::LOGLEVEL_DEBUG);

	switch(msg.msgid) {
		case MAVLINK_MSG_ID_DATA_TRANSMISSION_HANDSHAKE: {
			mavlink_data_transmission_handshake_t mav_data_handshake;
			mavlink_msg_data_transmission_handshake_decode(&msg, &mav_data_handshake);
			log("FPGACamToolApp got data transmission handshake message", static_cast<int>(mav_data_handshake.packets), Logger::LOGLEVEL_DEBUG);

			Array2D::setupImage(mav_data_handshake.width, mav_data_handshake.height, mav_data_handshake.packets);
			data_transmission_size = mav_data_handshake.payload;
			break;
		}
		case MAVLINK_MSG_ID_ENCAPSULATED_DATA: {
			mavlink_encapsulated_data_t data;
			mavlink_msg_encapsulated_data_decode(&msg, &data);
	
			log("FPGACamToolApp got encapsuled data message", static_cast<int>(data.seqnr), Logger::LOGLEVEL_DEBUG);
				
			Array2D::addImageData(data.data, data_transmission_size, data.seqnr);
			break;
		}
		default:;
	}
}

void FPGACamToolApp::print(std::ostream &os) const {
	AppLayer<mavlink_message_t>::print(os);
}

void FPGACamToolApp::run() {
	log("FPGACamToolApp running", Logger::LOGLEVEL_DEBUG);

	Array2D array(Core::argc, Core::argv);

	while( !interrupted() ) {
		if ( Array2D::status == Array2D::CLEAR || Array2D::status == Array2D::COMPLETE ) {
			Array2D::status = Array2D::PENDING;
			Lock tx_lock(tx_mav_mutex);
			mavlink_msg_data_transmission_handshake_pack(system_id(), component_id, &tx_mav_msg, DATA_TYPE_RAW_IMAGE, 0, 0, 0, 0, 0, 0);
			AppLayer<mavlink_message_t>::send(tx_mav_msg);
		}
			
		Array2D::display();
		usleep(10000);
	}

	{
		Lock tx_lock(tx_mav_mutex);
		mavlink_msg_data_transmission_handshake_pack(system_id(), component_id, &tx_mav_msg, 0, 0, 0, 0, 0, 0, 0);
		AppLayer<mavlink_message_t>::send(tx_mav_msg);
	}

	log("FPGACamToolApp stop running", Logger::LOGLEVEL_DEBUG);
}

void FPGACamToolApp::send_heartbeat() {
	Lock tx_lock(tx_mav_mutex);
	mavlink_msg_heartbeat_pack(system_id(), component_id, &tx_mav_msg, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_INVALID, MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 0, MAV_STATE_ACTIVE);
	AppLayer<mavlink_message_t>::send(tx_mav_msg);
}


} // namespace mavhub

#endif // HAVE_GL_GLUT_H
#endif // HAVE_MAVLINK_H
