// talk to FC with mkpackage

#ifndef _CTRL_HOVER_H_
#define _CTRL_HOVER_H_

#include "debug_channels.h"
#include "core/logger.h"
#include "filter_kalmancv.h"
#include "PID.h"
#include "core/protocollayer.h"
#include "qk_helper.h"
#include "pp.h"
#include "pp_uss.h"
#include "pp_acc.h"
#include "pp_baro.h"
#include "pp_ir.h"

namespace mavhub {
	/// Controller: hover (altitude)
  class Ctrl_Hover : public AppLayer {
  public:
		/// Constructor
		// Ctrl_Hover(int component_id_, int numchan, const std::list< std::pair<int, int> > chanmap, const std::map<std::string, std::string> args);
		Ctrl_Hover(const std::map<std::string, std::string> args);
		virtual ~Ctrl_Hover();
		/// mavhub protocolstack input handler
		virtual void handle_input(const mavlink_message_t &msg);
		/// sensor types
		enum sensor_types_t {
			USS,
			BARO,
			ACC,
			IR_SHARP_30_3V,
			IR_SHARP_150_3V,
			STATUS,
			IR_SHARP_30_5V,
			IR_SHARP_150_5V
		};

		/// debugout type to index map
		enum mk_debugout_map_t {
			USSvalue = 0,
			USSlastvalid = 1,
			USScred = 2, // XXX: changed in FC to sticknick, roll, yaw
			ADval_press = 3,
			ATTabsh = 4,
			ATTrelh = 5,
			USSoffset = 6,
			USSstatus = 7,
			ADval_gyrroll = 8,
			ADval_gyrnick = 9,
			ADval_gyryaw = 10,
			ATTrelacctopint = 11,
			ADval_ubat = 12,
			GASmixfrac1 = 13,
			GASmixfrac2 = 14,
			RC_rssi = 15,
			ATTmeanaccnick = 16,
			ATTmeanaccroll = 17,
			ATTmeanacctop = 18,
			ATTintnickl = 19,
			ATTintrolll = 20,
			ATTintyawl = 21,
			FCParam_extctlswitch = 22,
			FCParam_gpsswitch = 23,
			ADval_accnick = 24,
			ADval_accroll = 25,
			ADval_acctop = 26,
			CTL_stickgas = 27,
			ADval_acctopraw = 28,
			ATTintnickh = 29,
			ATTintrollh = 30,
			ATTintyawh = 31
		};

  protected:
		/// this thread's main method
		virtual void run();
		/// preprocess raw sensor inputs
		virtual void preproc();
		/// reset barometer reference
		virtual void reset_baro_ref(double ref);

  private:
		/// component id
		uint16_t component_id;

		// HUCH stuff
		/// huch attitude struct
		mavlink_huch_attitude_t attitude;
		/// huch altitude struct
		mavlink_huch_fc_altitude_t altitude;
		/// huch exp ctrl struct
		mavlink_huch_exp_ctrl_t exp_ctrl;
		/// huch ranger(s) struct
		mavlink_huch_ranger_t ranger;
		/// MK external control
		extern_control_t extctrl;
		/// local state
		mavlink_huch_ctrl_hover_state_t ctrl_hover_state;
		/// MK DebugOut structure
		mavlink_mk_debugout_t mk_debugout;
		/// MK FC Status
		mavlink_mk_fc_status_t mk_fc_status;

		// PIXHAWK stuff
		/// pixhawk raw imu
		mavlink_raw_imu_t raw_imu;
		/// pixhawk attitude
		mavlink_attitude_t ml_attitude;
		/// pixhawk manual control input
		mavlink_manual_control_t manual_control;
		/// pixhawk attitude controller output
		mavlink_attitude_controller_output_t attitude_controller_output;

		/// Kalman instance
		Kalman_CV* kal;
		/// PID instance (altitude)
		PID* pid_alt;
		/// number of sensor inputs processed by the module
		int numchan;
		std::list< std::pair<int, int> > chanmap_pairs;
		std::vector<int> chanmap;
		/// raw sensor input array
		std::vector<int> raw;
		/// pre-processed sensor array: first: value, second: status/valid
		std::vector< std::pair< double, int > > pre;
		/// preprocessor modules
		std::vector<PreProcessor *> premod;
		/// barometer reference
		double baro_ref;
		// config variables
		/// controller bias (hovergas)
		int ctl_bias;
		double ctl_Kc;
		double ctl_Ti;
		double ctl_Td;
		int ctl_sp;
		int ctl_bref;
		int ctl_sticksp;
		double ctl_mingas;
		double ctl_maxgas;
		// output
		int output_enable;

		// parameters
		int param_request_list;
		int param_count;		

		/// strapdown matrix setter
		int sd_comp_C(mavlink_huch_attitude_t *a, CvMat *C);

		/// read data from config
		virtual void read_conf(const std::map<std::string, std::string> args);
		/// send debug data
		void send_debug(mavlink_message_t* msg, mavlink_debug_t* dbg, int index, double value);
		/// limit gas
		virtual int limit_gas(double gas);

		// MK debug structure conversion
		int mk_debugout_digital_offset;
		// fetch different types from byte based mk_debugout
		uint16_t debugout_getval_u(mavlink_mk_debugout_t* dbgout, int index);
		int16_t debugout_getval_s(mavlink_mk_debugout_t* dbgout, int index);
		int32_t debugout_getval_s32(mavlink_mk_debugout_t* dbgout, int indexl, int indexh);

		void debugout2attitude(mavlink_mk_debugout_t* dbgout);
		void debugout2altitude(mavlink_mk_debugout_t* dbgout);
		void debugout2exp_ctrl(mavlink_mk_debugout_t* dbgout, mavlink_huch_exp_ctrl_t* exp_ctrl);
		void debugout2ranger(mavlink_mk_debugout_t* dbgout, mavlink_huch_ranger_t* ranger);
		void debugout2status(mavlink_mk_debugout_t* dbgout, mavlink_mk_fc_status_t* status);

		void set_pxh_raw_imu();
		void set_pxh_attitude();
		void set_pxh_manual_control();
  };

	// limit gas
	inline int Ctrl_Hover::limit_gas(double gas) {
		if(gas < ctl_mingas)
			gas = ctl_mingas; // mingas
		if (gas > ctl_maxgas)
			gas = ctl_maxgas; // maxgas
		return gas;
	}

}

#endif
