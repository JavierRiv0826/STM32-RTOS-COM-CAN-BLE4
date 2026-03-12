/*
 * bmp280.h
 *
 *  Created on: 8 feb 2026
 *      Author: Xavi
 */

#ifndef BMP280_BMP280_H_
#define BMP280_BMP280_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <math.h>

/* Defines -------------------------------------------------------------------*/
/* BMP280 I2C Address (7-bit shifted left) */
#define BMP280_I2C_ADDR   (0x77 << 1)

/* BMP280 Registers */
#define BMP280_REG_ID     0xD0
#define BMP280_REG_RESET  0xE0
#define BMP280_REG_CTRL_MEAS   0xF4
#define BMP280_REG_CONFIG      0xF5
#define BMP280_REG_PRESS_MSB   0xF7

/* BMP280 Constants */
#define BMP280_CHIP_ID    0x58
#define BMP280_RESET_CMD  0xB6
#define BMP280_SEA_LEVEL_PRESSURE  101325.0f

/* Exported types ------------------------------------------------------------*/
/* Driver handle */
typedef struct
{
    int32_t adc_T;
    int32_t adc_P;
} bmp280_raw_t;

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_t;

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    uint8_t chip_id;
    bmp280_calib_t calib;
    int32_t t_fine;
} bmp280_t;

typedef struct
{
    int32_t  temperature_x100;
    uint32_t pressure_pa;
    int32_t  altitude_x100;
} bmp280_data_t;

/* Function prototypes -----------------------------------------------*/
HAL_StatusTypeDef BMP280_Init(bmp280_t *dev, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BMP280_Reset(bmp280_t *dev);
HAL_StatusTypeDef BMP280_ReadID(bmp280_t *dev);
HAL_StatusTypeDef BMP280_ReadCalibration(bmp280_t *dev);
HAL_StatusTypeDef BMP280_Configure(bmp280_t *dev);
HAL_StatusTypeDef BMP280_ReadRaw(bmp280_t *dev, bmp280_raw_t *raw);
int32_t BMP280_Compensate_Temp(bmp280_t *dev, int32_t adc_T);
uint32_t BMP280_Compensate_Pressure(bmp280_t *dev, int32_t adc_P);
int32_t BMP280_CalcAltitude(uint32_t pressure_pa);
HAL_StatusTypeDef BMP280_ReadData(bmp280_t *dev, bmp280_data_t *data);

#endif /* BMP280_BMP280_H_ */
