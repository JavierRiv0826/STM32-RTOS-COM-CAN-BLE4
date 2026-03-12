/*
 * aht20.h
 *
 *  Created on: 8 feb 2026
 *      Author: Xavi
 */

#ifndef AHT20_AHT20_H_
#define AHT20_AHT20_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <stdint.h>

/* Defines -------------------------------------------------------------------*/
#define AHT20_I2C_ADDR   (0x38 << 1)

/* Commands */
#define AHT20_CMD_INIT      0xBE
#define AHT20_CMD_TRIGGER   0xAC

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    I2C_HandleTypeDef *hi2c;
    uint8_t status;
} aht20_t;

typedef struct
{
    int32_t  temperature_x100;   // °C ×100
    uint16_t humidity_x100;      // %RH ×100
} aht20_data_t;

/* Function prototypes -----------------------------------------------*/
/* API */
HAL_StatusTypeDef AHT20_Init(aht20_t *dev, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef AHT20_TriggerMeasurement(aht20_t *dev);
HAL_StatusTypeDef AHT20_ReadRaw(aht20_t *dev, uint8_t *data);
HAL_StatusTypeDef AHT20_ReadData(aht20_t *dev, aht20_data_t *data);

#endif /* AHT20_AHT20_H_ */
