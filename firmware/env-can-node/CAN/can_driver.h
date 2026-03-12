/*
 * can_driver.h
 *
 *  Created on: 27 feb 2026
 *      Author: Xavi
 */

#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include <stdint.h>

void CAN_Driver_Init(void);
void CAN_Driver_Send(uint16_t std_id,
                     uint8_t *data,
                     uint8_t len);
uint8_t CAN_Driver_GetRx(uint16_t *id,
                         uint8_t *data,
                         uint8_t *len);

#endif /* CAN_DRIVER_H_ */
