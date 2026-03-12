/*
 * wateway.c
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

/* Includes ------------------------------------------------------------------*/
#include "gateway.h"

/* Private defines -----------------------------------------------------------*/
#define OFFLINE_TIMEOUT_MS 15000

/* Private  types  -----------------------------------------------------------*/
typedef struct
{
    uint8_t  node_id;
    uint32_t last_seen_ms;
    uint8_t  online;
} node_status_t;

typedef struct
{
    uint8_t  active;
    uint8_t  fragments_received;   // bit0 = frag0, bit1 = frag1
    uint8_t  buffer[10];
} env_reassembly_t;

/* Private variables ---------------------------------------------------------*/
static node_status_t motion_node =
{
    .node_id = 1,
    .last_seen_ms = 0,
    .online = 0
};

static node_status_t env_node =
{
    .node_id = 1,
    .last_seen_ms = 0,
    .online = 0
};

/* Optional: track last counter */
static uint8_t last_motion_counter = 0;
static uint8_t last_env_counter = 0;

static env_reassembly_t env_rx;
static sensor_packet_t packet;

static uint8_t ble_tx_buffer[64];
//Rx
static protocol_t ble_protocol;
static proto_frame_t ble_frame;

/* External variables --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void handle_motion(protocol_frame_t *frame);
static void handle_env(protocol_frame_t *frame);
static void gateway_send_can_command(uint8_t target, uint8_t cmd);
static void gateway_handle_frame(proto_frame_t *frame);

/* Public functions ----------------------------------------------------------*/
void Gateway_Init(void)
{
	BLE_Init();
	Protocol_Init(&ble_protocol);
	CAN_Driver_Init();
}

/* =========================
   Main CAN Entry Point
   ========================= */
void Gateway_ProcessCan(uint16_t std_id,
                         uint8_t *data,
                         uint8_t len)
{
    if (len != 8)
        return;   // Must always be 8 bytes

    protocol_frame_t frame;

    memcpy(&frame, data, sizeof(protocol_frame_t));

    switch (std_id)
    {
        case CAN_ID_MOTION:
            handle_motion(&frame);
            break;

        case CAN_ID_ENV:
            handle_env(&frame);
            break;

        default:
            break;
    }
}

/* =========================
   //Offline Detection
   ========================= */
void Gateway_Supervision_Task(void)
{
    uint32_t now = HAL_GetTick();

    if (motion_node.online &&
        (now - motion_node.last_seen_ms) > OFFLINE_TIMEOUT_MS)
    {
        motion_node.online = 0;
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET); // LED OFF = offline

        //Forward via UART to HM-10
        uint8_t payload[2];

        payload[0] = STATUS_NODE_DISCONNECTED;
        payload[1] = NODE_TYPE_MOTION;

        uint16_t len =
			Protocol_Build_Frame(
				BLE_TYPE_STATUS,
				(uint8_t*)&payload,
				sizeof(payload),
				ble_tx_buffer);

		BLE_SendBytes(ble_tx_buffer, len);
    }

    if (env_node.online &&
		(now - env_node.last_seen_ms) > OFFLINE_TIMEOUT_MS)
	{
		env_node.online = 0;
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // LED OFF = offline

		//Forward via UART to HM-10
		uint8_t payload[2];

		payload[0] = STATUS_NODE_DISCONNECTED;
		payload[1] = NODE_TYPE_ENV;

		uint16_t len =
			Protocol_Build_Frame(
				BLE_TYPE_STATUS,
				(uint8_t*)&payload,
				sizeof(payload),
				ble_tx_buffer);

		BLE_SendBytes(ble_tx_buffer, len);
	}
}

void Gateway_ProcessBle(uint8_t byte)
{
    if (Protocol_Process_Byte(&ble_protocol,
                              byte,
                              &ble_frame))
    {
        // Frame complete
        gateway_handle_frame(&ble_frame);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    }
}

/* Private functions ---------------------------------------------------------*/
/* =========================
   Motion Handler
   ========================= */
static void handle_motion(protocol_frame_t *frame)
{
    motion_payload_t payload;

    /* Extract payload */
    memcpy(&payload,
           frame->data,
           sizeof(motion_payload_t));

    uint8_t version  = (frame->ver_type >> 6) & 0x03;
    uint8_t msg_type = frame->ver_type & 0x3F;

    /* Validate */
    if (version != PROTOCOL_VERSION)
        return;

    /* Counter tracking (optional loss detection) */
    last_motion_counter = frame->counter;

    switch(msg_type)
    {
        case MSG_MOTION:
        	//Forward via UART to HM-10
        	uint16_t len =
				Protocol_Build_Frame(
					BLE_TYPE_MOTION,
					(uint8_t*)&payload,
					sizeof(payload),
					ble_tx_buffer);

			BLE_SendBytes(ble_tx_buffer, len);
            break;

        case MSG_HEARTBEAT:

            break;
    }

    //Reset Offline Detection
    uint32_t now = HAL_GetTick();

	motion_node.last_seen_ms = now;

	if (!motion_node.online)
	{
		motion_node.online = 1;
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); // LED ON = online
	}
}

/* =========================
   Env Handler
   ========================= */
static void handle_env(protocol_frame_t *frame)
{
	uint8_t version  = (frame->ver_type >> 6) & 0x03;
	//uint8_t msg_type = frame->ver_type & 0x3F;

	/* Validate */
	if (version != PROTOCOL_VERSION)
		return;

	/* Counter tracking (optional loss detection) */
	last_env_counter = frame->counter;

	//Fragments assembly
	uint8_t frag = frame->fragment_idx;

	/* New packet detected */
	if (!env_rx.active)
	{
		env_rx.active = 1;
		env_rx.fragments_received = 0;
	}

	/* Copy fragment data */
	if (frag < 2)
	{
		memcpy(&env_rx.buffer[frag * 5],
			   frame->data,
			   5);

		env_rx.fragments_received |= (1 << frag);
	}

	/* Check if complete */
	if (env_rx.fragments_received == 0x03)
	{

		memcpy(&packet, env_rx.buffer, 10);

		//Reset assembly
		env_rx.active = 0;
		env_rx.fragments_received = 0;


		//Forward via UART to HM-10
		uint16_t len =
			Protocol_Build_Frame(
				BLE_TYPE_ENV,
				(uint8_t*)&packet,
				sizeof(packet),
				ble_tx_buffer);

		BLE_SendBytes(ble_tx_buffer, len);

		//Reset Offline Detection

		uint32_t now = HAL_GetTick();

		env_node.last_seen_ms = now;

		if (!env_node.online)
		{
			env_node.online = 1;
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET); // LED ON = online
		}
	}
}

/* =========================
   Command
   ========================= */
static void gateway_handle_frame(proto_frame_t *frame)
{
    switch(frame->type)
    {
        case 0x10:   // ALL LEDs OFF
            gateway_send_can_command(TARGET_ALL, CMD_LED_OFF);
            break;

        case 0x11:   // ALL LEDs ON
            gateway_send_can_command(TARGET_ALL, CMD_LED_ON);
            break;

        case 0x12:   // Motion Node LED OFF
			gateway_send_can_command(TARGET_MOTION, CMD_LED_OFF);
			break;

		case 0x13:   // Motion Node LED ON
			gateway_send_can_command(TARGET_MOTION, CMD_LED_ON);
			break;

		case 0x14:   // Env Node LED OFF
			gateway_send_can_command(TARGET_ENV, CMD_LED_OFF);
			break;

		case 0x15:   // Env Node LED ON
			gateway_send_can_command(TARGET_ENV, CMD_LED_ON);
			break;

        default:
            break;
    }
}

static void gateway_send_can_command(uint8_t target, uint8_t cmd)
{
    protocol_frame_t frame;
    command_payload_t payload;

    payload.target  = target;
    payload.command = cmd;
    payload.data[0] = 0;
    payload.data[1] = 0;
    payload.data[2] = 0;

    frame.ver_type =
        ((PROTOCOL_VERSION & 0x03) << 6) |
        (MSG_COMMAND & 0x3F);

    static uint8_t gw_counter = 0;

    frame.counter = gw_counter++;
    frame.fragment_idx = 0;

    memcpy(frame.data, &payload, sizeof(payload));

    CAN_Driver_Send(CAN_ID_GATEWAY,
                    (uint8_t*)&frame,
                    8);
}

