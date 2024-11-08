 /*
 * codebackup.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */

#include "app_lib.h"
/*
********************************************************************************
********************************************************************************
*/ 
extern u32_t  LocalRxTotSize;
extern u32_t NewCodeBaseAddress;
extern u32_t BackupBaseAddress;
extern u32_t CodeLen;
extern unsigned char ReadCode[READ_CODE_TEMP_LEN];

u32_t VS1BaseAddres;
u32_t VS2BaseAddres;
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  BlCalFlashCrc16
**  功能描述:  从指定地址的FLASH开始计算CRC值，长度为len
**  输入参数:  address : 地址
**             len     : 长度
**  输出参数:  无
**  返回参数:  计算的CRC值
********************************************************************************
*/
unsigned short Flash_If_CalCrc16(unsigned int address,unsigned int len) 
{    
    unsigned short crc;
    unsigned char temp;
    unsigned char data;
    unsigned char tempbuf[4];
    unsigned char i;
    unsigned int AddRess;
    crc = 0;
    AddRess=address;

    while(len) {
        FLASH_If_Read(&AddRess,(uint32_t *) tempbuf, 1 );
        i = 0;
        while (i < 4) {

            data = tempbuf[i];

            temp = ((unsigned char)(crc >> 8)) >> 4;   
            crc <<= 4;                            
            crc ^= UstrCrcTable[temp^(data >> 4)];                


            temp = ((unsigned char)(crc >> 8)) >> 4;    
            crc <<= 4;                                
            crc ^= UstrCrcTable[temp^(data & 0x0f)]; 
            i++;
        }
                len--;
    }

    return crc;
}

/*
********************************************************************************
**  函数名称:  BlUpdataCodeBackupMsgSave
**  功能描述:  代码备份信息存储
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char BlUpdataCodeBackupMsgSave(void)
{
    __IO uint32_t  FlashAddress;
    u16_t crcval;

    FlashAddress = APPLICATION_ADDRESS;
    bl_log(LG_DBG,"updata code rev code len is 0x%x", LocalRxTotSize);

    crcval = Flash_If_CalCrc16(FlashAddress,LocalRxTotSize / 4);

    bl_log(LG_DBG,"updata code backup crc is 0x%x ",crcval);

    VS1BaseAddres = FLASH_VS1_ADDR;

    VS2BaseAddres = FLASH_VS2_ADDR;

    FlashWriteStr(VS1BaseAddres + FLASH_CRC_VAL, (u8_t*)&crcval, 2 );
    FlashWriteStr(VS1BaseAddres + FLASH_CODE_LEN, (u8_t*)&LocalRxTotSize, 4 );

    FlashWriteStr(VS2BaseAddres + FLASH_CRC_VAL, (u8_t*)&crcval, 2 );
    FlashWriteStr(VS2BaseAddres + FLASH_CODE_LEN, (u8_t*)&LocalRxTotSize, 4 );

    return 0;
}

/*
********************************************************************************
**  函数名称:  Bl_BackupIntFlashToExFlash
**  功能描述:  内部代码备份到外部作为存储使用
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char Bl_BackupIntFlashToExFlash(void)
{
    __IO uint32_t  FlashAddress;
    u32_t vs1addr;
    u32_t vs2addr;
    u32_t totwrlen = 0;
    u16_t ulen=1024;

    FlashAddress = APPLICATION_ADDRESS;

    vs1addr = VS1BaseAddres + FLASH_CODE_OFFER;
    vs2addr = VS2BaseAddres + FLASH_CODE_OFFER;

    for (; totwrlen < LocalRxTotSize; ) {
        FLASH_If_Read(&FlashAddress, (uint32_t *)ReadCode, ulen / 4 );
        FlashWriteStr(vs1addr, ReadCode, ulen);
        FlashWriteStr(vs2addr, ReadCode, ulen);
        vs1addr += ulen;
        vs2addr += ulen;
        totwrlen += ulen;
    }

    return 0;
}
/*
********************************************************************************
**  函数名称:  Bl_UpdataCodeBackup                                         
**  功能描述:  备份现有的代码（将待升级的代码复制一份到外部存储器中）      
**  输入参数:  无                                                          
**  输出参数:  无                                                          
**  返回参数:  无                                                          
********************************************************************************
*/
unsigned char Bl_UpdataCodeBackup(void)
{
    bl_log(LG_DBG,"code back up");
    LocalRxTotSize = USER_FLASH_SIZE;
    // LocalRxTotSize =10;
    BlUpdataCodeBackupMsgSave();
    Bl_BackupIntFlashToExFlash();

    return 0;                                                              
}
/*
********************************************************************************
**  函数名称:  Bl_UpdataCodeBackup                                         
**  功能描述:  备份现有的代码（将待升级的代码复制一份到外部存储器中）      
**  输入参数:  无                                                          
**  输出参数:  无                                                          
**  返回参数:  无                                                          
********************************************************************************
*/
u8_t Bl_UpdataGetRecoveryMsg(void)
{
    u8_t version = 0;

    version = FlashReadByte(FLASH_USVS_FLAG);
    if (version == 1) {
        NewCodeBaseAddress = FLASH_VS2_ADDR;
        bl_log(LG_DBG,"Restore using backup version 2");
    } else {
        NewCodeBaseAddress = FLASH_VS1_ADDR;
        bl_log(LG_DBG,"Restore using backup version 1");
    }

    FlashReadStr(NewCodeBaseAddress + FLASH_CODE_LEN, (u8_t*)&CodeLen, 4);
    bl_log(LG_DBG,"Reference address 0x%4x The length of the code :%8x ",
        NewCodeBaseAddress, CodeLen);
    if (CodeLen > USER_FLASH_SIZE) {
        bl_log(LG_ERR,"Recovery failed: Save code exception");

        return 1;
    }

    return 0;
}

/*
********************************************************************************
**  函数名称:  Bl_UpdataCodeRecovery                                            
**  功能描述:  还原备份代码(升级完成后，引导程序会将升级标设置为2，而应用程序检 
**              测到标志位2时会将该标志清除为0,                                 
**             当引导程序运行时检测到该标志为2时会启动代码恢复操作)             
**  输入参数:  无                                                               
**  输出参数:  无                                                               
**  返回参数:  无                                                               
********************************************************************************
*/
unsigned char  Bl_UpdataCodeRecovery(void)
{
    /*读取外部FLASH 特定的升级标注地址的值，如果不等于2 直接退出来，*/
    if (FlashReadByte(FLASH_UPDATA_FLAG) != 2 && Bl_GetLocalDownLoad_Flg() != 1) {
        return 0;
    }

    if (Bl_UpdataGetRecoveryMsg()) {
        return 0;
    }

    if (CodeUpdataFlieCrcVef()) {
        return 0;
    }

    //代码从外部拷贝到内部存储器
    Bl_CodeCopyToInterFlash();

    Bl_Errorflg(UPDATA_ABNO_ERR);

    return 0;
}

