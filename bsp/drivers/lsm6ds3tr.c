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

#include "w25qxx.h"

/*
********************************************************************************
********************************************************************************
*/

#define LSM6DS3TRC_STATUS_TEMPERATURE    0x04
#define LSM6DS3TRC_STATUS_GYROSCOPE        0x02
#define LSM6DS3TRC_STATUS_ACCELEROMETER    0x01





/*
********************************************************************************
********************************************************************************
*/

#define LSM6DS3TRC_STATUS_REG       0x1E
#define LSM6DS3TRC_OUTX_L_XL        0x28
#define LSM6DS3TRC_OUTX_L_G         0x22
#define LSM6DS3TRC_OUT_TEMP_L       0x20


/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    :  LSM6DS3TRC_ReadCommand
*Description :  
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
uint8_t LSM6DS3TRC_ReadCommand(uint8_t reg,uint8_t *buf,uint8_t len)
{


}
/*
********************************************************************************
*Function    :  
*Description :  
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
uint8_t LSM6DS3TRC_Get_Status(void)
{
    uint8_t buf[1] = {0};
    
    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_STATUS_REG, buf, 1);
    
    return buf[0];
}
/*
获取加速度数据
加速度传感器中的"±2g"、“±4g”、“±8g”、“±16g"表示传感器的测量范围或量程，单位为"g”，代表重力加速度。
"g"是地球表面上的重力加速度，大约等于9.8米/秒²。因此，传感器的量程为"±2g"意味着它可以测量从-2g到+2g的加速度范围，即-19.6米/秒²到+19.6米/秒²。同样地，"±4g"的量程为-39.2米/秒²到+39.2米/秒²，"±8g"的量程为-78.4米/秒²到+78.4米/秒²，"±16g"的量程为-156.8米/秒²到+156.8米/秒²。
因此，这些不同的量程表示了传感器可以测量的加速度范围大小。选取适当的量程取决于应用的需求。如果预计测量的加速度值不会超过±2g，那么选择"±2g"的传感器就足够了，而如果需要测量更大范围的加速度，就需要选择具有更高量程的传感器。选择合适的量程可以确保传感器在测量过程中不会超出其最大测量范围，从而提供准确的测量结果。
在参考代码中，当STATUS_REG（0x1E）中的XLDA位为1时，表示已成功获取加速度数据，可以进行读取操作。
*/

/*
********************************************************************************
LSM6DS3TRC Get Acceleration Value
********************************************************************************
*/

/*
********************************************************************************
*Function    :  
*Description :  
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
void LSM6DS3TRC_Get_Acceleration(uint8_t fsxl, float *acc_float)
{
    uint8_t buf[6];
    int16_t acc[3];

    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_OUTX_L_XL, buf, 6);
    acc[0] = buf[1] << 8 | buf[0];acc[1] = buf[3] << 8 | buf[2];
    acc[2] = buf[5] << 8 | buf[4];
    
    switch (fsxl){
        case LSM6DS3TRC_ACC_FSXL_2G:
            acc_float[0] = ((float)acc[0] * 0.061f);
            acc_float[1] = ((float)acc[1] * 0.061f);
            acc_float[2] = ((float)acc[2] * 0.061f);
            break;
        case LSM6DS3TRC_ACC_FSXL_16G:
            acc_float[0] = ((float)acc[0] * 0.488f);
            acc_float[1] = ((float)acc[1] * 0.488f);
            acc_float[2] = ((float)acc[2] * 0.488f);
            break;
        case LSM6DS3TRC_ACC_FSXL_4G:
            acc_float[0] = ((float)acc[0] * 0.122f);
            acc_float[1] = ((float)acc[1] * 0.122f);
            acc_float[2] = ((float)acc[2] * 0.122f);
            break;
        case LSM6DS3TRC_ACC_FSXL_8G:
            acc_float[0] = ((float)acc[0] * 0.244f);
            acc_float[1] = ((float)acc[1] * 0.244f);
            acc_float[2] = ((float)acc[2] * 0.244f);
            break;
    }
}


/*
********************************************************************************
LSM6DS3TRC Get Gyroscope Value
********************************************************************************
*/

/*
********************************************************************************
*Function    :  
*Description :  
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
void LSM6DS3TRC_Get_Gyroscope(uint8_t fsg, float *gry_float)
{
    uint8_t buf[6];
    int16_t gry[3];
    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_OUTX_L_G, buf, 6);
    gry[0] = buf[1] << 8 | buf[0];
    gry[1] = buf[3] << 8 | buf[2];
    gry[2] = buf[5] << 8 | buf[4];
    switch (fsg){
        case LSM6DS3TRC_GYR_FSG_250:
            gry_float[0] = ((float)gry[0] * 8.750f);
            gry_float[1] = ((float)gry[1] * 8.750f);
            gry_float[2] = ((float)gry[2] * 8.750f);
            break;
        case LSM6DS3TRC_GYR_FSG_500:
            gry_float[0] = ((float)gry[0] * 17.50f);
            gry_float[1] = ((float)gry[1] * 17.50f);
            gry_float[2] = ((float)gry[2] * 17.50f);
            break;
        case LSM6DS3TRC_GYR_FSG_1000:
            gry_float[0] = ((float)gry[0] * 35.00f);
            gry_float[1] = ((float)gry[1] * 35.00f);
            gry_float[2] = ((float)gry[2] * 35.00f);
            break;
        case LSM6DS3TRC_GYR_FSG_2000:
            gry_float[0] = ((float)gry[0] * 70.00f);
            gry_float[1] = ((float)gry[1] * 70.00f);
            gry_float[2] = ((float)gry[2] * 70.00f);
            break;
    }
}/*
********************************************************************************
数据需要除以256在加上25度
********************************************************************************
*/
/*
********************************************************************************
*Function    :  
*Description :  
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
float LSM6DS3TRC_Get_Temperature(void)
{
    uint8_t buf[2];
    int16_t temp;
    
    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_OUT_TEMP_L, buf, 2);
    
    temp = buf[1] << 8 | buf[0];
    
    return (((float)temp / 256.0) + 25.0);
}
/*
********************************************************************************
*Function    :  
*Description :  
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/

void LSM6DS3TRC_demo(void)
{
    /* Infinite loop *//* USER CODE BEGIN WHILE */
    while (1){
        uint8_t status;status = LSM6DS3TRC_Get_Status();
        
        if (status & LSM6DS3TRC_STATUS_ACCELEROMETER){
            float acc[3] = {0};
            LSM6DS3TRC_Get_Acceleration(LSM6DS3TRC_ACC_FSXL_2G, acc);
            printf("\r\nacc:X:%2f,\tY:%2f,\tZ:%2f\r", acc[0], acc[1], acc[2]);
        }
        if (status & LSM6DS3TRC_STATUS_GYROSCOPE){
            float gyr[3] = {0};
            LSM6DS3TRC_Get_Gyroscope(LSM6DS3TRC_GYR_FSG_2000, gyr);
            printf("\rgyr:X:%4.2f,\tY:%4.2f,\tZ:%4.2f\r", gyr[0], gyr[1], gyr[2]);
        }
        if (status & LSM6DS3TRC_STATUS_TEMPERATURE){
            printf("\rtemp:%2f\r\n",
            LSM6DS3TRC_Get_Temperature());
        }
        HAL_Delay(100);                
    }
}