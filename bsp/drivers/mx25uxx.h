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

#ifndef __MX25UXX_H__
#define __MX25UXX_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
********************************************************************************
********************************************************************************
*/ 

#define FLASH_NONE_ERR            1      
#define FLASH_BUSY_ERR            2       
#define FLASH_PARM_ERR            3       
#define FLASH_ADDR_ERR            4
#define FLASH_OPRD_ERR            5
#define FLASH_OPWR_ERR            6    
#define FLASH_ERASE_ERR           7 
#define FLASH_OTHER_ERR           8
#define FLASH_DONEFAIL            9      
#define FLASH_DONEOK              10   

/*
********************************************************************************
********************************************************************************
*/ 
// IOCTL命令
// {
#define FLASH_CMD_ERASE_SECTOR			0x10	// 扇区除块
#define FLASH_CMD_CHECK_SECTOR       	0x11	// 扇区擦除后检查是否坏块
#define FLASH_CMD_ERASE_CHIP        	0x12	// 片擦除
#define FLASH_CMD_GET_CHIP_ID       	0x13	// 获取芯片ID
#define FLASH_CMD_ERASE_BLOCK           0x14
// }

/*
********************************************************************************
********************************************************************************
*/ 

// 串行Flash设备名称
#define SFLASH_DEVICE_NAME				"flash"

// FLASH参数
// {
// FLASH容量(Bytes)
#define W25QXX_MAX_SIZE             (4 * 1024 * 1024)
// 页大小，256
#define W25QXX_PAGE_SIZE			256
// 扇区大小，4K
#define W25QXX_SECTOR_SIZE			4096
// 块大小，64k
#define W25QXX_BLOCK_SIZE			65536

#define W25QXX_SECTOR_START         ((2 * 1024 * 1024)/W25QXX_SECTOR_SIZE)
 
// FLASH 扇区个数
#define W25QXX_SECTOR_NUM           (W25QXX_MAX_SIZE / W25QXX_SECTOR_SIZE)
// FLASH 一个扇区有几个页
#define W25QXX_PAGE_PER_SECTOR      (W25QXX_SECTOR_SIZE / W25QXX_PAGE_SIZE)
 // FLASH 扇区个数
#define W25QXX_SECTOR_TOTAL         (W25QXX_MAX_SIZE / W25QXX_SECTOR_SIZE)
// }


/*
********************************************************************************
********************************************************************************
*/ 
// FLASH设备定义
typedef struct st_flash_device
{
    struct rt_device parent;		/**< 设备父类 */

	rt_uint32_t byte_total;			/**< 容量 */

    rt_uint16_t page_size;			/**< 页大小 */
    rt_uint16_t oob_size;			/**< 一页额外的空间，软件用于存储校验，用户可以编程 */
    rt_uint16_t oob_free;			/**< 一页额外的空间，硬件用于存储校验，由硬件自动编程 */
	
    rt_uint16_t pages_per_sector;	/**< 一扇区多少页 */
    rt_uint32_t sector_total;		/**< 扇区总数 */
	
    rt_uint32_t sector_start;		/**< 扇区起始序号 */
    rt_uint32_t sector_end;			/**< 扇区结束序号 */
} st_flash_device_t, *pst_flash_device_t;

/*
********************************************************************************
********************************************************************************
*/ 
int W25QXXEraseChip(void);
rt_uint16_t W25QXXReadID(void);
rt_err_t W25QXXEraseSector(rt_uint32_t SectorNum);
rt_uint8_t W25QXXRead(rt_uint32_t addr, rt_uint8_t* pbuf, rt_uint32_t ulen);
void W25QXXWrite(rt_uint32_t dst, rt_uint8_t* pdata, rt_uint32_t ulen);
#ifdef __cplusplus
}
#endif

#endif // #ifndef __W25QXX_H__

