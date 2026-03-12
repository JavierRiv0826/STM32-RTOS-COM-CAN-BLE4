/*
 * ble_driver.c
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

/* Includes ------------------------------------------------------------------*/
#include "ble_driver.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define BLE_RX_BUFFER_SIZE 128

/* Private  types  -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t rx_byte;
static uint8_t rx_buffer[BLE_RX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern osSemaphoreId bleRxSemHandle;

/* Private function prototypes -----------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
void BLE_Init(void)
{
    // Nothing special for now
	HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void BLE_SendBytes(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, data, len, 100);
}

uint8_t BLE_GetByte(uint8_t *byte)
{
    if (rx_head == rx_tail)
        return 0;

    *byte = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % BLE_RX_BUFFER_SIZE;

    return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint16_t next = (rx_head + 1) % BLE_RX_BUFFER_SIZE;

        if (next != rx_tail)
        {
            rx_buffer[rx_head] = rx_byte;
            rx_head = next;
        }

        osSemaphoreRelease(bleRxSemHandle);

        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}
