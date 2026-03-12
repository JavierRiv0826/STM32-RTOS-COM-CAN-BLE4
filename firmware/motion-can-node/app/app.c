/*
 * app.c
 *
 *  Created on: 27 feb 2026
 *      Author: Xavi
 */

/* Includes ------------------------------------------------------------------*/
#include "app.h"
#include "can_driver.h"
#include "protocol.h"
#include "stm32f1xx_hal.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define NODE_ID        1
#define HEARTBEAT_PERIOD 5000  // 1 second

/* Exported types  -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t motion_counter = 0;
static uint32_t last_hb_time = 0;

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;

/* Private function prototypes -----------------------------------------------*/
static void send_frame(uint8_t msg_type);
static void app_process_can(uint16_t std_id,
                     uint8_t *data,
                     uint8_t len);

/* Public functions ----------------------------------------------------------*/
void App_Init(void)
{
    CAN_Driver_Init();
    HAL_TIM_Base_Start(&htim2);  // Start timer2 for timestamp
}

void App_Run(void)
{
	//CAN Rx handling
	uint16_t id;
	uint8_t  data[8];
	uint8_t  len;

	if (CAN_Driver_GetRx(&id, data, &len))
	{
		app_process_can(id, data, len);
	}

    /* Event-driven system */
	 uint32_t now = HAL_GetTick();

	if ((now - last_hb_time) >= HEARTBEAT_PERIOD)
	{
		last_hb_time = now;
		send_frame(MSG_HEARTBEAT);
	}
}

void App_MotionTrigger(void)
{
	send_frame(MSG_MOTION);
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

/* Private functions ---------------------------------------------------------*/
static void send_frame(uint8_t msg_type)
{
	protocol_frame_t frame;
	motion_payload_t payload;

	/* Build payload */
	payload.node_id = NODE_ID;
	payload.timestamp = __HAL_TIM_GET_COUNTER(&htim2);

	/* Build protocol header */
	frame.ver_type =
		((PROTOCOL_VERSION & 0x03) << 6) |
		(msg_type & 0x3F);

	frame.counter = motion_counter++;
	frame.fragment_idx = 0;

	/* Copy payload into frame */
	memcpy(frame.data, &payload, sizeof(frame.data));

	/* Always send 8 bytes */
	CAN_Driver_Send(
		CAN_ID_MOTION,
		(uint8_t*)&frame,
		8
	);

	//HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

static void app_process_can(uint16_t std_id,
                     uint8_t *data,
                     uint8_t len)
{
    if (std_id != CAN_ID_GATEWAY)
        return;

    if (len != 8)
        return;

    protocol_frame_t frame;
    memcpy(&frame, data, sizeof(frame));

    uint8_t version  = (frame.ver_type >> 6) & 0x03;
    uint8_t msg_type = frame.ver_type & 0x3F;

    if (version != PROTOCOL_VERSION)
        return;

    if (msg_type != MSG_COMMAND)
        return;

    command_payload_t payload;
    memcpy(&payload, frame.data, sizeof(payload));

    if (payload.target != TARGET_ALL &&
        payload.target != TARGET_MOTION)
        return;

    switch (payload.command)
    {
        case CMD_LED_ON:
        	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
            break;

        case CMD_LED_OFF:
        	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
            break;
    }
}
