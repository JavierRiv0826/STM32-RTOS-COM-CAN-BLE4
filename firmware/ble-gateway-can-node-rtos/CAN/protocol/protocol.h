/*
 * protocol.h
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Defines -------------------------------------------------------------------*/
#define PROTOCOL_VERSION  1

#define CAN_ID_MOTION    0x100
#define CAN_ID_ENV		 0x200
#define CAN_ID_GATEWAY   0x300

#define TARGET_ALL     1
#define TARGET_MOTION  2
#define TARGET_ENV     3

#define CMD_LED_OFF    0
#define CMD_LED_ON     1

/* Exported types ------------------------------------------------------------*/
/* Message Types (6-bit max) */
typedef enum
{
    MSG_HEARTBEAT = 1,
    MSG_MOTION    = 2,
    MSG_ENV       = 3,
	MSG_COMMAND   = 4
} msg_type_t;

/* Unified 8-byte CAN frame */
typedef struct __attribute__((packed))
{
    uint8_t ver_type;      // [7:6]=Version | [5:0]=MsgType
    uint8_t counter;
    uint8_t fragment_idx;
    uint8_t data[5];
} protocol_frame_t;

/* Motion payload (5 bytes total) */
typedef struct __attribute__((packed))
{
    uint8_t  node_id;
    uint32_t timestamp;
} motion_payload_t;

/* Env payload */
typedef struct __attribute__((packed))
{
    int16_t  bmp_temp_x100;
    int16_t  aht_temp_x100;
    uint16_t humidity_x100;
    uint16_t pressure_dpa;     // pressure / 10
    int16_t  altitude_x10;
} sensor_packet_t;

typedef struct __attribute__((packed))
{
    uint8_t target;
    uint8_t command;
    uint8_t data[3];
} command_payload_t;

#endif /* PROTOCOL_H_ */
