// MESSAGE HUCH_ACTION PACKING

#define MAVLINK_MSG_ID_HUCH_ACTION 230

typedef struct __mavlink_huch_action_t
{
 uint8_t target; ///< The system executing the action
 uint8_t target_component; ///< The component executing the action
 uint8_t action; ///< The action id
} mavlink_huch_action_t;

#define MAVLINK_MSG_ID_HUCH_ACTION_LEN 3
#define MAVLINK_MSG_ID_230_LEN 3



#define MAVLINK_MESSAGE_INFO_HUCH_ACTION { \
	"HUCH_ACTION", \
	3, \
	{  { "target", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_huch_action_t, target) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_huch_action_t, target_component) }, \
         { "action", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_huch_action_t, action) }, \
         } \
}


/**
 * @brief Pack a huch_action message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target The system executing the action
 * @param target_component The component executing the action
 * @param action The action id
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_huch_action_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t target, uint8_t target_component, uint8_t action)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[3];
	_mav_put_uint8_t(buf, 0, target);
	_mav_put_uint8_t(buf, 1, target_component);
	_mav_put_uint8_t(buf, 2, action);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 3);
#else
	mavlink_huch_action_t packet;
	packet.target = target;
	packet.target_component = target_component;
	packet.action = action;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 3);
#endif

	msg->msgid = MAVLINK_MSG_ID_HUCH_ACTION;
	return mavlink_finalize_message(msg, system_id, component_id, 3, 233);
}

/**
 * @brief Pack a huch_action message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param target The system executing the action
 * @param target_component The component executing the action
 * @param action The action id
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_huch_action_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t target,uint8_t target_component,uint8_t action)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[3];
	_mav_put_uint8_t(buf, 0, target);
	_mav_put_uint8_t(buf, 1, target_component);
	_mav_put_uint8_t(buf, 2, action);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 3);
#else
	mavlink_huch_action_t packet;
	packet.target = target;
	packet.target_component = target_component;
	packet.action = action;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 3);
#endif

	msg->msgid = MAVLINK_MSG_ID_HUCH_ACTION;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 3, 233);
}

/**
 * @brief Encode a huch_action struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param huch_action C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_huch_action_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_huch_action_t* huch_action)
{
	return mavlink_msg_huch_action_pack(system_id, component_id, msg, huch_action->target, huch_action->target_component, huch_action->action);
}

/**
 * @brief Send a huch_action message
 * @param chan MAVLink channel to send the message
 *
 * @param target The system executing the action
 * @param target_component The component executing the action
 * @param action The action id
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_huch_action_send(mavlink_channel_t chan, uint8_t target, uint8_t target_component, uint8_t action)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[3];
	_mav_put_uint8_t(buf, 0, target);
	_mav_put_uint8_t(buf, 1, target_component);
	_mav_put_uint8_t(buf, 2, action);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_HUCH_ACTION, buf, 3, 233);
#else
	mavlink_huch_action_t packet;
	packet.target = target;
	packet.target_component = target_component;
	packet.action = action;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_HUCH_ACTION, (const char *)&packet, 3, 233);
#endif
}

#endif

// MESSAGE HUCH_ACTION UNPACKING


/**
 * @brief Get field target from huch_action message
 *
 * @return The system executing the action
 */
static inline uint8_t mavlink_msg_huch_action_get_target(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field target_component from huch_action message
 *
 * @return The component executing the action
 */
static inline uint8_t mavlink_msg_huch_action_get_target_component(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field action from huch_action message
 *
 * @return The action id
 */
static inline uint8_t mavlink_msg_huch_action_get_action(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Decode a huch_action message into a struct
 *
 * @param msg The message to decode
 * @param huch_action C-struct to decode the message contents into
 */
static inline void mavlink_msg_huch_action_decode(const mavlink_message_t* msg, mavlink_huch_action_t* huch_action)
{
#if MAVLINK_NEED_BYTE_SWAP
	huch_action->target = mavlink_msg_huch_action_get_target(msg);
	huch_action->target_component = mavlink_msg_huch_action_get_target_component(msg);
	huch_action->action = mavlink_msg_huch_action_get_action(msg);
#else
	memcpy(huch_action, _MAV_PAYLOAD(msg), 3);
#endif
}
