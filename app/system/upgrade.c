/*
 * updata.c
 *
 *  Created on: 2023年7月17日
 *      Author: lwp
 */
#include "board.h"
#include "app_lib.h"


#define DBG_TAG "ota"
#define DBG_LVL DBG_LOG   
#include <rtdbg.h>

/*
********************************************************************************
********************************************************************************
*/ 

#define UPDATA_PAGE_MAX_LEN  1024


/*
********************************************************************************
********************************************************************************
*/ 


/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : __FlashReadStr
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u8_t __FlashReadStr(u32_t address,u8_t *pdata,u16_t ulen)
{
    W25QXXRead( address, pdata, ulen);

    return 0;
}
/*
********************************************************************************
*Function    : __FlashReadByte
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u8_t __FlashReadByte(u32_t address)
{
    u8_t data;
    
    W25QXXRead( address, &data, 1);
 
    return data;
} 
/*
********************************************************************************
*Function    : __FlashWriteStr
*Description :
*Input       :
*Output      : 0X091800
*Return      : 0X200000
*Others      :
********************************************************************************
*/
s8_t __FlashWriteStr(u32_t address,u8_t *pdata,u16_t ulen)
{
    u32_t sectornum;
    u8_t  *pbuf;
    s8_t  res;

    if(( address & 0xFFF )==0){
        sectornum = address/4096;
        W25QXXEraseSector(sectornum);
        LOG_D("address = %08X:sectornum=[%d]",address,sectornum);
    }

    pbuf = rt_malloc(ulen);
    if( pbuf == NULL ){
        LOG_E("%s rt_malloc len:[%d]  ",__func__,ulen);
        return -2;
    }
    LOG_D("address = %08X:ulen=[%d]",address,ulen);
    W25QXXWrite( address, pdata, ulen);
   
    res = W25QXXRead( address, pbuf, ulen);
    if(res == -1 ){
        
    }
    res = memcmp(pdata,pbuf,ulen);
    if( res != 0) {
      LOG_E("%s address=[%08X]",__func__,address);
    }

    rt_free(pbuf);
    return 0;
}
/*
********************************************************************************
*Function    : __FlashWriteByte
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u8_t __FlashWriteByte(u32_t address,u8_t data)
{
    u32_t sectornum;
    
    if(( address & 0xFFF )==0){
        sectornum = address/4096;
        W25QXXEraseSector(sectornum);
    }
 
    W25QXXWrite( address, (u8_t*)&data, 1);

    return 0;
}
/*
********************************************************************************
*Function    : file_size
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int file_size(const char *file)
{
    int  fd;
    int ulen; 

    fd = open( file , O_RDONLY);
    if( fd == -1 ){ 
        LOG_E("Unable to find file %s ",file);
        return 0;
    }
    lseek(fd, 0, SEEK_CUR);
    ulen = lseek(fd, 0, SEEK_END);
    
    close(fd);
    
    return ulen;
}
/*
********************************************************************************
*Function    : CalFlashCrc16
*Description : 从指定地址的FLASH开始计算CRC值，长度为len
*Input       : address : 地址
**             len     : 长度
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
unsigned short CalFlashCrc16(unsigned int address,unsigned int len) 
{    
    const unsigned int CrcTable[16] =
    {    
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
    0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF
    };
    unsigned short crc;
    unsigned short readlen;
    unsigned char temp;
    unsigned char data;
    char *pbuf;
    crc = 0;

    pbuf = rt_malloc(1024);
    if( pbuf == NULL ){
        LOG_E("%s rt_malloc  ",__func__);
        return 0;
    }
    do{

        if( len > 1024 ){
            readlen = 1024;
        }else{
            readlen = len;
        }
       
        __FlashReadStr(address ,(u8_t*)pbuf ,readlen);

        for(int i=0;i<readlen;i++){
            data = pbuf[i];
  
            temp = ((unsigned char)(crc >> 8)) >> 4;
            crc <<= 4;
            crc ^= CrcTable[temp^(data >> 4)];

            temp = ((unsigned char)(crc >> 8)) >> 4;
            crc <<= 4;
            crc ^= CrcTable[temp^(data & 0x0f)];
        }
        address+=readlen;
        len-=readlen;

    }while(len);

    rt_free(pbuf);

    return crc;
}
/*
********************************************************************************
*Function    : CalFileBinCrc16
*Description : 计算文件CRC16
*Input       : address : 地址
**             len     : 长度
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
unsigned short CalFileBinCrc16(int fd,unsigned int address,unsigned int len) 
{   
    const unsigned int CrcTable[16] =
    {    
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
    0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF
    };
    unsigned short crc;
    unsigned char temp;
    unsigned char data;
    s8_t  res;
    
    crc = 0;
	lseek(fd, 0, SEEK_SET);
    
 
    while(1) {
        res = read(fd,  (char*)&data, 1);
        if (res <= 0 || address >=len ){
            break;
        }
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
*Function    : file_size_and_crc
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :O_WRONLY
********************************************************************************
*/
int file_size_and_crc(const char *filename, unsigned int *p_size, unsigned short *p_crc)
{
    const unsigned int CrcTable[16] = {
        0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
        0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF
    };

    struct stat f_info;
    int fd;
    int len, i;
    unsigned short crc;
    unsigned char temp, data;
    char buf[128];

    if (filename == RT_NULL) {
        return -1;
    }

    fd = open(filename , O_RDONLY);
    if( fd < 0 ) {
        return -1;
    }

    if (fstat(fd, &f_info) < 0 || f_info.st_size <= 0){
        close(fd);
        return -1;
    }

    crc = 0;
    while (1) {
        len = read(fd,  buf, sizeof(buf));
        if (len <= 0){
            break;
        }
        i = 0;
        while (i < len) {
            data = buf[i++];

            temp = ((unsigned char)(crc >> 8)) >> 4;
            crc <<= 4;
            crc ^= CrcTable[temp^(data >> 4)];

            temp = ((unsigned char)(crc >> 8)) >> 4;
            crc <<= 4;
            crc ^= CrcTable[temp^(data & 0x0f)];
        }
    }

    close(fd);

    *p_size = f_info.st_size;
    *p_crc = crc;

    return 0;
}
/*
********************************************************************************
*Function    : upgrade_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void upgrade_init(void)
{
    u8_t  Version;

    if(__FlashReadByte(FLASH_UPDATA_FLAG)!=0){//监测是否需要升级上报
        __FlashWriteByte(FLASH_UPDATA_FLAG,0);//清除升级标志
        app_set_otastat(__FlashReadByte(FLASH_ERROR_FLG));
    }
    Version = __FlashReadByte(FLASH_USVS_FLAG);
    if(Version==2){
         LOG_D("The VS1 area can be used for upgrades Version 0x%x",Version);
    }else{
         LOG_D("The VS2 area can be used for upgrades Version 0x%x",Version);
    } 
}
/*
********************************************************************************
*Function    : monitor_upgrade_flag
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u8_t *monitor_upgrade_flag(void)
{
    return NULL;//upgrade_error_flag(Updata_Err_Flg);
}
/*
********************************************************************************
*Function    : upgrade
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
s8_t upgrade_app(char *pfile)
{
    int   fd;
    char *pbuf;
	struct stat buf;
    rt_err_t res;
    u32_t fp_len=0;
    u32_t address;
    u32_t flash_base_addr;
    u8_t  Version;
    u16_t flash_crcval;
    u16_t file_crcval;
    u16_t progress;
    u16_t progressnum;
    u16_t progresscnt;

    
    if(pfile==NULL){
        pfile= "OTAV1002";
    }
    /*文件相关操作*/
    fd = open(pfile , O_RDONLY);
    if( fd < 0 ){ 
        LOG_E("open file %s ",pfile);
        return -1;
    }
	
    if (fstat(fd, &buf) < 0){
        close(fd);
        LOG_E("fstat file %s",pfile);
        return -1;
    }else{
        if (buf.st_size < 20){
            close(fd);
            LOG_E("file %s file size %d",pfile,buf.st_size);
            return -2;
        }
    }
    /*版本信息*/
    Version = __FlashReadByte(FLASH_USVS_FLAG);
    if(Version==2){
        flash_base_addr=FLASH_VS1_ADDR;
        LOG_D("The VS1 0X%80X area can be used for upgrades",flash_base_addr);
    }else{
        flash_base_addr=FLASH_VS2_ADDR;
        LOG_D("The VS2 0X%80X area can be used for upgrades",flash_base_addr);
    } 
    /*数据复制*/
    address = flash_base_addr+FLASH_CODE_OFFER;

    pbuf = rt_malloc(UPDATA_PAGE_MAX_LEN);
    if( pbuf == NULL ){
        LOG_E("%s rt_malloc  ",__func__);
        return -3;
    }

    fp_len=0;
    progresscnt=0;
    progressnum = buf.st_size/1024;
	lseek(fd, 0, SEEK_CUR);
 
    while(1){
      
        res = read(fd,  pbuf, UPDATA_PAGE_MAX_LEN);
        if (res <= 0){
            break;
        }
        __FlashWriteStr(address, (u8_t *)pbuf, res); 

        progresscnt++;
        
        fp_len+= res;
        
        address += res;
        progress = progresscnt*100/progressnum;
   
        LOG_D("address 0x%x res %d progress %d",address,res,progress);
    } 
    /*CRC 数据信息*/
    address = flash_base_addr+FLASH_CODE_OFFER;
    
    flash_crcval = CalFlashCrc16(address,fp_len);
    
    file_crcval = CalFileBinCrc16(fd,0,fp_len);
    
    close(fd);
    
    LOG_D("flash_base_addr = 0x%08X ",flash_base_addr);
    LOG_D("flash crcval is 0x%x file_crcval is 0x%x  fp_len  0x%x",flash_crcval,file_crcval,fp_len);
    if(flash_crcval != file_crcval ){
        LOG_E("flash crcval is 0x%x file_crcval is 0x%x ",flash_crcval,file_crcval);
        return -1;
    }else{
        __FlashWriteStr(flash_base_addr+FLASH_CRC_VAL, (u8_t*)&flash_crcval, 2); 
    }
    
    rt_free(pbuf);
    
    __FlashWriteStr(flash_base_addr+FLASH_CODE_LEN, (u8_t*)&fp_len, 4);
 
    __FlashWriteByte(FLASH_UPDATA_FLAG,1);
 
    LOG_D("The VS1 area can be used for upgrades %d",__FlashReadByte(FLASH_USVS_FLAG));
     
    syswatch_reboot(); 
    return 0;
}
/*
********************************************************************************
*Function    : ota_version_rollback
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
rt_err_t ota_version_rollback(void)
{
    __FlashWriteByte(FLASH_UPDATA_FLAG,1);

    rt_thread_mdelay(1000);
    
    syswatch_reboot(); 

    return RT_EOK;
}
/*
********************************************************************************
Function      : upgrade
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void upgrade(int argc, char** argv)
{
    if (argc < 2 ) {
        rt_kprintf("Please input '<start|rollback >' \n"); 
        return;
    }
    if(strcmp(argv[1], "start") == 0) {
         upgrade_app(argv[2]);
    }else if(strcmp(argv[1], "rollback") == 0) {  
         ota_version_rollback();     
    }else{  
        rt_kprintf("Please input '<start|rollback>' \n"); 
    }
}
MSH_CMD_EXPORT(upgrade,upgrade <start|rollback> );


