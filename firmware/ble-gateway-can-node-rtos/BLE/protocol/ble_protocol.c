/*
 * ble_protocol.c
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

/* Includes ------------------------------------------------------------------*/
#include "ble_protocol.h"
#include "crc16.h"

/* Public functions ----------------------------------------------------------*/
void Protocol_Init(protocol_t *p)
{
    if (!p)
        return;

    p->state = PROTO_WAIT_AA;
    p->payload_idx = 0;
}

uint16_t Protocol_Build_Frame(
    uint8_t type,
    const uint8_t *payload,
    uint8_t payload_len,
    uint8_t *out_buf)
{
    if (!out_buf || payload_len > PROTO_MAX_PAYLOAD)
        return 0;

    uint16_t idx = 0;
    uint8_t len = payload_len + 1;

    out_buf[idx++] = PROTO_HEADER_1;
    out_buf[idx++] = PROTO_HEADER_2;

    out_buf[idx++] = len;
    out_buf[idx++] = type;

    for (uint8_t i = 0; i < payload_len; i++)
        out_buf[idx++] = payload[i];

    uint16_t crc =
		Crc16_Compute(&out_buf[2], len + 1);

    out_buf[idx++] = crc & 0xFF;
    out_buf[idx++] = (crc >> 8) & 0xFF;

    return idx;
}

bool Protocol_Process_Byte(protocol_t *p,
                           uint8_t b,
                           proto_frame_t *out)
{
    if (!p || !out)
        return false;

    switch (p->state)
    {
        case PROTO_WAIT_AA:
            if (b == PROTO_HEADER_1)
                p->state = PROTO_WAIT_55;
            break;

        case PROTO_WAIT_55:
            if (b == PROTO_HEADER_2)
                p->state = PROTO_WAIT_LEN;
            else
                p->state = PROTO_WAIT_AA;
            break;

        case PROTO_WAIT_LEN:

            if (b == 0 || b > (PROTO_MAX_PAYLOAD + 1))
            {
                p->state = PROTO_WAIT_AA;
                break;
            }

            p->len = b;
            p->payload_idx = 0;

            p->crc_buffer[0] = b; // LEN
            p->state = PROTO_WAIT_TYPE;
            break;

        case PROTO_WAIT_TYPE:

            p->type = b;
            p->crc_buffer[1] = b; // TYPE

            if (p->len == 1)
                p->state = PROTO_WAIT_CRC_L;
            else
                p->state = PROTO_WAIT_PAYLOAD;
            break;

        case PROTO_WAIT_PAYLOAD:

            p->payload[p->payload_idx] = b;
            p->crc_buffer[2 + p->payload_idx] = b;

            p->payload_idx++;

            if (p->payload_idx >= (p->len - 1))
                p->state = PROTO_WAIT_CRC_L;

            break;

        case PROTO_WAIT_CRC_L:

            p->crc_rx = b;
            p->state = PROTO_WAIT_CRC_H;
            break;

        case PROTO_WAIT_CRC_H:
        {
            p->crc_rx |= ((uint16_t)b << 8);

            uint16_t crc_calc =
				Crc16_Compute(p->crc_buffer, p->len + 1);

            if (p->crc_rx == crc_calc)
            {
                out->type = p->type;
                out->len  = p->len - 1;

                for (uint8_t i = 0; i < out->len; i++)
                    out->payload[i] = p->payload[i];

                p->state = PROTO_WAIT_AA;
                return true;
            }

            p->state = PROTO_WAIT_AA;
            break;
        }
    }

    return false;
}

