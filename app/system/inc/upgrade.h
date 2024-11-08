/*
 * updata.h
 *
 *  Created on: 2023年7月17日
 *      Author: lwp
 */
#ifndef __UPGRADE_H_
#define __UPGRADE_H_

u8_t __FlashReadByte(u32_t address);
u8_t __FlashReadStr(u32_t address,u8_t *pdata,u16_t ulen);
s8_t __FlashWriteStr(u32_t address,u8_t *pdata,u16_t ulen);
u8_t __FlashWriteByte(u32_t address,u8_t data);
/*
********************************************************************************
********************************************************************************
*/
void upgrade_init(void);
unsigned short CalFlashCrc16(unsigned int address,unsigned int len) ;
unsigned short CalFileBinCrc16(int fd,unsigned int address,unsigned int len) ;
int file_size_and_crc(const char *filename, unsigned int *p_size, unsigned short *p_crc);
s8_t upgrade_from_udisk_to_flash(char *pfile);
rt_err_t monitor_soft_version_switch(void);
u8_t *mpnitor_upgrade_flag(void);
rt_err_t ota_version_rollback(void);

#endif
/* __UPGRADE_H_ */

