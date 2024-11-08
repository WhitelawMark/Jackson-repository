/*
******************************************************************************
Copyright (C),  Fujian <Company>. Co., Ltd.
File name    ： uffs_flashif.c
Description  ： uffs文件系统flash的接口
Compile      ： 
Author       ： emb_wlq
Version      ： v1.0
Date         ： 
Others       ： 
History      ： 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期：2018.09.16         修改者：emb_wlq
   内容：v1.0

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
2）...

******************************************************************************
*/
/* 头文件 */
#include <rtthread.h>

#include "w25qxx.h"

#include "dfs_uffs.h"

/*
******************************************************************************
                              局部宏定义
******************************************************************************
*/
/* 开启数据写入校验，uffs可配置写入校验，所以此处不开启 */
//#define UFFS_PAGE_WRITE_CHECK_ENBLE

/*
******************************************************************************
                           结构体或联合体定义
******************************************************************************
*/

/*
******************************************************************************
                              外部变量定义
******************************************************************************
*/

/*
******************************************************************************
                              全局变量定义
******************************************************************************
*/
/* page校验缓存 */
#ifdef UFFS_PAGE_WRITE_CHECK_ENBLE
#define W25QXX_PAGE_SIZE 	256
static u8 uffs_CheckPageBuf[W25QXX_PAGE_SIZE];
#endif

/*
******************************************************************************
                              局部函数声明
******************************************************************************
*/

/*
******************************************************************************
                              局部表定义
******************************************************************************
*/

/*
=====================================================================
                              局部函数
=====================================================================
*/
/*
*************************************************************************
Function    : 函数名称
Description : 函数功能、性能描述
Input       : 输入参数说明，包括参数的作用、取值说明及参数间关系。
Output      : 输出参数说明。
Return      : 函数返回值说明
Others      : 其它说明
************************************************************************
*/
/*
*************************************************************************
Function      : uffs_InitFlash
Description   : 初始化FALSH
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int uffs_InitFlash(uffs_Device *dev)
{
    return UFFS_FLASH_NO_ERR;
}

/*
*************************************************************************
Function      : uffs_ReleaseFlash
Description   : 释放FALSH
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int uffs_ReleaseFlash(uffs_Device *dev)
{
    return UFFS_FLASH_NO_ERR;
}

/*
*************************************************************************
Function      : uffs_EraseBlock
Description   : 擦除块
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int uffs_EraseBlock(uffs_Device *dev, u32 block)
{
    int res;
    pst_flash_device_t flash_dev;

    flash_dev = (pst_flash_device_t)dev->_private;

    res = rt_device_control((rt_device_t)flash_dev, FLASH_CMD_ERASE_SECTOR, (void *)&block);
    return (res == RT_EOK ? UFFS_FLASH_NO_ERR : UFFS_FLASH_IO_ERR);
}

/*
*************************************************************************
Function      : uffs_ReadPage
Description   : 页读函数
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int uffs_ReadPage(uffs_Device *dev, u32 block, u32 page, u8 *data, int data_len, u8 *ecc,
                        u8 *spare, int spare_len)
{
    u8 val;
    int ret = UFFS_FLASH_NO_ERR;
    u32 page_size, block_size;
    pst_flash_device_t flash_dev;

    page_size = dev->attr->page_data_size + dev->attr->spare_size;
    block_size = page_size * dev->attr->pages_per_block;
    flash_dev = (pst_flash_device_t)dev->_private;

    /* 读取页数据 */
    if (data && data_len > 0) 
    {
        rt_device_read((rt_device_t)flash_dev, block*block_size+page*page_size, (void *)data, data_len);
    }

    /* 读取spare数据 */
    if (spare && spare_len > 0) 
    {
        rt_device_read((rt_device_t)flash_dev, block*block_size+page*page_size+dev->attr->page_data_size,
                        (void *)spare, spare_len);
    }

    /* 如果不是读取页数据和spare数据，读取坏块标识 */
    if (data == NULL && spare == NULL) 
    {
        val = 0;
        rt_device_read((rt_device_t)flash_dev, block*block_size+page*page_size+dev->attr->page_data_size+dev->attr->block_status_offs,
                        (void *)&val, 1);
        ret = (val == 0xFF ? UFFS_FLASH_NO_ERR : UFFS_FLASH_BAD_BLK);
    }

    return ret;
}

/*
*************************************************************************
Function      : uffs_WritePage
Description   : 页写函数
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
static int uffs_WritePage(uffs_Device *dev, u32 block, u32 page,
                            const u8 *data, int data_len, const u8 *spare, int spare_len)
{
    u8 val;
    int ret = UFFS_FLASH_NO_ERR;
    u32 page_size, block_size;
    pst_flash_device_t flash_dev;

    page_size = dev->attr->page_data_size + dev->attr->spare_size;
    block_size = page_size * dev->attr->pages_per_block;
    flash_dev = (pst_flash_device_t)dev->_private;
    
    /* 写页数据 */
    if (data && data_len > 0) 
    {
        rt_device_write((rt_device_t)flash_dev, block*block_size+page*page_size, (void *)data, data_len);

        /* 读取校验 */
    #ifdef UFFS_PAGE_WRITE_CHECK_ENBLE
        rt_device_read((rt_device_t)flash_dev, block*block_size+page*page_size, (void *)uffs_CheckPageBuf, data_len);
        if (rt_memcmp(data, uffs_CheckPageBuf, data_len) != 0)ret = UFFS_FLASH_IO_ERR;
    #endif
    }

    if (ret != UFFS_FLASH_NO_ERR)
        goto ext;

    /* 写spare数据 */
    if (spare && spare_len > 0) 
    {
        rt_device_write((rt_device_t)flash_dev, block*block_size+page*page_size+dev->attr->page_data_size,
                        (void *)spare, spare_len);

        /* 读取校验 */
    #ifdef UFFS_PAGE_WRITE_CHECK_ENBLE
        rt_device_read((rt_device_t)flash_dev, block*block_size+page*page_size+dev->attr->page_data_size, 
                        (void *)uffs_CheckPageBuf, spare_len);
        if (rt_memcmp(spare, uffs_CheckPageBuf, spare_len) != 0)ret = UFFS_FLASH_IO_ERR;
    #endif
    }

    /* 如果不是写入页数据和spare数据，标识坏块 */
    if (data == NULL && spare == NULL) 
    {
        val = 0;
        rt_device_write((rt_device_t)flash_dev, block*block_size+page*page_size+dev->attr->page_data_size+dev->attr->block_status_offs,
                        (void *)&val, 1);

        /* 读取校验 */
    #ifdef UFFS_PAGE_WRITE_CHECK_ENBLE
        rt_device_read((rt_device_t)flash_dev, block*block_size+page*page_size+dev->attr->page_data_size+dev->attr->block_status_offs,
                        (void *)&val, 1);
        if (val != 0)ret = UFFS_FLASH_IO_ERR;
    #endif
    }

ext:

    return ret;
}

const uffs_FlashOps uffs_Ops =
{
    uffs_InitFlash,    /* InitFlash() */
    uffs_ReleaseFlash, /* ReleaseFlash() */
    uffs_ReadPage,     /* ReadPage() */
    NULL,                /* ReadPageWithLayout */
    uffs_WritePage,    /* WritePage() */
    NULL,               /* WirtePageWithLayout */
    NULL,               /* IsBadBlock(), let UFFS take care of it. */
    NULL,               /* MarkBadBlock(), let UFFS take care of it. */
    uffs_EraseBlock,   /* EraseBlock() */
};

/*
=====================================================================
                              全局函数
=====================================================================
*/
/*
*************************************************************************
Function    : 函数名称
Description : 函数功能、性能描述
Input       : 输入参数说明，包括参数的作用、取值说明及参数间关系。
Output      : 输出参数说明。
Return      : 函数返回值说明
Others      : 其它说明
************************************************************************
*/
/*
*************************************************************************
Function      : uffsSetupStorage
Description   : 初始化存储设备的参数
Input         : 
Output        : 
Return        : 
Others        : 
************************************************************************
*/
void uffsSetupStorage(struct uffs_StorageAttrSt *attr, st_flash_device_t *flash)
{
    rt_memset(attr, 0, sizeof(struct uffs_StorageAttrSt));

    attr->total_blocks = flash->sector_total;                /* total blocks */
    attr->pages_per_block = flash->pages_per_sector;         /* pages per block */
    if (flash->page_size > 512)
    {
        attr->page_data_size = flash->page_size - 32;          /* page data size */
        attr->spare_size = 32;                                  /* page spare size */
    }
    else
    {
        attr->page_data_size = flash->page_size - 16;          /* page data size */
        attr->spare_size = 16;  
    }
    attr->block_status_offs = 4;                             /* block status offset is 5th byte in spare */
    attr->ecc_opt = RT_CONFIG_UFFS_ECC_MODE;                 /* ecc option */
    attr->layout_opt = RT_CONFIG_UFFS_LAYOUT;                /* let UFFS do the spare layout */
}


