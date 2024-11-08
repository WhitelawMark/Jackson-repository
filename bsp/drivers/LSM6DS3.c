/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-29     RealThread   first version
 * 2023-07-14     lwp         32MB FLASH（ 4Byte Address） support
 */
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>


/*
********************************************************************************
********************************************************************************
*/
#include "LSM6DS3.h"
#include <string.h>



#define DRV_DEBUG
#define LOG_TAG             "drv.rtc"
#include <drv_log.h>

/* 私有函数声明区域 */
static void Lsm6ds3_Init(void);
static void Lsm6ds3_WriteByte(uint8_t reg, uint8_t data);
static uint8_t Lsm6ds3_ReadByte(uint8_t reg);
static uint16_t Lsm6ds3_ReadMultiple(uint8_t reg_addr_begin, uint8_t reg_addr_end, uint8_t *data);

uint8_t Lsm6ds3_ReadOrigin(void);
uint8_t Lsm6ds3_ReadWhoAmI(void);
int16_t *Lsm6ds3_ReadAccelerationRaw(int16_t *pbuf);
int16_t *Lsm6ds3_ReadAngularRateRaw(int16_t *pbuf);
int16_t *Lsm6ds3_ReadTemperatureRaw(int16_t *pbuf);

I2C_HandleTypeDef hi2c1;
/* lsm6ds3传感器对象初始化 */
sLSM6DS3_Dev g_lsm6ds3 =
{
    &hi2c1,
    {0},
    LSM6DS3_SlaveAddress,
    LSM6DS3_DeviceID,
    
    Lsm6ds3_Init,
    Lsm6ds3_ReadByte,
    Lsm6ds3_WriteByte,
    Lsm6ds3_ReadMultiple,
    
    Lsm6ds3_ReadOrigin,
    Lsm6ds3_ReadWhoAmI,
    Lsm6ds3_ReadAngularRateRaw,
    Lsm6ds3_ReadAccelerationRaw,
    Lsm6ds3_ReadTemperatureRaw,
};

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param[in]  handle    customizable argument. In this examples is used in
 *                       order to select the correct sensor bus handler.
 * @param[in]  reg       register to read
 * @param[in]  bufp      pointer to buffer that store the data read
 * @param[in]  len       number of consecutive register to read
 *
 */
static I2C_Status platform_ReadByte(void *handle, uint8_t slaveAddress, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    return HAL_I2C_Mem_Read(handle, slaveAddress, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param[in]  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param[in]  reg       register to write
 * @param[in]  bufp      pointer to data to write in register reg
 * @param[in]  len       number of consecutive register to write
 *
 */
static I2C_Status platform_WriteByte(void *handle, uint8_t slaveAddress, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    return HAL_I2C_Mem_Write(handle, slaveAddress, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)bufp, len, 1000);
}

/**
 * @brief      写入一个字节
 *
 * @param[in]  reg   寄存器地址
 * @param[in]  data  待写入的数据
 */
static void Lsm6ds3_WriteByte(uint8_t reg, uint8_t data)
{
    I2C_Status ret = platform_WriteByte(g_lsm6ds3.handle,  g_lsm6ds3.slaveAddress | eWrite, 
        reg, &data, sizeof(data));
    
    if(ret != HAL_OK)
        LOG_E("error_code:%d", ret);
}

/**
 * @brief      读取一个字节
 *
 * @param[in]  reg   寄存器地址
 *
 * @return     返回读取到的数据
 */
static uint8_t Lsm6ds3_ReadByte(uint8_t reg)
{
    I2C_Status ret = platform_ReadByte(g_lsm6ds3.handle,  g_lsm6ds3.slaveAddress | eRead, 
        reg, &(g_lsm6ds3.m_dataBuf.uReaddata), sizeof(g_lsm6ds3.m_dataBuf.uReaddata));
    if(ret != HAL_OK)
    {
        LOG_E("error_code:%d", ret);
        g_lsm6ds3.m_dataBuf.uReaddata= 0xff;
    }
    LOG_I("Lsm6ds3_ReadByte ret=%d, reg=0x%x, data=0x%x", ret, reg, g_lsm6ds3.m_dataBuf.uReaddata);
    return g_lsm6ds3.m_dataBuf.uReaddata;
}

/**
 * @brief      连续地址读取传感器参数/数据
 *
 * @param[in]  reg_addr_begin  The register address begin
 * @param[in]  reg_addr_end    The register address end
 * @param[out] data            The data
 *
 * @return     返回读取的字节数
 */
static uint16_t Lsm6ds3_ReadMultiple(uint8_t reg_addr_begin, uint8_t reg_addr_end, uint8_t *data)
{
    uint16_t nCount = 0;
    if(reg_addr_begin > reg_addr_end)
    {
        LOG_E("register address invalid!");
        return 0;
    }
    
    while(nCount < (reg_addr_end - reg_addr_begin + 1)) 
    {
        data[nCount] = Lsm6ds3_ReadByte(reg_addr_begin + nCount);
        nCount++;
    }
    
    return nCount;
}

/**
 * @brief      初始化陀螺仪和加速度传感器
 */
static void Lsm6ds3_Init(void)
{
    uint8_t whoAmI = Lsm6ds3_ReadByte(LSM6DS3_WHO_AM_I);
    
    LOG_I("Lsm6ds3_Init[G-SensorId] -> 0x%x", whoAmI);
    if(whoAmI != LSM6DS3_DeviceID)
    {
        LOG_E("read who am i failed!");
        return;
    }
    // 加速度计52HZ（倾斜角检测功能工作在26HZ，因此加速度计ODR必须设置为>=26hz）,2g量程。
    Lsm6ds3_WriteByte(LSM6DS3_CTRL1_XL, 0x20);
    // 使能加速度计x,y,z轴
    Lsm6ds3_WriteByte(LSM6DS3_CTRL9_XL, 0x38);
    // enable accelerometer int1
    Lsm6ds3_WriteByte(LSM6DS3_INT1_CTRL, 0x01); 
    
    // 陀螺仪208hz  2000dps
    Lsm6ds3_WriteByte(LSM6DS3_CTRL2_G, 0x5C);
    // 使能陀螺仪x,y,z轴
    Lsm6ds3_WriteByte(LSM6DS3_CTRL10_C, 0x38);

    // WAKE_UP INTERRUPT Configuration
    Lsm6ds3_WriteByte(LSM6DS3_TAP_CFG, 0x90);
    Lsm6ds3_WriteByte(LSM6DS3_WAKE_UP_DUR, 0x00);
    Lsm6ds3_WriteByte(LSM6DS3_WAKE_UP_THS, 0x02);
    Lsm6ds3_WriteByte(LSM6DS3_MD1_CFG, 0x20);
    
    // 6D Orientation Configuration
    Lsm6ds3_WriteByte(LSM6DS3_TAP_THS_6D, 0x40);
    Lsm6ds3_WriteByte(LSM6DS3_CTRL8_XL, 0x01);

    LOG_I("Lsm6ds3 init successfule!");
}
/*
********************************************************************************
                                 应用接口
********************************************************************************
*/
#ifdef USER_APP_INTERFACE
/**
 * @brief      获取传感器坐标零点
 *
 * @return     见#define LSM6DS3_D6D_SRC
 */
uint8_t Lsm6ds3_ReadOrigin(void)
{
    g_lsm6ds3.m_dataBuf.orientation = Lsm6ds3_ReadByte(LSM6DS3_D6D_SRC); 
    return g_lsm6ds3.m_dataBuf.orientation;
}

/**
 * @brief      获取芯片ID
 *
 * @return     返回芯片ID
 */
uint8_t Lsm6ds3_ReadWhoAmI(void)
{
    g_lsm6ds3.m_dataBuf.whoamI = Lsm6ds3_ReadByte(LSM6DS3_WHO_AM_I); 
    return g_lsm6ds3.m_dataBuf.whoamI;
}

/**
 * @brief      读取角速度寄存器原始值
 *
 * @param[out] pbuf  The pbuf
 *
 * @return     返回角速度寄存器原始值（带符号数值）
 */
int16_t *Lsm6ds3_ReadAngularRateRaw(int16_t *pbuf)
{
    // 读取寄存器值使用无符号类型，读取后取值再转为有符号
    uint8_t buf[6] = {0};

    if((g_lsm6ds3.ReadByte(LSM6DS3_STATUS_REG) & STATUS_GDA_GYRO_E) != 0)
    {
        Lsm6ds3_ReadMultiple(LSM6DS3_OUTX_L_G, LSM6DS3_OUTZ_H_G, buf);
        g_lsm6ds3.m_dataBuf.data_raw_angular_rate[0] = (buf[1] << 8) | buf[0];
        g_lsm6ds3.m_dataBuf.data_raw_angular_rate[1] = (buf[3] << 8) | buf[2];
        g_lsm6ds3.m_dataBuf.data_raw_angular_rate[2] = (buf[5] << 8) | buf[4];
    }
    
    if(NULL != pbuf)
        memcpy(pbuf, buf, sizeof(buf));
    
    return g_lsm6ds3.m_dataBuf.data_raw_angular_rate;
}

/**
 * @brief      读取加速度寄存器原始值
 *
 * @param[out] pbuf  The pbuf
 *
 * @return     返回加速度寄存器原始值（带符号数值）
 */
int16_t *Lsm6ds3_ReadAccelerationRaw(int16_t *pbuf)
{
    // 读取寄存器值使用无符号类型，读取后取值再转为有符号
    uint8_t buf[6] = {0};

    if((g_lsm6ds3.ReadByte(LSM6DS3_STATUS_REG) & STATUS_XLDA_ACC_E) != 0)
    {
        Lsm6ds3_ReadMultiple(LSM6DS3_OUTX_L_XL, LSM6DS3_OUTZ_H_XL, buf);
        g_lsm6ds3.m_dataBuf.data_raw_acceleration[0] = (buf[1] << 8) | buf[0];
        g_lsm6ds3.m_dataBuf.data_raw_acceleration[1] = (buf[3] << 8) | buf[2];
        g_lsm6ds3.m_dataBuf.data_raw_acceleration[2] = (buf[5] << 8) | buf[4];
    }
    
    if(NULL != pbuf)
        memcpy(pbuf, buf, sizeof(buf));
    
    return g_lsm6ds3.m_dataBuf.data_raw_acceleration;
}

/**
 * @brief      读取温度寄存器原始值
 *
 * @param[out] pbuf  The pbuf
 *
 * @return     返回温度存器原始值（带符号数值）
 */
int16_t *Lsm6ds3_ReadTemperatureRaw(int16_t *pbuf)
{
    // 读取寄存器值使用无符号类型，读取后取值再转为有符号
    uint8_t buf[2] = {0};

    if((g_lsm6ds3.ReadByte(LSM6DS3_STATUS_REG) & STATUS_TDA_TEMP_E) != 0)
    {
        Lsm6ds3_ReadMultiple(LSM6DS3_OUT_TEMP_L, LSM6DS3_OUT_TEMP_H, buf);
        g_lsm6ds3.m_dataBuf.data_raw_temperature = (buf[1] << 8) | buf[0];
    }
    
    if(NULL != pbuf)
        memcpy(pbuf, buf, sizeof(buf));
    
    return &(g_lsm6ds3.m_dataBuf.data_raw_temperature);
}
#else
__weak uint8_t Lsm6ds3_ReadOrigin(void)
{
    return 0xff;
}

__weak uint8_t Lsm6ds3_ReadWhoAmI(void)
{
    return 0xff;
}

__weak int16_t *Lsm6ds3_ReadAngularRateRaw(int16_t *pbuf)
{
    return NULL;
}

__weak int16_t *Lsm6ds3_ReadAccelerationRaw(int16_t *pbuf)
{
    return NULL;
}

__weak int16_t *Lsm6ds3_ReadTemperatureRaw(int16_t *pbuf)
{
    return NULL;
}
#endif