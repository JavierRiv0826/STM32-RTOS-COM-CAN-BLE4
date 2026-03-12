/*
 * crc16.h
 *
 *  Created on: 3 mar 2026
 *      Author: Xavi
 */

#ifndef CRC16_H_
#define CRC16_H_

#pragma once
#include <stdint.h>

uint16_t Crc16_Compute(uint8_t *data, uint16_t len);

#endif /* CRC16_H_ */
