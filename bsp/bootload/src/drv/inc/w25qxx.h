 /*
 * w25qxx.c
 *
 *  Created on: 2023年10月19日
 *      Author: lwp edit
 */
#ifndef __W25QXX_H__
#define __W25QXX_H__

/*
********************************************************************************
********************************************************************************
*/
#ifdef __cplusplus
extern "C" {
#endif

  
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
#define W25QXX_MAX_SIZE             (32 * 1024 * 1024)
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
int SFLASH_W25QXX_Init(void);
unsigned char FlashEraseChip(void);		
unsigned short FlashReadID(void);
unsigned char FlashWriteStr(unsigned int addr,unsigned char *data,unsigned int ulen);		
unsigned char FlashWriteByte(unsigned int addr,unsigned char data);	
unsigned char FlashReadStr(unsigned int addr,unsigned char *data,unsigned int ulen);				
unsigned char FlashReadByte( unsigned int addr );	
#ifdef __cplusplus
}
#endif

#endif // #ifndef __W25QXX_H__

