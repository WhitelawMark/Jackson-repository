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

#ifndef __LSM6DS3_H__
#define __LSM6DS3_H__

#ifdef __cplusplus
extern "C" {
#endif
/*
********************************************************************************
********************************************************************************
*/ 
#define LSM6DS3_WHO_AM_I                0X0F // 能读出ID,根据芯片不同而不同,可能是0x69,也或许是0x6a
#define LSM6DS3_CTRL1_XL                0X10 // 线性加速度传感器控制寄存器1 (r/w)。
/*  bit7                        ~                          bit0
    ODR_XL3 ODR_XL2 ODR_XL1 ODR_XL0 FS_XL1 FS_XL0 BW_XL1 BW_XL0
    其中:ODR_XL3 ODR_XL2 ODR_XL1 ODR_XL0 输出数据速率和电源模式选择
    FS_XL1 FS_XL0：ACC满量程选择
    BW_XL1 BW_XL0：抗混叠滤波器带宽选择。
*/
#define LSM6DS3_CTRL2_G                 0X11
/*  bit7                        ~               bit0
    ODR_G3  ODR_G2 ODR_G1 ODR_G0 FS_G1  FS_G0 FS_125
    其中:ODR_G3  ODR_G2 ODR_G1 ODR_G0:陀螺仪输出速率选择
    FS_G1  FS_G0：陀螺仪满量程选择
    FS_125：陀螺仪满量程为125dps；    
*/
#define LSM6DS3_CTRL3_C                 0X12
/*  bit7                        ~               bit0
    BooT BDU H_LACTIVE PP_OD SIM IF_INC BLE SW_RESET
    BooT:重启内存里的内容
    BDU：更新块数据
    H_LACTIVE：中断活跃水平
    PP_OD：INT1和INT2衬垫上的推拉/开式排水选择。
    SIM：SPI Serial interface mode select
    IF_INC:寄存器地址在进行串行多字节访问时自动增加
    BLE：大/小端数据选择
    SW_RESET：软件复位
*/
#define LSM6DS3_CTRL4_C                 0X13
/*  bit7                        ~               bit0
    XL_BW_SCAL_ODR:加速度计带宽的选择;
    SLEEP_G:陀螺仪睡眠模式使能;
    INT2_on_INT1:所有中断信号在INT1上可用
    FIFO_TEMP_EN:使能稳如数据作为4th FIFO数据设置
    DRDY_MASK:数据读掩码使能;
    I2C_disable:disable i2c接口
    0
    STOP_ON_FTH:启用FIFO阈值级别使用;
*/
#define LSM6DS3_CTRL5_C                 0X14
/*
    ROUNDING2 ROUNDING1 ROUNDING0 0 ST1_G  ST0_G ST1_XL ST0_XL
    ROUNDING2 ROUNDING1 ROUNDING0:从输出寄存器中读取循环突发模式(舍入)。
    ST1_G  ST0_G:角速率传感器自检使能。
    ST1_XL ST0_XL:线性加速度传感器自检使能。
*/

#define LSM6DS3_CTRL6_C                 0X15
/*
    角速率传感器控制寄存器
    TRIG_EN LVLen LVL2_EN XL_HM_MODE 0 0 0 0
    TRIG_EN:陀螺仪数据边缘敏感触发启用。
    LVLen:陀螺仪数据电平敏感触发启用。
    LVL2_EN:陀螺仪电平敏感锁存使能。
    XL_HM_MODE:加速计高性能工作模式禁用.
*/
#define LSM6DS3_CTRL7_G                 0X16
/*
    角速率传感器控制寄存器7
    G_HM_MODE HP_G_EN HPCF_G1 HPCF_G0 HP_G_RST ROUNDING_STATUE 0 0
    G_HM_MODE:陀螺仪的高性能工作模式禁用
    HP_G_EN:陀螺数字高通滤波器使能。只有当陀螺仪处于HP模式时，才启用过滤器。
    HPCF_G1 HPCF_G0:陀螺仪高通滤波器截止频率选择。
    HP_G_RST:陀螺数字HP滤波器复位
    ROUNDING_STATUE:源寄存器四舍五入功能使能在1E,53,1B    
*/
#define LSM6DS3_CTRL8_XL                0X17
/*
    线性加速度传感器控制寄存器8 (r/w)。
    LPF2_XL_EN    HPCF_XL1 HPCF_XL0 0 0 HP_SLOPE_XL_EN 0 LOW_PASS_ON_6D
    LPF2_XL_EN: 加速度计低通滤波器LPF2选择。
    HPCF_XL1 HPCF_XL0:加速度计斜率滤波器和高通滤波器配置和截止设置。
        见表68。它还用于选择LPF2滤波器的截止频率，如表69所示。通过将CTRL8XL (17h)的LOW PASS ON 6D位设置为1，这种低通滤波器也可以用于6D/4D功能。
    HP_SLOPE_XL_EN:加速度计斜率滤波器/高通滤波器选择。
    LOW_PASS_ON_6D:低通滤波器对6D功能的选择。
*/
#define LSM6DS3_CTRL9_XL                0X18
/*
    0 0 Zen_XL Yen_XL Xen_XL SOFT_EN 0 0
    Zen_XL:加速计z轴输出启用。
    Yen_XL:加速计y轴输出启用。
    Xen_XL：加速计x轴输出启用.
    SOFT_EN:启用磁强计软铁校正算法
*/
#define LSM6DS3_CTRL10_C                0X19
/*
    0 0 Zen_G Yen_G Xen_G FUNC_EN FEDO_RST_STEP SIGN_MOTION_EN
    Zen_G:陀螺偏航轴(Z)输出使能。
    Yen_G:陀螺滚轴(Y)输出使能。
    Xen_G:陀螺螺距轴(X)输出使能。
    FUNC_EN:启用嵌入式功能(计步器、倾斜、显著运动、传感器轮毂和熨烫)和加速度计HP和LPF2滤波器
        (参见图6)。默认值:0
    FEDO_RST_STEP:重置计步器步长计数器。
    SIGN_MOTION_EN:使能重要运动功能
*/

//i2c master config register
#define LSM6DS3_MASTER_CONFIG           0X1A

//interrupts register
#define LSM6DS3_WAKE_UP_SRC             0X1B
/*
    0 0 FF_IA SLEEP_STATE_IA WU_IA X_WU Y_WU Z_WU
    FF_IA:自由落体时间检测状态
    SLEEP_STATE_IA:睡眠时间状态
    WU_IA:唤醒时间检测状态
    X_WU：x轴上的唤醒事件检测状态。
    Y_WU: y轴上的唤醒事件检测状态。
    Z_WU: z轴上的唤醒事件检测状态。
*/
#define LSM6DS3_TAP_SRC                 0X1C
/*
    0 TAP_IA SIGLE_TAP DOUBLE_TAP TAP_SIGN X_TAP Y_TAP Z_TAP
    TAP_IA:轻击事件检测状态
    SIGLE_TAP:单击事件检测状态
    DOUBLE_TAP:双击事件检测状态
    TAP_SIGN：轻击事件检测到的加速标志。
    X_TAP Y_TAP Z_TAP：点击x/y/z轴上的事件检测状态
*/
#define LSM6DS3_D6D_SRC                 0X1D
/*
    纵向、横向、面朝上和面朝下源寄存器(r)
    0 D6D_IA ZH ZL YH YL XH XL 
    D6D_IA：激活中断以改变位置纵向，横向，正面向上，正面向下。
    ZH: Z轴高事件(高于阈值)
    ZL：Z轴低事件(低于阈值)
    ...
*/
//status data register
#define LSM6DS3_STATUS_REG              0X1E
/*
    - - - - - TDE GDA XLDA
    TDE:温度可用的新数据
    GDA:陀螺仪可用的新数据
    XLDA:加速度计可用的新数据
*/
#define LSM6DS3_TAP_CFG            0x58
#define LSM6DS3_TAP_THS_6D        0x59
#define LSM6DS3_WAKE_UP_THS            0x5B
#define LSM6DS3_WAKE_UP_DUR        0x5C
#define LSM6DS3_MD1_CFG            0x5E

//temperature output data register
#define LSM6DS3_OUT_TEMP_L              0X20
#define LSM6DS3_OUT_TEMP_H              0X21
/*温度数据输出寄存器(r)。L寄存器和H寄存器一起表示一个16位的字。*/

//gyroscope output register
#define LSM6DS3_OUTX_L_G                0X22
#define LSM6DS3_OUTX_H_G                0X23
#define LSM6DS3_OUTY_L_G                0X24
#define LSM6DS3_OUTY_H_G                0X25
#define LSM6DS3_OUTZ_L_G                0X26
#define LSM6DS3_OUTZ_H_G                0X27

//acc output register
#define LSM6DS3_OUTX_L_XL               0X28
#define LSM6DS3_OUTX_H_XL               0X29
#define LSM6DS3_OUTY_L_XL               0X2A
#define LSM6DS3_OUTY_H_XL               0X2B
#define LSM6DS3_OUTZ_L_XL               0X2C
#define LSM6DS3_OUTZ_H_XL               0X2D

#define LSM6DS3_INT1_CTRL        0X0D
/*
    Write INT1_CTRL = 01h // Acc Data Ready interrupt on INT1
    Write INT1_CTRL = 02h // Gyro Data Ready interrupt on INT1
*/

#define I2C_Handle            hi2c1
#define LSM6DS3_SlaveAddress        0xd4
#define I2C_Status            HAL_StatusTypeDef
#define LSM6DS3_DeviceID        0x6a

//#define USER_APP_INTERFACE
#undef    USER_APP_INTERFACE

// 操作模式读/写
typedef enum 
{
    eRead = 0u,
    eWrite = 1u,
} OPmode_t;

// 查询状态标志
typedef enum{
    STATUS_XLDA_ACC_E = 0x01, // acc 数据可读
    STATUS_GDA_GYRO_E = 0x02, // gyro 数据可读
    STATUS_TDA_TEMP_E = 0x04, // temp 数据可读
}LSM6DS3_STATUS_t;

// 数据类型
typedef struct 
{
    int16_t data_raw_acceleration[3];
    int16_t data_raw_angular_rate[3];
    int16_t data_raw_temperature;
    float acceleration_mg[3];
    float angular_rate_mdps[3];
    float temperature_degC;
    uint8_t whoamI;
    uint8_t uReaddata;
    uint8_t orientation;
} sLSM6DS3_Data;

typedef struct 
{
    /* I2C句柄，使用IO模拟I2C则设置句柄为NULL */
    void *handle;
    /* 读数据 */
    sLSM6DS3_Data m_dataBuf;
    /* 设备从机地址 */
    uint8_t slaveAddress;
    /* 设备ID */
    uint8_t deviceID;

    // 接口函数区域
    void (*Init)(void);
    uint8_t (*ReadByte)(uint8_t reg);
    void (*WriteByte)(uint8_t reg, uint8_t data);
    uint16_t (*ReadMultiple)(uint8_t reg_begin, uint8_t reg_end, uint8_t *data);
    
    // 扩展用户接口，默认为弱定义，用户可自行实现
    uint8_t (*ReadOrigin)(void);
    uint8_t (*ReadWhoAmI)(void);
    int16_t *(*ReadAngularRateRaw)(int16_t *pbuf);
    int16_t *(*ReadAccelerationRaw)(int16_t *pbuf);
    int16_t *(*ReadTemperatureRaw)(int16_t *pbuf);
    
} sLSM6DS3_Dev;
 
// 声明对象
extern sLSM6DS3_Dev g_lsm6ds3;

/*
********************************************************************************
********************************************************************************
*/ 

/*
********************************************************************************
********************************************************************************
*/ 

#ifdef __cplusplus
}
#endif

#endif 
// #ifndef __LSM6DS3_H__

