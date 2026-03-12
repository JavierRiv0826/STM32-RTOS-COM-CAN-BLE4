/*
 * ble_protocol.h
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

#ifndef PROTOCOL_BLE_PROTOCOL_H_
#define PROTOCOL_BLE_PROTOCOL_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Defines -------------------------------------------------------------------*/
#define PROTO_MAX_PAYLOAD 32
#define PROTO_HEADER_1 0xAA
#define PROTO_HEADER_2 0x55

/* Types ------------------------------------------------------------*/
typedef struct
{
    uint8_t type;
    uint8_t len;
    uint8_t payload[PROTO_MAX_PAYLOAD];
} proto_frame_t;

typedef enum
{
    PROTO_WAIT_AA,
    PROTO_WAIT_55,
    PROTO_WAIT_LEN,
    PROTO_WAIT_TYPE,
    PROTO_WAIT_PAYLOAD,
    PROTO_WAIT_CRC_L,
    PROTO_WAIT_CRC_H
} proto_state_t;

typedef struct
{
    proto_state_t state;

    uint8_t len;
    uint8_t type;
    uint8_t payload[PROTO_MAX_PAYLOAD];
    uint8_t payload_idx;

    uint16_t crc_rx;
    uint8_t crc_buffer[PROTO_MAX_PAYLOAD + 2];

} protocol_t;

/* Public function prototypes -----------------------------------------------*/
void Protocol_Init(protocol_t *p);

uint16_t Protocol_Build_Frame(
    uint8_t type,
    const uint8_t *payload,
    uint8_t payload_len,
    uint8_t *out_buf);

bool Protocol_Process_Byte(
    protocol_t *p,
    uint8_t byte,
    proto_frame_t *out);

#endif /* PROTOCOL_BLE_PROTOCOL_H_ */
