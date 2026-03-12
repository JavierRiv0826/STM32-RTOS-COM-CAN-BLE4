/*
 * can_driver.c
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

/* Includes ------------------------------------------------------------------*/
#include "can_driver.h"
#include "stm32f1xx_hal.h"
#include "protocol.h"
#include "cmsis_os.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/* Exported types  -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan;
extern osMessageQId q_can_rxHandle;
extern osPoolId canPoolHandle;

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

    /* Accept only Motion (0x100) and Env (0x200) */
    filter.FilterIdHigh = (CAN_ID_MOTION << 5);
    filter.FilterIdLow  = (CAN_ID_ENV << 5);

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

/* Interrupt-based RX */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rxHeader;
    can_frame_t *frame;

    frame = osPoolAlloc(canPoolHandle);
    if (frame == NULL)
        return; // pool full, drop frame

    HAL_CAN_GetRxMessage(
        hcan,
        CAN_RX_FIFO0,
        &rxHeader,
        frame->data
    );

    frame->id  = rxHeader.StdId;
    frame->len = rxHeader.DLC;

    osMessagePut(q_can_rxHandle,
                 (uint32_t)frame,
                 0);
}

/* Private functions ---------------------------------------------------------*/

