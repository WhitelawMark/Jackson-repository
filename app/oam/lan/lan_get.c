/*
 * lan_get.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include "app_lib.h"
#include <time.h>

#define DBG_TAG "lan"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>       


/*
********************************************************************************
********************************************************************************
*/
#define DEF_GETCMD_FN(fn)  static int fn(u8_t *msg, u32_t len, lan_lowlayer *from, void *arg)
/*
********************************************************************************
********************************************************************************
*/
/* 1、查询ICCID0 */
DEF_GETCMD_FN(LanGetCmd01)     
{   
    u8_t tmpbuf[64];
    
    PPItemRead(PP_ESIM_CCID0, tmpbuf,  PPItemSize(PP_ESIM_CCID0));
    
    LOG_D("Query ICCID0");
    return lan_send( BLUE_GET|BLUE_ACK, 0x01, tmpbuf, 64,from);
}
 /* 2、查询ICCID1 */
DEF_GETCMD_FN(LanGetCmd02)    
{    
    u8_t tmpbuf[64];
    
    PPItemRead(PP_ESIM_CCID1, tmpbuf,  PPItemSize(PP_ESIM_CCID1));
    
    LOG_D("Query ICCID1");
    return lan_send( BLUE_GET|BLUE_ACK, 0x02, tmpbuf, 64,from);
}
/* 3、查询ICCID2 */
DEF_GETCMD_FN(LanGetCmd03)     
{    
    u8_t tmpbuf[64];
    
    PPItemRead(PP_ESIM_CCID2, tmpbuf,  PPItemSize(PP_ESIM_CCID2));
    
    LOG_D("Query ICCID2");
    return lan_send( BLUE_GET|BLUE_ACK, 0x03, tmpbuf, 64,from);
}
/* 4、查询IMEI   */
DEF_GETCMD_FN(LanGetCmd04)     
{
    u8_t tmpbuf[64];
    
    strcpy((char*)tmpbuf, at_get_imei()); 
    
    LOG_D("Query IMEI");
    return lan_send( BLUE_GET|BLUE_ACK, 0x04, tmpbuf,64,from);
}
 /* 5、查询RSSI   */
DEF_GETCMD_FN(LanGetCmd05)    
{
    u8_t rssi;
    rssi=at_get_rssi();
    LOG_D("Query RSSI");
    return lan_send( BLUE_GET|BLUE_ACK, 0x05, &rssi, 1,from);
}
/* 6、查询序列号 */
DEF_GETCMD_FN(LanGetCmd06)     
{    
    u8_t tmpbuf[64];
    
    PPItemRead(PP_PRD_PSN, tmpbuf,  PPItemSize(PP_PRD_PSN));
    
    LOG_D("Query serial number");
    return lan_send( BLUE_GET|BLUE_ACK, 0x06, tmpbuf, 64,from);
}
/* 7、版本信息查询*/
DEF_GETCMD_FN(LanGetCmd07)     
{    
    u8_t tmpbuf[8];
    int boardvs;
    int softvs;
    
    
    boardvs = atoi(SYS_BOARD_VERSION); 
    softvs = atoi(SYS_SOFE_VERSION); 
    LOG_D("Version information query");
    oam_set_le_dword(tmpbuf,boardvs);
    oam_set_le_dword(tmpbuf+4,softvs);
      
    
    return lan_send( BLUE_GET|BLUE_ACK, 0x07, tmpbuf, 8,from);
}

/* 8、时间查询 */
DEF_GETCMD_FN(LanGetCmd08)     
{
    time_t timep;
    struct tm *p;
    u8_t tmpbuf[8];
    
    LOG_D("Time query");
    
    time(&timep);
    p = localtime(&timep);
    
    tmpbuf[0] =  p->tm_year;
    tmpbuf[1] =  p->tm_mon+1;
    tmpbuf[2] =  p->tm_mday;
    tmpbuf[3] =  p->tm_hour;
    tmpbuf[4] =  p->tm_min;
    tmpbuf[5] =  p->tm_sec;

    return lan_send( BLUE_GET|BLUE_ACK, 0x08, tmpbuf, 6,from);
}
/* 9、GPS信息查询 */
DEF_GETCMD_FN(LanGetCmd11)     
{
    u8_t tmpbuf[16];
    u32_t longx,lati;
    
    longx = 1;
    lati = 2;
    
    oam_set_le_dword(tmpbuf,longx);
    oam_set_le_dword(tmpbuf+4,lati);
    LOG_D("GPS information query");
    return lan_send( BLUE_GET|BLUE_ACK, 0x11, tmpbuf, 8,from);
}
 /* 10、六轴查询 */
DEF_GETCMD_FN(LanGetCmd12)    
{
    u8_t tmpbuf[24];
    u32_t posx,posy,posz;
    u32_t posh,posv,poss;
    
    posx = 1;
    posy = 2;
    posz = 3;
    posh = 4;
    posv = 5;
    poss = 6;
 
    oam_set_le_dword(tmpbuf,posx);
    oam_set_le_dword(tmpbuf+4,posy);
    oam_set_le_dword(tmpbuf+8,posz);
    
    oam_set_le_dword(tmpbuf+12,posh);
    oam_set_le_dword(tmpbuf+16,posv);
    oam_set_le_dword(tmpbuf+20,poss);
    
    LOG_D("Query Gyroscope");
    return lan_send( BLUE_GET|BLUE_ACK, 0x12, tmpbuf, 24,from);
}


static const struct langet_cmd {
    char cmdid;
    int (*docmd)(u8_t *msg, u32_t len, lan_lowlayer *from, void *arg);
} lan_get_cmd_table[] = {
    {0x01, LanGetCmd01},     /* 1、查询ICCID0 */
    {0x02, LanGetCmd02},     /* 2、查询ICCID1 */
    {0x03, LanGetCmd03},     /* 3、查询ICCID2 */
    {0x04, LanGetCmd04},     /* 4、查询IMEI   */
    {0x05, LanGetCmd05},     /* 5、查询RSSI   */
    {0x06, LanGetCmd06},     /* 6、查询序列号 */
    {0x07, LanGetCmd07},     /* 7、版本信息查询*/
    {0x08, LanGetCmd08},     /* 8、时间查询 */
    {0x11, LanGetCmd11},     /* 9、GPS信息查询 */
    {0x12, LanGetCmd12},     /* 10、六轴查询 */
    {0,0}
};
/*
********************************************************************************
*Function    : lan_get
*Description : 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void lan_get(u8_t *msg, u32_t len, lan_lowlayer *from, void *arg)
{
    const struct langet_cmd *cmd;
    int table_size, i;
    u8_t cmdid;
    
    cmdid = msg[0];  
    
    table_size = sizeof(lan_get_cmd_table)/sizeof(lan_get_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        if(cmdid == lan_get_cmd_table[i].cmdid ) {
            break;
        }
    }
    if (i == table_size) {
        LOG_E("unknown message cmdid %d", cmdid);
        return;
    }
    
    cmd = &lan_get_cmd_table[i];
    
    if (cmd->docmd) {
        cmd->docmd(msg+1, len-1,from,arg);
    }
}