/*
 * bmp280.c
 *
 *  Created on: 8 feb 2026
 *      Author: Xavi
 */

/* Includes ------------------------------------------------------------------*/
#include "bmp280.h"

/* Private defines -----------------------------------------------------------*/

/* Exported types  -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef bmp280_write_reg(bmp280_t *dev, uint8_t reg, uint8_t data);
static HAL_StatusTypeDef bmp280_read_reg(bmp280_t *dev, uint8_t reg, uint8_t *data, uint16_t len);

/* Public functions ----------------------------------------------------------*/
HAL_StatusTypeDef BMP280_ReadID(bmp280_t *dev)
{
    return bmp280_read_reg(dev, BMP280_REG_ID, &dev->chip_id, 1);
}

HAL_StatusTypeDef BMP280_Reset(bmp280_t *dev)
{
    HAL_StatusTypeDef status;

    status = bmp280_write_reg(dev, BMP280_REG_RESET, BMP280_RESET_CMD);
    if (status != HAL_OK)
        return status;

    HAL_Delay(5);   // Datasheet recommends >2 ms
    return HAL_OK;
}

HAL_StatusTypeDef BMP280_Init(bmp280_t *dev, I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;

    dev->hi2c = hi2c;

    status = BMP280_ReadID(dev);
    if (status != HAL_OK)
        return status;

    if (dev->chip_id != BMP280_CHIP_ID)
        return HAL_ERROR;

    status = BMP280_Reset(dev);
    if (status != HAL_OK)
        return status;

    status = BMP280_ReadCalibration(dev);
	if (status != HAL_OK)
		return status;

    return HAL_OK;
}

HAL_StatusTypeDef BMP280_ReadCalibration(bmp280_t *dev)
{
    uint8_t buf[24];
    HAL_StatusTypeDef status;

    status = bmp280_read_reg(dev, 0x88, buf, 24);
    if (status != HAL_OK)
        return status;

    dev->calib.dig_T1 = (uint16_t)(buf[1] << 8 | buf[0]);
    dev->calib.dig_T2 = (int16_t)(buf[3] << 8 | buf[2]);
    dev->calib.dig_T3 = (int16_t)(buf[5] << 8 | buf[4]);

    dev->calib.dig_P1 = (uint16_t)(buf[7] << 8 | buf[6]);
    dev->calib.dig_P2 = (int16_t)(buf[9] << 8 | buf[8]);
    dev->calib.dig_P3 = (int16_t)(buf[11] << 8 | buf[10]);
    dev->calib.dig_P4 = (int16_t)(buf[13] << 8 | buf[12]);
    dev->calib.dig_P5 = (int16_t)(buf[15] << 8 | buf[14]);
    dev->calib.dig_P6 = (int16_t)(buf[17] << 8 | buf[16]);
    dev->calib.dig_P7 = (int16_t)(buf[19] << 8 | buf[18]);
    dev->calib.dig_P8 = (int16_t)(buf[21] << 8 | buf[20]);
    dev->calib.dig_P9 = (int16_t)(buf[23] << 8 | buf[22]);

    return HAL_OK;
}

HAL_StatusTypeDef BMP280_Configure(bmp280_t *dev)
{
    uint8_t ctrl_meas = 0x27; // temp x1, press x1, normal mode
    return bmp280_write_reg(dev, BMP280_REG_CTRL_MEAS, ctrl_meas);
}

HAL_StatusTypeDef BMP280_ReadRaw(bmp280_t *dev, bmp280_raw_t *raw)
{
    uint8_t buf[6];
    HAL_StatusTypeDef status;

    status = bmp280_read_reg(dev, BMP280_REG_PRESS_MSB, buf, 6);
    if (status != HAL_OK)
        return status;

    raw->adc_P = (int32_t)(
        (buf[0] << 12) |
        (buf[1] << 4)  |
        (buf[2] >> 4)
    );

    raw->adc_T = (int32_t)(
        (buf[3] << 12) |
        (buf[4] << 4)  |
        (buf[5] >> 4)
    );

    return HAL_OK;
}

int32_t BMP280_Compensate_Temp(bmp280_t *dev, int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)dev->calib.dig_T1 << 1))) *
            ((int32_t)dev->calib.dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)dev->calib.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)dev->calib.dig_T1))) >> 12) *
            ((int32_t)dev->calib.dig_T3)) >> 14;

    dev->t_fine = var1 + var2;

    T = (dev->t_fine * 5 + 128) >> 8;

    return T; // temperature in 0.01 °C
}

uint32_t BMP280_Compensate_Pressure(bmp280_t *dev, int32_t adc_P)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)dev->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dev->calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)dev->calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)dev->calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dev->calib.dig_P3) >> 8) +
           ((var1 * (int64_t)dev->calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) *
           ((int64_t)dev->calib.dig_P1) >> 33;

    if (var1 == 0)
        return 0; // avoid division by zero

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dev->calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dev->calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) +
        (((int64_t)dev->calib.dig_P7) << 4);

    return (uint32_t)(p >> 8); // pressure in Pa
}

int32_t BMP280_CalcAltitude(uint32_t pressure_pa)
{
    float ratio = (float)pressure_pa / BMP280_SEA_LEVEL_PRESSURE;

    float altitude =
        44330.0f *
        (1.0f - powf(ratio, 0.19029495f));

    return (int32_t)(altitude * 100.0f);
}

HAL_StatusTypeDef BMP280_ReadData(bmp280_t *dev, bmp280_data_t *data)
{
    if (dev == NULL || data == NULL)
        return HAL_ERROR;

    bmp280_raw_t raw;

    HAL_StatusTypeDef status = BMP280_ReadRaw(dev, &raw);
    if (status != HAL_OK)
        return status;

    /* Temperature (already x100 from your compensation) */
    data->temperature_x100 =
        BMP280_Compensate_Temp(dev, raw.adc_T);

    /* Pressure in Pa */
    data->pressure_pa =
        BMP280_Compensate_Pressure(dev, raw.adc_P);

    /* Altitude in meters x100 */
    data->altitude_x100 =
        BMP280_CalcAltitude(data->pressure_pa);

    return HAL_OK;
}

/* Private functions ---------------------------------------------------------*/
static HAL_StatusTypeDef bmp280_write_reg(bmp280_t *dev, uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    return HAL_I2C_Master_Transmit(dev->hi2c, BMP280_I2C_ADDR, buf, 2, 100);
}

static HAL_StatusTypeDef bmp280_read_reg(bmp280_t *dev, uint8_t reg, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Transmit(dev->hi2c, BMP280_I2C_ADDR, &reg, 1, 100);
    if (status != HAL_OK)
        return status;

    return HAL_I2C_Master_Receive(dev->hi2c, BMP280_I2C_ADDR, data, len, 100);
}
