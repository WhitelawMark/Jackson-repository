/*
 * lan_set.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include "app_lib.h"


#define DBG_TAG "lan"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>       

/*
********************************************************************************
********************************************************************************
*/
#define DEF_LANCMD_FN(fn)  static int fn(u8_t *msg, u32_t len, lan_lowlayer *from, void *arg)
/*
********************************************************************************
********************************************************************************
*/
/* 
 * 1、激活设备 
 */
DEF_LANCMD_FN(LanSetCmd01)    
{
    LOG_D("activating device");
    

    return lan_send( BLUE_SET|BLUE_ACK, 0x01, 0, 0,from);
}
/* 
 * 2、飞行模式 
 */
DEF_LANCMD_FN(LanSetCmd02)   
{
    LOG_D("airplane mode %d",msg[0]);
   
    PPItemWrite(PP_AIRPLANE_MODE, (u8_t*)&msg,  PPItemSize(PP_AIRPLANE_MODE));   
       
    return lan_send( BLUE_SET|BLUE_ACK, 0x02, 0, 0,from);
}
/* 
 * 3、GPS设置
 */
DEF_LANCMD_FN(LanSetCmd03)    
{
    PPItemWrite(PP_GPS_MODE, (u8_t*)&msg,  PPItemSize(PP_GPS_MODE));   
  
    LOG_D("GPS Settings  mode %d",msg[0]);
    return lan_send( BLUE_SET|BLUE_ACK, 0x03, 0, 0,from);
}
/* 
 * 4、陀螺仪设置
 */
DEF_LANCMD_FN(LanSetCmd04)    
{    
    PPItemWrite(PP_GYR_MODE, (u8_t*)&msg,  PPItemSize(PP_GYR_MODE));   
  
    LOG_D("Gyroscope setup  mode %d",msg[0]);
    return lan_send( BLUE_SET|BLUE_ACK, 0x04, 0, 0,from);
}
/* 
 * 5、时间设置
 */
DEF_LANCMD_FN(LanSetCmd05)    
{
    rt_uint32_t year,  month,  day;
    rt_uint32_t hour,  minute,  second;
    
    
    LOG_D("timeset");
    
    year  = 1900+msg[0];
    month = msg[1]+1;  
    day   = msg[2];
    
    hour   = msg[3];
    minute = msg[4];
    second = msg[5];
    
    
    set_date(year, month, day);
    set_time(hour, minute, second);
      
    return lan_send( BLUE_SET|BLUE_ACK, 0x05, 0, 0,from);
}
/* 
 * 6、MQTT URL设置 
 */
DEF_LANCMD_FN(LanSetCmdF0)  
{
    LOG_D("MQTT URL Settings url[%s]",msg);
    
    PPItemWrite(PP_MQT_URL, (u8_t*)msg,  PPItemSize(PP_MQT_URL));   
 
    return lan_send( BLUE_SET|BLUE_ACK, 0xF0, 0, 0,from);
}
/* 
 * 7、MQTT PORT 设置
 */
DEF_LANCMD_FN(LanSetCmdF1)    
{
    LOG_D("MQTT PORT[%d] Settings",oam_get_le_word(msg));
    
    PPItemWrite(PP_MQT_PORT, (u8_t*)msg,  PPItemSize(PP_MQT_PORT));   
 
    return lan_send( BLUE_SET|BLUE_ACK, 0xF1, 0, 0,from);
}
/* 
 * 8、恢复出厂化
 */
DEF_LANCMD_FN(LanSetCmdE0)   
{
    PPItemsFactory();
    LOG_D("factory reset");
    return lan_send( BLUE_SET|BLUE_ACK, 0xE0, 0, 0,from);
}
/* 
 * 9、设备重启
 */
DEF_LANCMD_FN(LanSetCmdE1)   
{
    LOG_D("restart");
    syswatch_reboot();
    return lan_send( BLUE_SET|BLUE_ACK, 0xE1, 0, 0,from);
}
/* 
 * 10、切换卡号 
 */
DEF_LANCMD_FN(LanSetCmdE2)   
{
    LOG_D("Switch Esim card num");
    return lan_send( BLUE_SET|BLUE_ACK, 0xE2, 0, 0,from);
}
/* 
 * 11、设备关机 
 */
DEF_LANCMD_FN(LanSetCmdE3)   
{
    LOG_D("shutdown");
    return lan_send( BLUE_SET|BLUE_ACK, 0xE3, 0, 0,from);
}
static const struct lanset_cmd {
    char cmdid;
    int (*docmd)(u8_t *msg, u32_t len, lan_lowlayer *from, void *arg);
} lan_set_cmd_table[] = {
    {0x01, LanSetCmd01},    /* 1、激活设备 */
    {0x02, LanSetCmd02},    /* 2、飞行模式 */
    {0x03, LanSetCmd03},    /* 3、GPS设置*/
    {0x04, LanSetCmd04},    /* 4、陀螺仪设置*/
    {0x05, LanSetCmd05},    /* 5、时间设置*/
    {0xF0, LanSetCmdF0},    /* 6、MQTT URL设置 */
    {0xF1, LanSetCmdF1},    /* 7、MQTT PORT 设置*/
    {0xE0, LanSetCmdE0},    /* 8、恢复出厂化*/
    {0xE1, LanSetCmdE1},    /* 9、设备重启*/
    {0xE2, LanSetCmdE2},    /* 10、切换卡号 */
    {0xE3, LanSetCmdE3},     /* 11、设备关机 */
    {0,0}
};
/*
********************************************************************************
*Function    : lan_set
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void lan_set(u8_t *msg, u32_t len, lan_lowlayer *from, void *arg)
{    
    const struct lanset_cmd *cmd;
    int table_size, i;
    u8_t cmdid;
    
    cmdid = msg[0];  
    
    table_size = sizeof(lan_set_cmd_table)/sizeof(lan_set_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        if(cmdid == lan_set_cmd_table[i].cmdid ) {
            break;
        }
    }
    if (i == table_size) {
        LOG_E("unknown message cmdid %d", cmdid);
        return;
    }
    
    cmd = &lan_set_cmd_table[i];
    
    if (cmd->docmd) {
        cmd->docmd(msg+1, len-1,from,arg);
    }
}