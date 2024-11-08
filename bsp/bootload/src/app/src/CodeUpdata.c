 /*
 * codeupdata.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */


#include "app_lib.h"
/*
********************************************************************************
********************************************************************************
*/ 
unsigned char ReadCode[READ_CODE_TEMP_LEN];

u32_t NewCodeBaseAddress;
u32_t BackupBaseAddress;
u32_t CodeLen;

const unsigned int UstrCrcTable[16] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF
};

int8_t Decrypt_Packet(uint8_t *pu8Packet, uint32_t u32Offset, uint32_t u32PacketLen);
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
#if 0
unsigned short BlCalFlashCrc16(unsigned int address,unsigned int len) 
{    
    unsigned short crc;
    unsigned char temp;
    unsigned char data;

    crc = 0;
    while (len--) {
        data = FlashReadByte(address);

        temp = ((unsigned char)(crc >> 8)) >> 4;
        crc <<= 4;
        crc ^= UstrCrcTable[temp^(data >> 4)];

        temp = ((unsigned char)(crc >> 8)) >> 4;
        crc <<= 4;
        crc ^= UstrCrcTable[temp^(data & 0x0f)];

        address++;
    }

    return crc;
}
#else
unsigned short BlCalFlashCrc16(unsigned int address,unsigned int len) 
{    
    unsigned short crc;
    unsigned short readlen;
    unsigned char temp;
    unsigned char data;
    char pbuf[1024];
    crc = 0;

    do{
        if( len > 1024 ){
            readlen = 1024;
        }else{
            readlen = len;
        }
       
        FlashReadStr(address ,(u8_t*)pbuf ,readlen);

        for(int i=0;i<readlen;i++){
            data = pbuf[i];
  
            temp = ((unsigned char)(crc >> 8)) >> 4;
            crc <<= 4;
            crc ^= UstrCrcTable[temp^(data >> 4)];

            temp = ((unsigned char)(crc >> 8)) >> 4;
            crc <<= 4;
            crc ^= UstrCrcTable[temp^(data & 0x0f)];
        }
        address+=readlen;
        len-=readlen;

    }while(len);

    return crc;
}
#endif
/*
********************************************************************************
**  函数名称:  VersionMsgRecode
**  功能描述:  版本信息转换
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
**  注    意:  驱动决定FLASH_UPDATA_FLAG要在FLASH_USVS_FLAG之前写，
********************************************************************************
*/
void VersionMsgUpdata(void)
{
    u8_t Version = 0;

    Version = FlashReadByte(FLASH_USVS_FLAG);
    bl_log(LG_DBG, "Version flags1 = %d", Version);
    if (Version == 2) {
        FlashWriteByte(FLASH_USVS_FLAG, 1);
        bl_log(LG_INF, "start cur_image 1");
    } else {
        FlashWriteByte(FLASH_USVS_FLAG, 2);
        bl_log(LG_INF, "start cur_image 2");
    }

    FlashWriteByte(FLASH_UPDATA_FLAG, 2);

    Version = FlashReadByte(FLASH_USVS_FLAG);

    bl_log(LG_DBG, "Version flags2 = %d", Version);
}

/*
********************************************************************************
**  函数名称:  CodeUpdataGetBaseAddr
**  功能描述:  获取升级基准地址
**  输入参数:  无
**  输出参数:  无
**  返回参数:  0获取成功，1获取失败，数据异常
********************************************************************************
*/
u8_t CodeUpdataGetBaseAddr(void)
{
    u8_t version = 0;

    version = FlashReadByte(FLASH_USVS_FLAG);
    if (version == 2) {
        NewCodeBaseAddress = FLASH_VS1_ADDR;
        BackupBaseAddress = FLASH_VS2_ADDR;
        bl_log(LG_INF, "Use the version 1");
    } else {
        NewCodeBaseAddress = FLASH_VS2_ADDR;
        BackupBaseAddress = FLASH_VS1_ADDR;
        bl_log(LG_INF,"Use the version 2");
    }

    FlashReadStr(NewCodeBaseAddress + FLASH_CODE_LEN, (u8_t*)&CodeLen, 4);
    bl_log(LG_DBG, "Reference address 0x%4x The length of the code :%8x LEN",
        NewCodeBaseAddress, CodeLen);
    if (CodeLen > USER_FLASH_SIZE) {
        bl_log(LG_ERR, "Illegal code length");
        FlashWriteByte(FLASH_UPDATA_FLAG, 2);

        return 1;
    }

    bl_log(LG_DBG, "Version Status bit = %d ", FlashReadByte(FLASH_USVS_FLAG)) ;
    bl_log(LG_DBG, "Upgrade flag = %d ", FlashReadByte(FLASH_UPDATA_FLAG)) ;

    return 0;
}
/*
********************************************************************************
**  函数名称:  CodeUpdataCrcVef
**  功能描述:  升级代码CRC校验
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无  
********************************************************************************
*/
u8_t CodeUpdataFlieCrcVef(void)
{
    u16_t savecrcval;
    u16_t calcac;

    FlashReadStr(NewCodeBaseAddress + FLASH_CRC_VAL, (u8_t*)&savecrcval, 2);

    calcac = BlCalFlashCrc16(NewCodeBaseAddress + FLASH_CODE_OFFER, CodeLen);
    bl_log(LG_DBG,"The CRC value is stored : %4x The current calculation : %4x",savecrcval,calcac);

    if (calcac == savecrcval) {
        return 0;
    }

    return 1;
}
/*
********************************************************************************
**  函数名称:  CodeUpdataSofeDet
**  功能描述:  监测代码是否升级
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
u8_t CodeUpdataSofeDet(void)
{
    u8_t data;

    data = FlashReadByte(FLASH_UPDATA_FLAG);

    if (data == UPDATA_CHECK) {
        return 0;
    }

    return 1;
}
/*
********************************************************************************
**  函数名称:  Bl_CodeCopyToInterFlash
**  功能描述:  外部代码搬运到内部，升级实际处理
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
int8_t Bl_CodeCopyToInterFlash(void)
{
    u32_t addr = 0;
    u16_t ulen = 1024;
    u32_t cntlen;
    uint32_t flashdestination = APPLICATION_ADDRESS;

    bl_log(LG_DBG, "FLASH is being erased \r\n");
    if (FLASH_If_EraseBlack(APPLICATION_ADDRESS, CodeLen) != 0) {
        bl_log(LG_ERR, "Internal memory erase failed\r\n");
        return -1;
    }

    cntlen = 0;
    addr = NewCodeBaseAddress + FLASH_CODE_OFFER;

    bl_log(LG_DBG, "Wait a moment and write to program memory\r\n");

    kprintf("load image...\r\n");
    for (flashdestination = APPLICATION_ADDRESS; cntlen < CodeLen; ) {
        FlashReadStr(addr, ReadCode, ulen);
        kprintf("#");

        // 解密
        if (Decrypt_Packet(ReadCode, cntlen, ulen) != 0) {
            bl_log(LG_ERR, "Decrypt failed\r\n");
            return -2;
        }

        if (FLASH_If_Write(flashdestination, (uint32_t*)ReadCode, (uint16_t)ulen / 4) != PASSED) {
            bl_log(LG_ERR,"Code writing failed\r\n");
            return -3;
        }
        flashdestination+=ulen;
        addr += ulen;
        cntlen += ulen;
    }
    kprintf("\r\n");

    VersionMsgUpdata();

    return 0;
}

/*
********************************************************************************
**  函数名称:  Bl_CodeCheckDecrypt
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
int8_t Bl_CodeCheckDecrypt(void)
{
    u32_t addr = 0;
    u16_t ulen = 1024;
    u32_t cntlen;

    addr = NewCodeBaseAddress + FLASH_CODE_OFFER;
    
    for (cntlen = 0; cntlen < 1024; ) {
        FlashReadStr(addr, ReadCode, ulen);

        // 解密
        if (Decrypt_Packet(ReadCode, cntlen, ulen) != 0) {
            bl_log(LG_ERR,"Decrypt failed");
            return -2;
        }

        addr += ulen;
        cntlen += ulen;
    }

    return 0;
}

/*
********************************************************************************
**  函数名称:  Bl_CodeUPData_Deal
**  功能描述:  跳转到应用程序
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void Bl_CodeUPData_Deal(void)
{
    //升级条件判断
    if (CodeUpdataSofeDet()) {
        return;
    }

    // 获取升级代码基准地址
    if (CodeUpdataGetBaseAddr()) {
        Bl_Errorflg(UPDATA_LEN_ERR);
        return;
    }

    if (CodeUpdataFlieCrcVef()) {
        Bl_Errorflg(UPDATA_CRC_ERR);
        return;
    }

    if (Bl_CodeCheckDecrypt() != 0) {
        Bl_Errorflg(UPDATA_ABNO_ERR);
        return;
    }

    // 代码从外部拷贝到内部存储器
    if (Bl_CodeCopyToInterFlash() != 0) {
        Bl_Errorflg(UPDATA_ABNO_ERR);
        return;
    }

    Bl_Errorflg(UPDATA_NONE_ERR);
}         

