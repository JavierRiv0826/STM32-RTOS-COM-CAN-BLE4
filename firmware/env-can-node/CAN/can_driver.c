/*
 * can_driver.c
 *
 *  Created on: 27 feb 2026
 *      Author: Xavi
 */

/* Includes ------------------------------------------------------------------*/
#include "can_driver.h"
#include "stm32f1xx_hal.h"
#include "protocol.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/* Exported types  -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static volatile uint8_t can_rx_flag = 0;
static uint16_t can_rx_id;
static uint8_t  can_rx_data[8];
static uint8_t  can_rx_len;

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan;

/* Private function prototypes -----------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
void CAN_Driver_Init(void)
{
    CAN_FilterTypeDef filter;

    filter.FilterActivation     = ENABLE;
    filter.FilterBank           = 0;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterMode           = CAN_FILTERMODE_IDLIST;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;

    /* Accept only Gateway (0x300) */
    filter.FilterIdHigh = (CAN_ID_GATEWAY << 5);
    filter.FilterIdLow  = 0x0000;   // second slot unused

    filter.FilterMaskIdHigh = 0;
    filter.FilterMaskIdLow  = 0;

    HAL_CAN_ConfigFilter(&hcan, &filter);
    HAL_CAN_Start(&hcan);

    HAL_CAN_ActivateNotification(
        &hcan,
        CAN_IT_RX_FIFO0_MSG_PENDING
    );
}

void CAN_Driver_Send(uint16_t std_id,
                     uint8_t *data,
                     uint8_t len)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t mailbox;

    txHeader.StdId = std_id;
    txHeader.ExtId = 0;
    txHeader.IDE   = CAN_ID_STD;
    txHeader.RTR   = CAN_RTR_DATA;
    txHeader.DLC   = len;

    HAL_CAN_AddTxMessage(&hcan,
                         &txHeader,
                         data,
                         &mailbox);
}

uint8_t CAN_Driver_GetRx(uint16_t *id,
                         uint8_t *data,
                         uint8_t *len)
{
    if (!can_rx_flag)
        return 0;

    __disable_irq();

    *id  = can_rx_id;
    *len = can_rx_len;
    memcpy(data, can_rx_data, 8);
    can_rx_flag = 0;

    __enable_irq();

    return 1;
}

/* Interrupt-based RX */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    HAL_CAN_GetRxMessage(
        hcan,
        CAN_RX_FIFO0,
        &rxHeader,
        rxData
    );

    if (!can_rx_flag)   // drop if previous not processed
    {
        can_rx_id  = rxHeader.StdId;
        can_rx_len = rxHeader.DLC;
        memcpy(can_rx_data, rxData, 8);
        can_rx_flag = 1;
    }
}

/* Private functions ---------------------------------------------------------*/
