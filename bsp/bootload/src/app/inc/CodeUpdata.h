 /*
 * CodeUpdata.h
 *
 *  Created on: 2022Äê10ÔÂ19ÈÕ
 *      Author: lwp edit
 */
#ifndef __CODE_UPDATA_H__
#define __CODE_UPDATA_H__
/*
********************************************************************************
********************************************************************************
*/
#define READ_CODE_TEMP_LEN  1024

extern unsigned char ReadCode[READ_CODE_TEMP_LEN];
extern const unsigned int UstrCrcTable[16];
/*
********************************************************************************
********************************************************************************
*/
void VersionMsgRecode(void);
int8_t Bl_CodeCopyToInterFlash(void);
u8_t CodeUpdataFlieCrcVef(void);
void Bl_CodeUPData_Deal(void);

#endif /* __CODE_UPDATA_H__ */

