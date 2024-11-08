/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-29     RealThread   first version
 */
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include "app_lib.h"

#include "w25qxx.h"




#define W25QX_Enable()      HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,   GPIO_PIN_RESET);
#define W25QX_Disable()     HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,   GPIO_PIN_SET);

// W25QXX指令表
// {
#define W25X_WriteEnable             0x06
#define W25X_WriteDisable            0x04
#define W25X_ReadStatusReg           0x05
#define W25X_WriteStatusReg          0x01
#define W25X_ReadData                0x03
#define W25X_FastReadData            0x0B
#define W25X_FastReadDual            0x3B
#define W25X_PageProgram             0x02
#define W25X_BlockErase              0xD8
#define W25X_SectorErase             0x20
#define W25X_ChipErase               0xC7
#define W25X_PowerDown               0xB9
#define W25X_ReleasePowerDown        0xAB
#define W25X_DeviceID                0xAB
#define W25X_ManufactDeviceID        0x90
#define W25X_JedecDeviceID           0x9F
#define W25X_ResetEnable             0x66
#define W25X_ResetMemory             0x99
                               
#define W25X_FastReadQuad            0x6B
#define W25X_EraseSuspend            0x75
#define W25X_EraseResume             0x7A
// }


static SPI_HandleTypeDef hspi3;

static void W25QXXWaitBusy(void);


/**
* @brief W25QXXWriteEnable
*
* @param None
* @retval None
*/
static void W25QXXWriteEnable(void)
{
    unsigned char cmd = W25X_WriteEnable;

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi3,&cmd,1,1000);

    W25QX_Disable();

    W25QXXWaitBusy();
}
/**
* @brief W25QXXReadID
*
* @param None
* @retval None
*/
static unsigned short W25QXXReadID(void)
{
    unsigned short u16Id = 0;
    unsigned char cmd[4] = {W25X_ManufactDeviceID,0x00,0x00,0x00};

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi3,cmd,4,1000);

    HAL_SPI_Receive(&hspi3,(unsigned char*)&u16Id, 2, 1000);

    W25QX_Disable();

    return u16Id;
}
/**
* @brief W25QXXWriteSR
*
* @param None
* @retval None
*/
static unsigned char W25QXXReadSR(void)
{
    unsigned char value;
    unsigned char cmd = W25X_ReadStatusReg;

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi3,&cmd,1,1000);

    HAL_SPI_Receive(&hspi3,&value, 1, 1000);

    W25QX_Disable();

    return value;
}
/**
* @brief W25QXXWriteSR
*
* @param None
* @retval None
*/
void W25QXXWriteSR(unsigned char SR)
{
    unsigned char cmd[2];

    cmd[0]=W25X_ReadStatusReg;
    cmd[1]=SR;

    HAL_SPI_Transmit(&hspi3,cmd,2,1000);
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
static void MX_SPI3_Init(void)
{
    /* SPI3 parameter configuration*/
    hspi3.Instance = SPI1;
    hspi3.Init.Mode = SPI_MODE_MASTER;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi3.Init.NSS = SPI_NSS_SOFT;
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.CRCPolynomial = 7;

    if (HAL_SPI_Init(&hspi3) != HAL_OK)
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
    static unsigned char inited = 0;

    if (inited == 0) {
        MX_SPI3_Init();
        inited = 1;
    }
  
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
    unsigned char cmd = W25X_ChipErase;
    W25QXXWriteEnable();

    W25QXXWaitBusy();

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi3,(unsigned char *)&cmd,1,1000);

    W25QX_Disable();

    W25QXXWaitBusy();

    return 0;
}
/**
* @brief W25QXXEraseSector
*
* @param None
* @retval None
*/
unsigned int W25QXXEraseSector(unsigned int SectorNum)
{
    unsigned int Addr = SectorNum * W25QXX_SECTOR_SIZE;
    unsigned char cmd[4];

    cmd[0]=W25X_SectorErase;
    cmd[1]=(unsigned char)((Addr) >> 16);
    cmd[2]=(unsigned char)((Addr) >> 8);
    cmd[3]=(unsigned char)Addr;

    W25QXXWriteEnable();

    W25QX_Enable();
    
    HAL_SPI_Transmit(&hspi3,(unsigned char*)cmd,4,1000);

    W25QX_Disable();

    W25QXXWaitBusy();

    return 0;
}
/**
* @brief W25QXXEraseBlock
*
* @param None
* @retval None
*/
unsigned int W25QXXEraseBlock(unsigned int u32BlockNum)
{
    unsigned int Addr = u32BlockNum * W25QXX_BLOCK_SIZE;
    unsigned char cmd[4];

    cmd[0]=W25X_BlockErase;
    cmd[1]=(unsigned char)((Addr) >> 16);
    cmd[2]=(unsigned char)((Addr) >> 8);
    cmd[3]=(unsigned char)Addr;

    W25QXXWriteEnable();

    W25QXXWaitBusy();

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi3,(unsigned char *)&cmd[0],4,1000);

    W25QX_Disable();

    W25QXXWaitBusy();

    return 0;
}
/**
* @brief W25QXXRead
*
* @param None
* @retval None
*/
unsigned char W25QXXRead(unsigned int addr, unsigned char* pbuf, unsigned int ulen)
{
    unsigned char cmd[4];

    cmd[0] = W25X_ReadData;
    cmd[1] = (unsigned char)(addr >> 16);
    cmd[2] = (unsigned char)(addr >> 8);
    cmd[3] = (unsigned char)(addr);
    W25QX_Enable();

    HAL_SPI_Transmit(&hspi3,cmd,4,1000);

    if (HAL_SPI_Receive(&hspi3, pbuf,ulen,1000) != HAL_OK)
    {
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
void W25QXX_Write_Page(unsigned int WriteAddr,unsigned char *pBuffer,unsigned short NumByteToWrite)
 {
    unsigned char cmd[4];

    cmd[0] = W25X_PageProgram;
    cmd[1] = (unsigned char)(WriteAddr >> 16);
    cmd[2] = (unsigned char)(WriteAddr >> 8);
    cmd[3] = (unsigned char)(WriteAddr);

    W25QXXWriteEnable();

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi3,cmd,4,1000);

    if (HAL_SPI_Transmit(&hspi3, pBuffer,NumByteToWrite,1000) != HAL_OK)
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
void W25QXXWrite(unsigned int dst, unsigned char* pdata, unsigned int ulen)
{
    unsigned char cmd[4];
    unsigned int size;

    if ((dst % W25QXX_PAGE_SIZE) != 0)//写入的地址不是整页的地方
    {
        if ((dst / W25QXX_PAGE_SIZE) != ((dst + ulen) / W25QXX_PAGE_SIZE))//写入的长度大于一页
        {
            size = W25QXX_PAGE_SIZE - dst % W25QXX_PAGE_SIZE;//同一页剩余的没写的字节大小
            ulen -= size;//扣掉写入的字节大小，等于剩下要写的
        }
        else
        {
            size = ulen;//跟写入的地址是同一页，要写入的少于一页
            ulen = 0;
        }
        cmd[0] = W25X_PageProgram;//页编程命令
        cmd[1] = (unsigned char)(dst >> 16);
        cmd[2] = (unsigned char)(dst >> 8);
        cmd[3] = (unsigned char)(dst);

        W25QXXWriteEnable();

        W25QX_Enable();

        HAL_SPI_Transmit(&hspi3,cmd,4,1000);

        if (HAL_SPI_Transmit(&hspi3, pdata,size,1000) != HAL_OK)
        {
            return ;
        }
        W25QX_Disable();

        W25QXXWaitBusy();

        dst += size;// 写入的指针往后移动SIZE大小
        pdata+=size;//要写入的内容指针往后移动SIZE大小
    }
    while (ulen > 0)//如果写入的长度大于0
    {
        if (ulen < W25QXX_PAGE_SIZE)//小于1页
        {
            size = ulen;//写入的大小等于长度
            ulen = 0;
        }
        else  
        {
            size = W25QXX_PAGE_SIZE;//如果大于一页 ，要写入大小为页大小
            ulen -= W25QXX_PAGE_SIZE;//写入的长度在减去写入的大小
        }

        cmd[0] = W25X_PageProgram;
        cmd[1] = (unsigned char)(dst >> 16);
        cmd[2] = (unsigned char)(dst >> 8);
        cmd[3] = (unsigned char)(dst);

        W25QXXWriteEnable();

        W25QX_Enable();
        HAL_SPI_Transmit(&hspi3,cmd,4,1000);

        if (HAL_SPI_Transmit(&hspi3, pdata,size,1000) != HAL_OK)
        {
            return ;
        }
        W25QX_Disable();
        W25QXXWaitBusy();
        dst += size;//下一页flash
        pdata+=size;//写入内容向下移动
    }
}
#if 0
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
INIT_DEVICE_EXPORT(rt_hw_spi_flash_init);

static unsigned int W25QXXFlashOpen(rt_device_t dev, rt_uint16_t oflag)
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
static unsigned int W25QXXFlashInit(rt_device_t dev)
{

    return RT_EOK;
}
static unsigned int W25QXXFlashClose(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t W25QXXFlashRead(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    pst_flash_device_t pdev = (pst_flash_device_t)dev;

    if ((pos + size) > pdev->byte_total)
    {
        size = pdev->byte_total - pos;
    }

    W25QXXRead(pos, buffer, size);

    return size;
}
static rt_size_t W25QXXFlashWrite(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    pst_flash_device_t pdev = (pst_flash_device_t)dev;

    if ((pos + size) > pdev->byte_total)
    {
        size = pdev->byte_total - pos;
    }

    W25QXXWrite(pos, (unsigned char*)buffer, size);

    return size;
}
static unsigned int W25QXXCheckSector(unsigned int u32SectorNum)
{
#if 0
    unsigned char v[W25QXX_PAGE_SIZE];
    unsigned int i, j, u32DstAddr = u32SectorNum * W25QXX_SECTOR_SIZE;

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
static unsigned int W25QXXFlashControl(rt_device_t dev, int cmd, void *args)
{
    unsigned int u32SectorNum;
    unsigned int ret;
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
                u32SectorNum = *(unsigned int *)args;
                ret = W25QXXEraseSector(u32SectorNum);
            }
            break;
        case FLASH_CMD_CHECK_SECTOR:
            if (NULL != args)
            {
                u32SectorNum = *(unsigned int *)args;
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
                *(unsigned int *)args = W25QXXReadID();
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
INIT_DEVICE_EXPORT(SFLASH_W25QXX_Init);
#endif
unsigned char prdbuf[4096];
unsigned char pwrbuf[4096];
void SPIFlashDemo(void)
{
    unsigned int addr =0;
    unsigned int page =0;
    unsigned short  size =10;
    unsigned int   ret=0;

   

     page = 0;
     size = 4096;


    addr = page;

    for (int var = 0; var < size; ++var){
        pwrbuf[var]=var%256;
    }
    kprintf("w25_demo spi flash addr = %d size =%d\n",addr,size);

    W25QXXEraseSector(addr/W25QXX_SECTOR_SIZE);
    
    W25QXXRead( addr, prdbuf, size);
    
    W25QXXRead( addr+32, prdbuf+32, size);
    
    W25QXXWrite( addr, pwrbuf, size);

    W25QXXRead( addr, prdbuf, size);

    for (int var = 0; var < size; var++)
    {
        if(prdbuf[var]!=pwrbuf[var])
        {
          //  rt_kprintf("miss:w25_wrbuf[%d]=%d != w25_rdbuf[%d]=%d\n",var,pwrbuf[var],var,prdbuf[var]);
            ret=1;
        }
    }
    if(ret==0)
    {
        kprintf("w25_demo spi flash test ok \n");
    }
    else
    {
        kprintf("w25_demo spi flash test fail \n");
    }

}


