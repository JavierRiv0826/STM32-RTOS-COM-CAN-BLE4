/*
 * ble_driver.h
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

#ifndef BLE_DRIVER_H_
#define BLE_DRIVER_H_

#include <stdint.h>

void BLE_Init(void);
//void BLE_SendString(char *str);
void BLE_SendBytes(uint8_t *data, uint16_t len);
uint8_t BLE_GetByte(uint8_t *byte);

#endif /* BLE_DRIVER_H_ */
