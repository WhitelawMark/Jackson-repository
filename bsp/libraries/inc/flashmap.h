/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */
#ifndef __FLASH_MAP_H
#define __FLASH_MAP_H 	

/*
********************************************************************************
********************************************************************************
*/
#define UPDATA_NONE_ERR        0
#define UPDATA_CRC_ERR         1
#define UPDATA_LEN_ERR         2
#define UPDATA_ABNO_ERR        3
/* FLASH_UPDATA_FLAG 升级标识定义 */
#define UPDATA_SUCCESS          (0)
#define UPDATA_CHECK            (1)
#define UPDATA_CRASH            (2)
/*
********************************************************************************
********************************************************************************
*/
#define FLASH_UPDATA_FLAG     0x00000000  
#define FLASH_USVS_FLAG       0x00001000
#define FLASH_ERAM_BK_ADDR    0x00002000
#define FLASH_SYS_FAULT_FLG   0x00003000
#define FLASH_BLACK_NUM       0x00004000

#define FLASH_ERROR_FLG       0x00006000/*错误标注地址*/
#define FLASH_LBERR_FLG       0x00007000
#define FLASH_BOOTVS          0x00008000
#define FLASH_BOOTUPFLG       0x00009000
#define FLASH_ACTIVATION      0x0000A000
#define FLASH_CODE_SIZE       0x0007FFFF
 
#define FLASH_VS1_ADDR        0x00010000
#define FLASH_VS1_END         FLASH_VS1_ADDR+FLASH_CODE_SIZE

#define FLASH_VS2_ADDR        0x00090000
#define FLASH_VS2_END         FLASH_VS2_ADDR+FLASH_CODE_SIZE 

#define FLASH_BLCODE_SIZE     0x00007FF0
#define FLASH_BL_ADDR         0x00110000
#define FLASH_BL_END          FLASH_BL_ADDR+FLASH_BLCODE_SIZE

/*
********************************************************************************
********************************************************************************
*/
#define FLASH_CRC_VAL         0x00000     // CRC       偏移量
#define FLASH_CODE_LEN        0x00004     // 代码长度  代码总长度
#define FLASH_VERS_STA        0x00008     // 版本状态
#define FLASH_FILE_MD5        0x0000B     // 文件的MD5
#define FLASH_TARN_BKP        0x00100     // 断点信息

#define FLASH_CODE_OFFER      0x01000     // 代码偏移 

/*
********************************************************************************
********************************************************************************
*/
#define FLASH_PP_FLAG         0x00150000
#define FLASH_DATA_ZONE       0x00151000
#define FLASH_BACK_ZONE       0x00152000
#endif
/*__FLASH_MAP_H*/ 