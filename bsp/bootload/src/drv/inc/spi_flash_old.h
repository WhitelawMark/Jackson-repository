 /*
 * spi_flash.h
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */
#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

/*
********************************************************************************
********************************************************************************
*/
// FLASH参数
// {
// 页大小，256
#define W25QXX_PAGE_SIZE            256
// 扇区大小，4K
#define W25QXX_SECTOR_SIZE          4096
// 块大小，64k
#define W25QXX_BLOCK_SIZE           65536

// FLASH容量(Bytes)
#define W25QXX_MAX_SIZE             (32 * 1024 * 1024)

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
void drv_flash_init(void);
unsigned char FlashEraseChip(void);		
unsigned short FlashReadID(void);
unsigned char FlashWriteStr(unsigned int addr,unsigned char *data,unsigned int ulen);		
unsigned char FlashWriteByte(unsigned int addr,unsigned char data);	
unsigned char FlashReadStr(unsigned int addr,unsigned char *data,unsigned int ulen);				
unsigned char FlashReadByte( unsigned int addr );		
void FlashDemo(void);
#endif 
/*_SPI_FLASH_H_*/

