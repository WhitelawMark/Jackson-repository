/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */                                                                    
                                                                           
#include "app_lib.h"                                                    
#include "ota.h"                                                         
  
#define DBG_TAG "ota"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
                                                                         

/*
********************************************************************************
********************************************************************************
*/                                                                        
static struct {
     u32_t runflg;
     u32_t rptstauts;
     u32_t zonebase;
} ota_mng;                                                                       
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
Function      : ota_ls_running
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
int ota_ls_running(void)
{
    return ota_mng.runflg;
}

/*
********************************************************************************
Function      : ota_get_le_dword
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
static u32_t ota_get_le_dword(u8_t *msg)
{
    return (((((u32_t)(msg[3])) << 24) & 0xFF000000) |
            ((((u32_t)(msg[2])) << 16) & 0x00FF0000) |
            ((((u32_t)(msg[1])) << 8 ) & 0x0000FF00) | msg[0]);
}
/*
********************************************************************************
Function      : ota_calc_crc16
Description   : 从指定地址的FLASH开始计算CRC值
Input         :address : 地址                                               
               len     : 长度   
Output        :
Return        : 计算的CRC值
Others        :
********************************************************************************
*/ 

static unsigned short ota_calc_crc16(unsigned int address,unsigned int len) 
{   
    const unsigned int CrcTable[16] =
    {    
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
    0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF
    };
    unsigned short crc;
    unsigned char temp;
    unsigned char data;

    crc = 0;
    
    while(len--) {
        data = __FlashReadByte(address);

        temp = ((unsigned char)(crc >> 8)) >> 4;      
        crc <<= 4;                            
        crc ^= CrcTable[temp^(data >> 4)];            

        temp = ((unsigned char)(crc >> 8)) >> 4;     
        crc <<= 4;                                
        crc ^= CrcTable[temp^(data & 0x0f)]; 

        address++;  
    }
    return crc;
}
 
/*
********************************************************************************
Function      : ota_finit
Description   : 初始化函数
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
static int ota_finit(int evtype)
{
    u8_t version;
    
    
    version = __FlashReadByte(FLASH_USVS_FLAG);
    if(version == 2){
        ota_mng.zonebase=FLASH_VS1_ADDR;
        LOG_D("ota_finit : The VS1 area can be used for update");
    }else {
        if(version != 1){
            __FlashWriteByte(FLASH_USVS_FLAG,1);
        }
        ota_mng.zonebase=FLASH_VS2_ADDR;
        LOG_D("ota_finit :The VS2 area can be used for update");
    }
    return 0;
}    
/*
********************************************************************************
Function      :  ota_fread
Description   :  数据读取函数
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 

static int ota_fread(int address, char *buf, int len)
{
    __FlashReadStr(ota_mng.zonebase+address,(u8_t *)buf,len );  
    LOG_D("[%08x] size:[%03d]",address,len);
    LOG_HEX("ota_fread",16,(rt_uint8_t*)buf,len);
    
    return 0;
}
/*
********************************************************************************
Function      : ota_fwrite
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
static int ota_fwrite(int address, char *buf, int len)
{
    LOG_D("[%08x] size:[%03d]",address,len);
    
    LOG_HEX("ota_fwrite",16,(rt_uint8_t*)buf,len);
    
    __FlashWriteStr(ota_mng.zonebase+address,(u8_t *)buf,len );  
    return 0;
}
/*
********************************************************************************
Function      : ota_operation
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
static int ota_operation(int opttype,int fp_size)
{
#if 1
    u32_t file_size;
    u32_t file_crc16;
    u16_t calcrc;
    u32_t address;
    u8_t headmsg[128];
    u8_t  transtat=1;
   
    address = ota_mng.zonebase+FLASH_CODE_OFFER;
    __FlashReadStr(ota_mng.zonebase,(u8_t *)headmsg,128 );  
  
    file_crc16 = ota_get_le_dword(headmsg+44);
    file_size = ota_get_le_dword(headmsg+48);
    LOG_D("file_crc16 : 0x%04X,file_size : 0x%04X",file_crc16,file_size);
    
    if( fp_size != file_size+512 ){
        LOG_E("file_size:0x%04X,fp_size:0x%04X ",file_size+512,fp_size);
        return 0;
    }
    calcrc = ota_calc_crc16(address+512,file_size);
    if( calcrc != file_crc16 ){
        LOG_E("file_crc16:0x%04X,calcrc:0x%04X ",file_crc16,calcrc);
        return 0;
    }
    
    __FlashWriteStr(ota_mng.zonebase+FLASH_VERS_STA,(u8_t *)&transtat,1 );  
        
    __FlashWriteStr(ota_mng.zonebase+FLASH_CRC_VAL,(u8_t*)&file_crc16,2 );  
        
    __FlashWriteStr(ota_mng.zonebase+FLASH_CODE_LEN,(u8_t*)&file_size,4 );  
   
    __FlashWriteByte(FLASH_UPDATA_FLAG,1);
        
    LOG_D("Firmware update succeeded");
    
    ota_mng.runflg = 0;
    
    rt_hw_cpu_reset();
#else
    
    ota_mng.runflg = 0;
    
    LOG_D("Firmware update succeeded");
#endif
    return 0;
}
/*
********************************************************************************
Function      : ota_start
Description   : 启动固件升级 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
void ota_start(void)
{
    ftp_device_t device;
    char ip[64];
    short port;
    char filename[64];
    char username[64];
    char password[64];
    
    if(ota_mng.runflg){
        LOG_E("ota_start already running");
        return ;
    }

#if 0
    PPItemRead(PP_FTP_URL, ip, PPItemSize(PP_FTP_URL));
    
    PPItemRead(PP_FTP_PORT, &port, PPItemSize(PP_FTP_PORT));

    PPItemRead(PP_FTP_USERNAME, filename, PPItemSize(PP_FTP_USERNAME));
    
    PPItemRead(PP_FTP_PASSWORD, &username, PPItemSize(PP_FTP_PASSWORD));
    
    PPItemRead(PP_FTP_FILENAME, &password, PPItemSize(PP_FTP_FILENAME));
    
    device.finit = ota_finit;
    
    device.fread = ota_fread;
    
    device.fwrite = ota_fwrite;
    
    device.operation = ota_operation;
#else
    
    snprintf(ip,64,"192.168.1.1");
    port =21;
    
    snprintf(filename,64,"XXXXX.OTA");
    
    snprintf(username,64,"000");
    
    snprintf(password,64,"11111");
    
    device.finit = ota_finit;
    
    device.fread = ota_fread;
    
    device.fwrite = ota_fwrite;
    
    device.operation = ota_operation;
#endif

    
    ota_mng.runflg = 1;
    LOG_D("ota start");
    LOG_D("FTP IP %s:%d",ip,port);
    LOG_D("FTP UserName:%s",username);
    LOG_D("FTP password:%s",password);
    LOG_D("FTP filename:%s",filename); 
    
    bg77_ftp_post(MSG_FTP_DOWNLOAD, ip, port,filename,username,password, &device);
}
/*
********************************************************************************
Function      : ota_init
Description   : 启动固件升级 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void ota_init(void)
{
    u8_t Version=0;
    u8_t flg=0;
 
 
    if(__FlashReadByte(FLASH_UPDATA_FLAG)!=0){//监测是否需要升级上报
        __FlashWriteByte(FLASH_UPDATA_FLAG,0);//清除升级标志
        flg = __FlashReadByte(FLASH_ERROR_FLG);
        ota_mng.rptstauts = flg;
    }
    if(__FlashReadByte(FLASH_BOOTUPFLG)!=0xaa){ //引导升级成功
        __FlashWriteByte(FLASH_BOOTUPFLG,0);// 
    }
    Version = __FlashReadByte(FLASH_USVS_FLAG);
    if(Version == 2){
        ota_mng.zonebase=FLASH_VS1_ADDR;
        LOG_D("ota_init:The VS1 area can be used for update");
    }else {
        if(Version != 1){
            __FlashWriteByte(FLASH_USVS_FLAG,1);
        }
        ota_mng.zonebase=FLASH_VS2_ADDR;
        LOG_D("ota_init:The VS2 area can be used for update");
    }

    ota_mng.runflg = 0;
 
    return ;
} 


