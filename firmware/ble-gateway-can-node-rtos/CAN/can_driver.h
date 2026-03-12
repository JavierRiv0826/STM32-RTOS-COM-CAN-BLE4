/*
 * can_driver.h
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

#ifndef CAN_DRIVER_H_
#define CAN_DRIVER_H_

#include <stdint.h>

typedef struct
{
    uint16_t id;
    uint8_t  len;
    uint8_t  data[8];
} can_frame_t;

void CAN_Driver_Init(void);
void CAN_Driver_Send(uint16_t std_id,
                     uint8_t *data,
                     uint8_t len);

#endif /* CAN_DRIVER_H_ */
