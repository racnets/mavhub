#include "plat_link_crrcsim.h"

#ifdef HAVE_MAVLINK_H

#include "core/logger.h"
#include "utility.h"
#include "protocol/protocolstack.h"

//#include "opencv2/opencv.hpp"

#include <iostream> //cout
#include <stdlib.h>
using namespace std;
//using namespace cv;

namespace mavhub {
	Plat_Link_Crrcsim::Plat_Link_Crrcsim(const map<string, string> args) : 
		AppInterface("plat_link_crrcsim_app"),
		ModuleBase(args, "plat_link_crrcsim_app"),
		ctl_mode(0),
		ctl_mode_lat(0),
		thrust(0.0)
	{
		// try and set reasonable defaults
		conf_defaults();
		// initialize module parameters from conf
		read_conf(args);

		assign_variable_from_args(component_id);
		// assign_variable_from_args(output_enable);

		// init execution timer
		exec_tmr = new Exec_Timing((int)params["ctl_update_rate"]);

		// kp="0.066320932874519622"
		// ki="0.05339107055892655"
		// kd="0.018905589636341851"
		// measurements from simulation
		z = 0.0;
		z_hat = 0.0;
		m1 = 0.0;
		m2 = 0.0;
		// double scalef = 0.0;
		// double ctl_Kc = 0.01;
		// double ctl_Ti = 0.0;
		// double ctl_Td = 100.0;
		// ctl_Kc = 0.05;
		// ctl_Ti = 4.3538;
		// ctl_Td = 0.28882;
		pid_alt = new PID(params["ac_pid_bias"],
											params["ac_pid_Kc"],
											params["ac_pid_Ki"],
											params["ac_pid_Kd"]);
		pid_alt->setSp(params["ac_sp"]);
		pid_alt->setPv_int_lim(params["ac_pid_int_lim"]);

		ctl.target = 0;
		ctl.roll = 0.;
		ctl.pitch = 0.;
		ctl.yaw = 0.;
		ctl.thrust = 0.;
		ctl.mask = 0;
		// Bumper
		// bump = new Bumper(params["bump_thr_low"], params["bump_thr_high"]);
		// bump_lat = new Bumper(0, 0.01);
		// ffnet
		// ffnet = new N_FF();
	}

	Plat_Link_Crrcsim::~Plat_Link_Crrcsim() {}

	void Plat_Link_Crrcsim::handle_input(const mavlink_message_t &msg) {
		mavlink_huch_sensor_array_t sa;
		//int i;
		static char param_id[16];

		switch(msg.msgid) {
		case MAVLINK_MSG_ID_HEARTBEAT:
			// Logger::log("Plat_Link_Crrcsim got mavlink heartbeat, (msgid, sysid)", (int)msg.msgid, (int)msg.sysid, Logger::LOGLEVEL_INFO);
			break;

		case MAVLINK_MSG_ID_ATTITUDE:
			phi = mavlink_msg_attitude_get_roll(&msg);
			theta = mavlink_msg_attitude_get_pitch(&msg);
			psi = mavlink_msg_attitude_get_yaw(&msg);
			break;

		case MAVLINK_MSG_ID_LOCAL_POSITION_NED:
			x = mavlink_msg_local_position_ned_get_x(&msg);
			y = mavlink_msg_local_position_ned_get_y(&msg);
			z = mavlink_msg_local_position_ned_get_z(&msg);
			break;

		case MAVLINK_MSG_ID_HUCH_SENSOR_ARRAY:
			mavlink_msg_huch_sensor_array_decode(&msg, &sa);
			m1 = sa.data[0];
			m2 = sa.data[1];
			z_hat = (sa.data[0] + sa.data[1]) * 0.5;
			break;

		case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
			Logger::log("Plat_Link_Crrcsim::handle_input: PARAM_REQUEST_LIST", Logger::LOGLEVEL_INFO);
			if(mavlink_msg_param_request_list_get_target_system (&msg) == system_id()) {
				param_request_list = true;
			}
			break;

		case MAVLINK_MSG_ID_PARAM_SET:
			if(mavlink_msg_param_set_get_target_system(&msg) == system_id()) {
				Logger::log("Plat_Link_Crrcsim::handle_input: PARAM_SET for this system", (int)system_id(), Logger::LOGLEVEL_INFO);
				if(mavlink_msg_param_set_get_target_component(&msg) == component_id) {
					Logger::log("Plat_Link_Crrcsim::handle_input: PARAM_SET for this component", (int)component_id, Logger::LOGLEVEL_INFO);
					mavlink_msg_param_set_get_param_id(&msg, param_id);
					Logger::log("Plat_Link_Crrcsim::handle_input: PARAM_SET for param_id", param_id, Logger::LOGLEVEL_INFO);

					typedef map<string, double>::const_iterator ci;
					for(ci p = params.begin(); p!=params.end(); ++p) {
						// Logger::log("plat_link_crrcsim param test", p->first, p->second, Logger::LOGLEVEL_INFO);
						if(!strcmp(p->first.data(), (const char *)param_id)) {
							params[p->first] = mavlink_msg_param_set_get_param_value(&msg);
							Logger::log("x Plat_Link_Crrcsim::handle_input: PARAM_SET request for", p->first, params[p->first], Logger::LOGLEVEL_INFO);
						}
					}
					// update variables
					pid_alt->setSp(params["ac_sp"]);
					pid_alt->setBias(params["ac_pid_bias"]);
					pid_alt->setKc(params["ac_pid_Kc"]);
					pid_alt->setTi(params["ac_pid_Ki"]);
					pid_alt->setTd(params["ac_pid_Kd"]);
					//pid_alt->setIntegral(0.0);

					// bump_lat->bump(0.0);
					//printf("ctl_mode: %d\n", ctl_mode);
					// bump->set_thr_low(params["bump_thr_low"]);
					// bump->set_thr_high(params["bump_thr_high"]);

					pid_alt->setPv_int_lim(params["ac_pid_int_lim"]);

					ctl_mode = static_cast<int>(params["ctl_mode"]);
					ctl_mode_lat = static_cast<int>(params["ctl_mode_lat"]);
					// output_enable = static_cast<int>(params["output_enable"]);
				}
			}
			break;

		case MAVLINK_MSG_ID_DEBUG:
			//Logger::log("Plat_Link_Crrcsim::handle_input: received debug from", (int)msg.sysid, (int)msg.compid, Logger::LOGLEVEL_INFO);
			if(msg.sysid == system_id()) {
				if(msg.compid == component_id) {
					// Logger::log("Plat_Link_Crrcsim::handle_input: received debug from", (int)msg.sysid, (int)msg.compid, Logger::LOGLEVEL_INFO);
					Logger::log("Plat_Link_Crrcsim::handle_input: received debug from",
											(int)mavlink_msg_debug_get_ind(&msg),
											mavlink_msg_debug_get_value(&msg),
											Logger::LOGLEVEL_INFO);
					if((int)mavlink_msg_debug_get_ind(&msg) == 1) {
						thrust = mavlink_msg_debug_get_value(&msg);
					}
				}
			}
			break;

		case MAVLINK_MSG_ID_HUCH_GENERIC_CHANNEL:
			switch(mavlink_msg_huch_generic_channel_get_index(&msg)) {
			case CHAN_THRUST:
				thrust = mavlink_msg_huch_generic_channel_get_value(&msg);
				// Logger::log("Plat_Link_Crrcsim::handle_input: received gen_chan thrust from", (int)msg.sysid, (int)msg.compid, thrust, Logger::LOGLEVEL_INFO);
				break;
			case CHAN_ROLL:
				roll = mavlink_msg_huch_generic_channel_get_value(&msg);
				// Logger::log("Plat_Link_Crrcsim::handle_input: received gen_chan roll from", (int)msg.sysid, (int)msg.compid, roll, Logger::LOGLEVEL_INFO);
				break;
			case CHAN_PITCH:
				pitch = mavlink_msg_huch_generic_channel_get_value(&msg);
				// Logger::log("Plat_Link_Crrcsim::handle_input: received gen_chan pitch from", (int)msg.sysid, (int)msg.compid, pitch, Logger::LOGLEVEL_INFO);
				break;
			default:
				break;
			}
			break;

		case MAVLINK_MSG_ID_HUCH_SIM_CTRL:
			Logger::log("Plat_Link_Crrcsim::handle_input: received sim_ctrl from", (int)msg.sysid, (int)msg.compid, Logger::LOGLEVEL_INFO);
			break;

		// case MAVLINK_MSG_ID_ACTION:
		// 	Logger::log("Plat_Link_Crrcsim::handle_input: received action request", (int)msg.sysid, (int)msg.compid, Logger::LOGLEVEL_INFO);
		// 	if(mavlink_msg_action_get_target(&msg) == system_id()){
		// 		if(mavlink_msg_action_get_target_component(&msg) == component_id) {
		// 			switch(mavlink_msg_action_get_action(&msg)) {
		// 			case 0:
		// 				bump->bump(0.0);
		// 				break;
		// 			default:
		// 				break;
		// 			}
		// 		}
		// 	}
		// 	break;
		default:
			break;

		}
		// AppLayer<mavlink_message_t>::send(msg);
	}

	void Plat_Link_Crrcsim::run() {
		if(!owner()) {
			Logger::log("Owner of Plat_Link_Crrcsim not set", Logger::LOGLEVEL_WARN);
			return;
		}

		int sleeptime;
		uint64_t dt;

		//Vec<double, 5> v;
		vector<double> v(5);

		Logger::log("Plat_Link_Crrcsim started, sys_id, comp_id", system_id(), component_id, Logger::LOGLEVEL_INFO);

		ctl.target = 43; // paramize
		ctl.roll = 0.;
		ctl.pitch = 0.;
		ctl.yaw = 0.;
		ctl.mask = 0;
		// ctl.roll_manual = 0;
		// ctl.pitch_manual = 0;
		// ctl.yaw_manual = 0;
		// ctl.thrust_manual = 0;

		v[0] = 1.0;
		v[1] = 0;
		v[2] = 0;
		v[3] = 0;
		v[4] = 0;

		//cout << "v[0] = " << v[0] << endl;

		while(1) {
			//Logger::log("plat_link_crrcsim: system_id", static_cast<int>(system_id()), Logger::LOGLEVEL_INFO);
			//AppLayer<mavlink_message_t>::send(msg_heartbeat);
			//send(msg_heartbeat);
			//IvySendMsg("%d MAVHUB_ALIVE %f", 155, 1.0);

			// run method exec timing stuff
			sleeptime = exec_tmr->calcSleeptime();
			//Logger::log("plat_link_crrcsim.cpp: sleeptime: ", sleeptime, Logger::LOGLEVEL_INFO);
			usleep(sleeptime);
			dt = exec_tmr->updateExecStats();

			// respond to parameter list request
			param_request_respond();

			v[0] = m1;
			v[1] = m2;

			//ffnet->eval(v);

			switch(ctl_mode) {
			case CTL_MODE_BUMP:
				ctl.thrust = 0; // bump->calc((double)dt * 1e-6);
				break;
			case CTL_MODE_AC:
				// FIXME: built in controller is obsolete. Rather, use
				//        external controller module so identical output can
				//        be routed to any copter hardware

				ctl.thrust = pid_alt->calc((double)dt * 1e-6, z_hat);

				if(ctl.thrust > params["thr_max"]) {
					ctl.thrust = params["thr_max"];
					pid_alt->setIntegralM1();
				}

				if(ctl.thrust <= params["thr_min"]) {
					ctl.thrust = params["thr_min"];
					pid_alt->setIntegralM1();
				}

				if(ctl.thrust <= params[""]) {
					ctl.thrust = params["thr_min"];
					pid_alt->setIntegralM1();
				}

				break;
			case CTL_MODE_DIRECT:
				// pipe through
				ctl.thrust = thrust;
				break;
			case CTL_MODE_NULL:
			default:
				ctl.thrust = 0.0;
				break;
			}

			switch(ctl_mode_lat) {
			case CTL_MODE_BUMP:
				ctl.roll = 0; // bump_lat->calc((double)dt * 1e-6);
				break;
			case CTL_MODE_DIRECT:
				ctl.roll = roll;
				ctl.pitch = pitch;
				break;
			case CTL_MODE_NULL:
			default:
				ctl.roll = 0.0;
				break;
			}

			// send control output to (onboard) controller
			if(params["output_enable"] > 0) {
				//Logger::log("plat_link_crrcsim: ctl.thust = ", ctl.thrust, Logger::LOGLEVEL_INFO);
				mavlink_msg_huch_attitude_control_encode(
																								 system_id(), 
																								 1,
																								 &msg,
																								 &ctl);
				AppLayer<mavlink_message_t>::send(msg);
			}
		}
	}

	// read config file
	void Plat_Link_Crrcsim::read_conf(const map<string, string> args) {
		map<string,string>::const_iterator iter;
		Logger::log("Plat_Link_Crrcsim::read_conf", Logger::LOGLEVEL_INFO);

		// iter = args.find("component_id");
		// if( iter != args.end() ) {
		// 	istringstream s(iter->second);
		// 	s >> component_id;
		// }

		iter = args.find("bump_thr_low");
		if( iter != args.end() ) {
			istringstream s(iter->second);
			s >> params["bump_thr_low"];
		}
		iter = args.find("bump_thr_high");
		if( iter != args.end() ) {
			istringstream s(iter->second);
			s >> params["bump_thr_high"];
		}

		iter = args.find("ctl_update_rate");
		if( iter != args.end() ) {
			istringstream s(iter->second);
			s >> params["ctl_update_rate"];
		}

		iter = args.find("output_enable");
		if( iter != args.end() ) {
			istringstream s(iter->second);
			s >> params["output_enable"];
		}

		iter = args.find("ctl_mode");
		if( iter != args.end() ) {
			istringstream s(iter->second);
			s >> params["ctl_mode"];
			ctl_mode = static_cast<int>(params["ctl_mode"]);
		}

		iter = args.find("ctl_mode_lat");
		if( iter != args.end() ) {
			istringstream s(iter->second);
			s >> params["ctl_mode_lat"];
			ctl_mode_lat = static_cast<int>(params["ctl_mode_lat"]);
		}

		Logger::log("Plat_Link_Crrcsim::read_conf: sysid, compid", system_id(), component_id, Logger::LOGLEVEL_INFO);
	}

	// set defaults
	void Plat_Link_Crrcsim::conf_defaults() {
		param_request_list = false;
		// 0.147893587316 0.14 0.00928571428571
		params["ac_pid_bias"] = 0.0;
		// manually tuned
		// params["ac_pid_Kc"] = 0.01;
		// params["ac_pid_Ki"] = 2.0;
		// params["ac_pid_Kd"] = 0.4;
		// from tuning recipe, doesn't work
		// params["ac_pid_Kc"] = 0.147893587316;
		// params["ac_pid_Ki"] = 0.14;
		// params["ac_pid_Kd"] = 0.00928571428571;
		// from evolution, old controller
		params["ac_pid_Kc"] = 0.066320932874519622;
		params["ac_pid_Ki"] = 1.2421727487431193;
		params["ac_pid_Kd"] = 0.28506217896710773;
		// from ol: 0.10861511  2.93254908  0.02315
		// from ol: 0.1, 2, 0.5
		params["ac_pid_int_lim"] = 10.0;
		params["ac_pid_scalef"] = 0.0;
		params["ac_sp"] = 2.23;
		params["ctl_mode"] = 0;
		params["ctl_mode_lat"] = 0;
		params["ctl_update_rate"] = 100; // Hz
		params["bump_thr_low"] = 0.38;
		params["bump_thr_high"] = 0.39;
		params["thr_max"] = 0.6;
		params["thr_min"] = 0.1;
		params["output_enable"] = 0;
	}

	// // handle parameter list request
	// void Plat_Link_Crrcsim::param_request_respond() {
	// 	Logger::log("Plat_Link_Crrcsim::param_request_respond", Logger::LOGLEVEL_INFO);
	// 	param_request_list = false;
	// 	typedef map<string, double>::const_iterator ci;
	// 	for(ci p = params.begin(); p!=params.end(); ++p) {
	// 		// Logger::log("plat_link_crrcsim param test", p->first, p->second, Logger::LOGLEVEL_INFO);
	// 		mavlink_msg_param_value_pack(system_id(), component_id, &msg, (const int8_t*) p->first.data(), p->second, 1, 0);
	// 		AppLayer<mavlink_message_t>::send(msg);
	// 	}
	// }

	// // send debug
	// void Plat_Link_Crrcsim::send_debug(mavlink_message_t* msg, mavlink_debug_t* dbg, int index, double value) {
	// 	dbg->ind = index;
	// 	dbg->value = value;
	// 	mavlink_msg_debug_encode(system_id(), static_cast<uint8_t>(component_id), msg, dbg);
	// 	//Logger::log("crrcsim: sending debug", Logger::LOGLEVEL_INFO);
	// 	AppLayer<mavlink_message_t>::send(*msg);
	// }



} // namespace mavhub

#endif // HAVE_MAVLINK_H

