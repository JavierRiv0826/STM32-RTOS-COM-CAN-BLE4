/*
 * protocol.h
 *
 *  Created on: 27 feb 2026
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
/* Message Types (6 bits max) */
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
    uint8_t counter;       // Rolling counter per node
    uint8_t fragment_idx;  // 0 = not fragmented
    uint8_t data[5];       // Bytes 3–7
} protocol_frame_t;

/* Motion payload = exactly 5 bytes */
typedef struct __attribute__((packed))
{
    uint8_t  node_id;
    uint32_t timestamp;
} motion_payload_t;

typedef struct __attribute__((packed))
{
    uint8_t target;
    uint8_t command;
    uint8_t data[3];
} command_payload_t;


#endif /* PROTOCOL_H_ */
