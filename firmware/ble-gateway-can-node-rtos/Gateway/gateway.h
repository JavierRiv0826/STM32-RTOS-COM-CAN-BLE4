/*
 * gateway.h
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

#ifndef GATEWAY_H_
#define GATEWAY_H_

#include "can_driver.h"
#include "protocol.h"
#include "ble_protocol.h"
#include "ble_types.h"
#include "ble_driver.h"
#include <string.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"
#include <stdint.h>

void Gateway_Init(void);
void Gateway_ProcessCan(uint16_t std_id,
                         uint8_t *data,
                         uint8_t len);
void Gateway_Supervision_Task(void);
void Gateway_ProcessBle(uint8_t byte);

#endif /* GATEWAY_H_ */
