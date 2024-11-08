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
 * ulog cfg
*/
#define LOG_TAG           "drv.w25q"
#define DBG_LVL           DBG_INFO
#include <drv_log.h>

#define W25X_CS_PIN         GET_PIN(D,14)

#define W25QX_Enable()      rt_pin_write(W25X_CS_PIN,PIN_LOW)
#define W25QX_Disable()     rt_pin_write(W25X_CS_PIN,PIN_HIGH)

// W25QXX指令表
// {
rt_uint8_t const W25X_WriteEnable      =      0x06;
rt_uint8_t const W25X_WriteDisable     =      0x04;
rt_uint8_t const W25X_ReadStatusReg    =      0x05;
rt_uint8_t const W25X_WriteStatusReg   =      0x01;
rt_uint8_t const W25X_ReadData         =      0x03;
rt_uint8_t const W25X_ReadData4Byte    =      0x13;
rt_uint8_t const W25X_FastReadData     =      0x0B;
rt_uint8_t const W25X_FastReadDual     =      0x3B;
rt_uint8_t const W25X_PageProgram      =      0x02;
rt_uint8_t const W25X_PageProgram4Byte =      0x12;

rt_uint8_t const W25X_BlockErase       =      0xD8;
rt_uint8_t const W25X_BlockErase4Byte  =      0xDC;
rt_uint8_t const W25X_SectorErase      =      0x20;
rt_uint8_t const W25X_SectorErase4Byte =      0x21;
rt_uint8_t const W25X_ChipErase        =      0xC7;
rt_uint8_t const W25X_PowerDown        =      0xB9;
rt_uint8_t const W25X_ReleasePowerDown =      0xAB;
rt_uint8_t const W25X_DeviceID         =      0xAB;
rt_uint8_t const W25X_ManufactDeviceID =      0x90;
rt_uint8_t const W25X_JedecDeviceID    =      0x9F;
rt_uint8_t const W25X_ResetEnable      =      0x66;
rt_uint8_t const W25X_ResetMemory      =      0x99;

rt_uint8_t const W25X_FastReadQuad     =      0x6B;
rt_uint8_t const W25X_EraseSuspend     =      0x75;
rt_uint8_t const W25X_EraseResume      =      0x7A;
// }

static st_flash_device_t s_stFlashDev;

static SPI_HandleTypeDef hspi;

static void W25QXXWaitBusy(void);


/**
* @brief W25QXXWriteEnable
*
* @param None
* @retval None
*/
static void W25QXXWriteEnable(void)
{
    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,(uint8_t *)&W25X_WriteEnable,1,1000);

    W25QX_Disable();

    W25QXXWaitBusy();
}
/**
* @brief W25QXXReadID
*
* @param None
* @retval None
*/
rt_uint16_t W25QXXReadID(void)
{
    rt_uint16_t u16Id = 0;
    rt_uint8_t cmd[4] = {W25X_ManufactDeviceID,0x00,0x00,0x00};

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,cmd,4,1000);

    HAL_SPI_Receive(&hspi,(rt_uint8_t*)&u16Id, 2, 1000);

    W25QX_Disable();

    return u16Id;
}
/**
* @brief W25QXXWriteSR
*
* @param None
* @retval None
*/
static rt_uint8_t W25QXXReadSR(void)
{
    rt_uint8_t value;
    uint8_t cmd = W25X_ReadStatusReg;

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,&cmd,1,1000);

    HAL_SPI_Receive(&hspi,&value, 1, 1000);

    W25QX_Disable();

    return value;
}
/**
* @brief W25QXXWriteSR
*
* @param None
* @retval None
*/
void W25QXXWriteSR(rt_uint8_t SR)
{
    rt_uint8_t cmd[2];

    cmd[0]=W25X_ReadStatusReg;
    cmd[1]=SR;

    HAL_SPI_Transmit(&hspi,cmd,2,1000);
}
/**
* @brief W25QXXWaitBusy
*
* @param None
* @retval None
*/
static void W25QXXWaitBusy(void)
{
    while ((W25QXXReadSR() & 0x01) == 0x01){
    }
}
/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{
    /* SPI3 parameter configuration*/
    hspi.Instance = SPI1;
    hspi.Init.Mode = SPI_MODE_MASTER;
    hspi.Init.Direction = SPI_DIRECTION_2LINES;
    hspi.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi.Init.NSS = SPI_NSS_SOFT;
    hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi.Init.CRCPolynomial = 7;

    if (HAL_SPI_Init(&hspi) != HAL_OK)
    {
       // Error_Handler();
    }
}
/**
* @brief W25QXXInit
*
* @param None
* @retval None
*/
void W25QXXInit(void)
{
    static rt_uint8_t inited = 0;

    if (inited == 0) {
        MX_SPI1_Init();
        inited = 1;
    }
    rt_pin_mode(W25X_CS_PIN, PIN_MODE_OUTPUT);

    W25QX_Disable();
}
/**
* @brief W25QXXEraseChip
*
* @param None
* @retval None
*/
int W25QXXEraseChip(void)
{

    W25QXXWriteEnable();

    W25QXXWaitBusy();

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,(uint8_t *)&W25X_ChipErase,1,1000);

    W25QX_Disable();

    W25QXXWaitBusy();

    return RT_EOK;
}
//MSH_CMD_EXPORT(W25QXXEraseChip, spi W25QXXEraseChip);
/**
* @brief W25QXXEraseSector
*
* @param None
* @retval None
*/
rt_err_t W25QXXEraseSector(rt_uint32_t SectorNum)
{
    rt_uint32_t Addr = SectorNum * W25QXX_SECTOR_SIZE;
    rt_uint8_t cmd[5];

    cmd[0]=W25X_SectorErase4Byte;
    cmd[1]=(rt_uint8_t)((Addr) >> 24);
    cmd[2]=(rt_uint8_t)((Addr) >> 16);
    cmd[3]=(rt_uint8_t)((Addr) >> 8);
    cmd[4]=(rt_uint8_t)Addr;

    W25QXXWriteEnable();

    W25QX_Enable();
    
    HAL_SPI_Transmit(&hspi,(uint8_t*)cmd,5,1000);

    W25QX_Disable();

    W25QXXWaitBusy();

    return RT_EOK;
}
/**
* @brief W25QXXEraseBlock
*
* @param None
* @retval None
*/
rt_err_t W25QXXEraseBlock(rt_uint32_t u32BlockNum)
{
    rt_uint32_t Addr = u32BlockNum * W25QXX_BLOCK_SIZE;
    uint8_t cmd[5];

    cmd[0]=W25X_BlockErase4Byte;
    cmd[1]=(rt_uint8_t)((Addr) >> 24);
    cmd[2]=(rt_uint8_t)((Addr) >> 16);
    cmd[3]=(rt_uint8_t)((Addr) >> 8);
    cmd[4]=(rt_uint8_t)Addr;

    W25QXXWriteEnable();

    W25QXXWaitBusy();

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,(uint8_t *)&cmd[0],5,1000);

    W25QX_Disable();

    W25QXXWaitBusy();

    return RT_EOK;
}
/**
* @brief W25QXXRead
*
* @param None
* @retval None
*/
rt_uint8_t W25QXXRead(rt_uint32_t addr, rt_uint8_t* pbuf, rt_uint32_t ulen)
{
    rt_uint8_t cmd[5];

    cmd[0] = W25X_ReadData4Byte;
    cmd[1] = (rt_uint8_t)(addr >> 24);
    cmd[2] = (rt_uint8_t)(addr >> 16);
    cmd[3] = (rt_uint8_t)(addr >> 8);
    cmd[4] = (rt_uint8_t)(addr);
    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,cmd,5,1000);

    if (HAL_SPI_Receive(&hspi, pbuf,ulen,5000) != HAL_OK)
    { 
        LOG_E("%s addr %d timeout ",__func__,addr);
        W25QX_Disable();
        return -1;
    }
    W25QX_Disable();

    return 0;
}
/**
* @brief W25QXX_Write_Page
*
* @param None
* @retval None
*/
void W25QXX_Write_Page(rt_uint32_t WriteAddr,rt_uint8_t *pBuffer,rt_uint16_t NumByteToWrite)
 {
    rt_uint8_t cmd[5];

    cmd[0] = W25X_PageProgram4Byte;
    cmd[1] = (rt_uint8_t)(WriteAddr >> 24);
    cmd[2] = (rt_uint8_t)(WriteAddr >> 16);
    cmd[3] = (rt_uint8_t)(WriteAddr >> 8);
    cmd[4] = (rt_uint8_t)(WriteAddr);

    W25QXXWriteEnable();

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,cmd,5,1000);

    if (HAL_SPI_Transmit(&hspi, pBuffer,NumByteToWrite,1000) != HAL_OK)
    {
        return ;
    }
    W25QX_Disable();

    W25QXXWaitBusy();
 }
/**
* @brief W25QXXWrite
*
* @param None
* @retval None
*/
void W25QXXWrite(rt_uint32_t dst, rt_uint8_t* pdata, rt_uint32_t ulen)
{
    rt_uint8_t cmd[5];
    rt_uint32_t size;

    if ((dst % W25QXX_PAGE_SIZE) != 0)
    {
        if ((dst / W25QXX_PAGE_SIZE) != ((dst + ulen) / W25QXX_PAGE_SIZE))
        {
            size = W25QXX_PAGE_SIZE - dst % W25QXX_PAGE_SIZE;
            ulen -= size;
        }
        else
        {
            size = ulen;
            ulen = 0;
        }
        cmd[0] = W25X_PageProgram4Byte;
        cmd[1] = (rt_uint8_t)(dst >> 24);
        cmd[2] = (rt_uint8_t)(dst >> 16);
        cmd[3] = (rt_uint8_t)(dst >> 8);
        cmd[4] = (rt_uint8_t)(dst);

        W25QXXWriteEnable();

        W25QX_Enable();

        HAL_SPI_Transmit(&hspi,cmd,5,1000);

        if (HAL_SPI_Transmit(&hspi, pdata,size,5000) != HAL_OK)
        {
            return ;
        }
        W25QX_Disable();

        W25QXXWaitBusy();

        dst += size;
        pdata+=size;
    }
    while (ulen > 0)
    {
        if (ulen < W25QXX_PAGE_SIZE)
        {
            size = ulen;
            ulen = 0;
        }
        else
        {
            size = W25QXX_PAGE_SIZE;
            ulen -= W25QXX_PAGE_SIZE;
        }

        cmd[0] = W25X_PageProgram4Byte;
        cmd[1] = (rt_uint8_t)(dst >> 24);
        cmd[2] = (rt_uint8_t)(dst >> 16);
        cmd[3] = (rt_uint8_t)(dst >> 8);
        cmd[4] = (rt_uint8_t)(dst);

        W25QXXWriteEnable();

        W25QX_Enable();
        HAL_SPI_Transmit(&hspi,cmd,5,1000);

        if (HAL_SPI_Transmit(&hspi, pdata,size,1000) != HAL_OK)
        {
            return ;
        }
        W25QX_Disable();
        W25QXXWaitBusy();
        dst += size;
        pdata+=size;
    }
}
/**
* @brief rt_hw_spi_flash_init
*
* @param None
* @retval None
*/

static int rt_hw_spi_flash_init(void)
{
    W25QXXInit();
    return RT_EOK;
}
// INIT_DEVICE_EXPORT(rt_hw_spi_flash_init);

static rt_err_t W25QXXFlashOpen(rt_device_t dev, rt_uint16_t oflag)
{
    rt_uint16_t u16Id;

    u16Id = W25QXXReadID();
    if ((u16Id == 0) || (u16Id == 0xffff))
    {
        LOG_E("%s[%d] RT_ERROR \n",__func__,__LINE__);
        return -RT_ERROR;
    }
    else
    {
        LOG_D("%s[%d] OK \n",__func__,__LINE__);
        return RT_EOK;
    }
}
static rt_err_t W25QXXFlashInit(rt_device_t dev)
{

    return RT_EOK;
}
static rt_err_t W25QXXFlashClose(rt_device_t dev)
{
    return RT_EOK;
}

static rt_ssize_t W25QXXFlashRead(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    pst_flash_device_t pdev = (pst_flash_device_t)dev;

    if ((pos + size) > pdev->byte_total)
    {
        size = pdev->byte_total - pos;
    }

    W25QXXRead(pos, buffer, size);

    return size;
}
static rt_ssize_t W25QXXFlashWrite(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    pst_flash_device_t pdev = (pst_flash_device_t)dev;

    if ((pos + size) > pdev->byte_total)
    {
        size = pdev->byte_total - pos;
    }

    W25QXXWrite(pos, (rt_uint8_t*)buffer, size);

    return size;
}
static rt_err_t W25QXXCheckSector(rt_uint32_t u32SectorNum)
{
#if 0
    rt_uint8_t v[W25QXX_PAGE_SIZE];
    rt_uint32_t i, j, u32DstAddr = u32SectorNum * W25QXX_SECTOR_SIZE;

    for (i = 0; i < W25QXX_PAGE_PER_SECTOR; i++)
    {
        rt_memset(v, 0, W25QXX_PAGE_SIZE);
        Sst26FlashSpiRead(u32DstAddr, v, W25QXX_PAGE_SIZE);

        /* 校验 */
        for (j = 0; j < W25QXX_PAGE_SIZE; j++)
        {
            if (v[j] != 0xFF)
                return -RT_ERROR;
        }

        u32DstAddr += W25QXX_PAGE_SIZE;
    }
#endif

    return RT_EOK;
}
static rt_err_t W25QXXFlashControl(rt_device_t dev, int cmd, void *args)
{
    rt_uint32_t u32SectorNum;
    rt_err_t ret;
    pst_flash_device_t pdev = (pst_flash_device_t)dev;

    ret = -RT_ERROR;

    if (pdev->parent.open_flag & RT_DEVICE_OFLAG_OPEN == 0)
    {
        return ret;
    }

    switch (cmd)
    {
        case FLASH_CMD_ERASE_SECTOR:
            if (NULL != args)
            {
                u32SectorNum = *(rt_uint32_t *)args;
                ret = W25QXXEraseSector(u32SectorNum);
            }
            break;
        case FLASH_CMD_CHECK_SECTOR:
            if (NULL != args)
            {
                u32SectorNum = *(rt_uint32_t *)args;
                ret = W25QXXCheckSector(u32SectorNum);
            }
            break;
        case FLASH_CMD_ERASE_CHIP:
            W25QXXEraseChip();
            ret = RT_EOK;
            break;
        case FLASH_CMD_GET_CHIP_ID:
            if (NULL != args)
            {
                *(rt_uint32_t *)args = W25QXXReadID();
                ret = RT_EOK;
            }
            break;
        default:
            break;
    }
    return ret;
}
int SFLASH_W25QXX_Init(void)
{
    int rv;

    rt_hw_spi_flash_init();

    rt_memset(&s_stFlashDev, 0, sizeof(s_stFlashDev));

    s_stFlashDev.parent.type = RT_Device_Class_MTD;
    s_stFlashDev.parent.init = W25QXXFlashInit;
    s_stFlashDev.parent.open = W25QXXFlashOpen;
    s_stFlashDev.parent.close = W25QXXFlashClose;
    s_stFlashDev.parent.read = W25QXXFlashRead;
    s_stFlashDev.parent.write = W25QXXFlashWrite;
    s_stFlashDev.parent.control = W25QXXFlashControl;
    s_stFlashDev.parent.user_data = RT_NULL;

    s_stFlashDev.byte_total = W25QXX_MAX_SIZE;
    s_stFlashDev.page_size = W25QXX_PAGE_SIZE;
    s_stFlashDev.oob_size = 0;
    s_stFlashDev.oob_free = 0;
    s_stFlashDev.pages_per_sector = W25QXX_PAGE_PER_SECTOR;
    s_stFlashDev.sector_end = W25QXX_SECTOR_NUM - 1;
    s_stFlashDev.sector_start = W25QXX_SECTOR_START;
    s_stFlashDev.sector_total = s_stFlashDev.sector_end-s_stFlashDev.sector_start+1;

    rv = rt_device_register(&s_stFlashDev.parent, SFLASH_DEVICE_NAME,
        RT_DEVICE_FLAG_STANDALONE | RT_DEVICE_FLAG_RDWR);

    rt_kprintf("board: W25QXX init pass.\r\n");

    return rv;
}
// INIT_DEVICE_EXPORT(SFLASH_W25QXX_Init);