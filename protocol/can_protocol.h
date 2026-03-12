#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <stdint.h>

/* =========================
   Version
   ========================= */
#define PROTOCOL_VERSION  1

/* =========================
   CAN IDs
   ========================= */
#define CAN_ID_MOTION     0x100
#define CAN_ID_ENV        0x200
#define CAN_ID_GATEWAY    0x300

/* =========================
   Targets
   ========================= */
#define TARGET_ALL        1
#define TARGET_MOTION     2
#define TARGET_ENV        3

/* =========================
   Commands
   ========================= */
#define CMD_LED_OFF       0
#define CMD_LED_ON        1

/* =========================
   Message Types
   ========================= */
typedef enum
{
    MSG_HEARTBEAT = 1,
    MSG_MOTION    = 2,
    MSG_ENV       = 3,
    MSG_COMMAND   = 4
} msg_type_t;

/* =========================
   CAN Frame (8 bytes)
   ========================= */
typedef struct __attribute__((packed))
{
    uint8_t ver_type;      /* [7:6]=Version | [5:0]=MsgType */
    uint8_t counter;
    uint8_t fragment_idx;
    uint8_t data[5];
} protocol_frame_t;

/* =========================
   Motion Payload
   ========================= */
typedef struct __attribute__((packed))
{
    uint8_t  node_id;
    uint32_t timestamp;
} motion_payload_t;

/* =========================
   Environment Payload (10 bytes)
   ========================= */
typedef struct __attribute__((packed))
{
    int16_t  bmp_temp_x100;
    int16_t  aht_temp_x100;
    uint16_t humidity_x100;
    uint16_t pressure_dpa;
    int16_t  altitude_x10;
} sensor_packet_t;

/* =========================
   Command Payload
   ========================= */
typedef struct __attribute__((packed))
{
    uint8_t target;
    uint8_t command;
    uint8_t data[3];
} command_payload_t;

#endif /* CAN_PROTOCOL_H */