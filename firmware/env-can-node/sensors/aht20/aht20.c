/*
 * aht20.c
 *
 *  Created on: 8 feb 2026
 *      Author: Omar
 */

/* Includes ------------------------------------------------------------------*/
#include "aht20.h"

/* Private defines -----------------------------------------------------------*/

/* Exported types  -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef aht20_write_cmd(aht20_t *dev, uint8_t cmd, uint8_t p1, uint8_t p2);

/* Public functions ----------------------------------------------------------*/
HAL_StatusTypeDef AHT20_Init(aht20_t *dev, I2C_HandleTypeDef *hi2c)
{
    dev->hi2c = hi2c;

    HAL_Delay(40); // Power-up delay

    return aht20_write_cmd(dev, AHT20_CMD_INIT, 0x08, 0x00);
}

HAL_StatusTypeDef AHT20_TriggerMeasurement(aht20_t *dev)
{
    return aht20_write_cmd(dev, AHT20_CMD_TRIGGER, 0x33, 0x00);
}

HAL_StatusTypeDef AHT20_ReadRaw(aht20_t *dev, uint8_t *data)
{
    return HAL_I2C_Master_Receive(dev->hi2c, AHT20_I2C_ADDR, data, 6, 100);
}

HAL_StatusTypeDef AHT20_ReadData(aht20_t *dev, aht20_data_t *data)
{
    if (dev == NULL || data == NULL)
        return HAL_ERROR;

    uint8_t data_raw[6];
    uint32_t raw_hum;
    uint32_t raw_temp;

    if (AHT20_TriggerMeasurement(dev) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(80);   // measurement time (we can improve later)

    if (AHT20_ReadRaw(dev, data_raw) != HAL_OK)
        return HAL_ERROR;

    dev->status = data_raw[0];

    if (data_raw[0] & 0x80)  // busy bit
        return HAL_BUSY;

    raw_hum  = ((uint32_t)data_raw[1] << 12) |
               ((uint32_t)data_raw[2] << 4)  |
               ((data_raw[3] & 0xF0) >> 4);

    raw_temp = (((uint32_t)(data_raw[3] & 0x0F)) << 16) |
               ((uint32_t)data_raw[4] << 8) |
               data_raw[5];

    /* Convert to fixed-point x100 */

    float humidity =
        ((float)raw_hum / 1048576.0f) * 100.0f;

    float temperature =
        ((float)raw_temp / 1048576.0f) * 200.0f - 50.0f;

    data->humidity_x100 =
        (uint16_t)(humidity * 100.0f);

    data->temperature_x100 =
        (int32_t)(temperature * 100.0f);

    return HAL_OK;
}


/* Private functions ---------------------------------------------------------*/
static HAL_StatusTypeDef aht20_write_cmd(aht20_t *dev, uint8_t cmd, uint8_t p1, uint8_t p2)
{
    uint8_t buf[3] = {cmd, p1, p2};
    return HAL_I2C_Master_Transmit(dev->hi2c, AHT20_I2C_ADDR, buf, 3, 100);
}
