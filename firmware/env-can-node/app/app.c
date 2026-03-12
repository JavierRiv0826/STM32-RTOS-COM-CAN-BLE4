/*
 * app.c
 *
 *  Created on: 27 feb 2026
 *      Author: Xavi
 */

/* Includes ------------------------------------------------------------------*/
#include "app.h"
#include "bmp280.h"
#include "aht20.h"
#include "can_driver.h"
#include "protocol.h"
#include <string.h>
#include "stm32f1xx_hal.h"

/* Private defines -----------------------------------------------------------*/
#define ENV_PERIOD_MS  10000

/* Exported types  -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static bmp280_t bmp280;
static bmp280_data_t bmp_data;
static aht20_t  aht20;
static aht20_data_t aht_data;

static uint32_t last_env_time = 0;
static uint8_t  packet_counter = 0;

/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef read_and_build_packet (sensor_packet_t *packet);
static void send_env_frame(sensor_packet_t *packet);
static void app_process_can(uint16_t std_id,
                     uint8_t *data,
                     uint8_t len);

/* Public functions ----------------------------------------------------------*/
void App_Init(void)
{
    /* Initialize CAN peripheral + filters */
    CAN_Driver_Init();

    //Sensors Init
    BMP280_Init(&bmp280, &hi2c1);
    BMP280_Configure(&bmp280);
    AHT20_Init(&aht20, &hi2c1);
}

void App_Run(void)
{
	//CAN Tx handling
	uint32_t now = HAL_GetTick();

	if ((now - last_env_time) >= ENV_PERIOD_MS)
	{
		last_env_time = now;

		sensor_packet_t packet;

		if (read_and_build_packet(&packet) == HAL_OK)
		{
			send_env_frame(&packet);
		}
	}

	//CAN Rx handling
	uint16_t id;
	uint8_t  data[8];
	uint8_t  len;

	if (CAN_Driver_GetRx(&id, data, &len))
	{
		app_process_can(id, data, len);
	}


}

/* Private functions ---------------------------------------------------------*/
static HAL_StatusTypeDef read_and_build_packet (sensor_packet_t *packet)
{
    if (AHT20_ReadData(&aht20, &aht_data) != HAL_OK)
        return HAL_ERROR;

    if (BMP280_ReadData(&bmp280, &bmp_data) != HAL_OK)
        return HAL_ERROR;

    packet->bmp_temp_x100 = (int16_t)bmp_data.temperature_x100;
    packet->aht_temp_x100 = (int16_t)aht_data.temperature_x100;
    packet->humidity_x100 = aht_data.humidity_x100;
    packet->pressure_dpa  = (uint16_t)(bmp_data.pressure_pa / 10);
    packet->altitude_x10  = (int16_t)(bmp_data.altitude_x100 / 10);

    return HAL_OK;
}

static void send_env_frame(sensor_packet_t *packet)
{
    protocol_frame_t frame;
    uint8_t raw[10];

    memcpy(raw, packet, 10);

    for (uint8_t frag = 0; frag < 2; frag++)
    {
        frame.ver_type =
            ((PROTOCOL_VERSION & 0x03) << 6) |
            (MSG_ENV_FRAGMENT & 0x3F);

        frame.counter = packet_counter++;
        frame.fragment_idx = frag;

        memcpy(frame.data, &raw[frag * 5], 5);

        CAN_Driver_Send(CAN_ID_ENV, (uint8_t*)&frame, 8);
    }

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
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
        payload.target != TARGET_ENV)
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
