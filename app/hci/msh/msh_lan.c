/*
 *  show.c
 *
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
#include "app_lib.h"

#define DBG_TAG "lan"
#define DBG_LVL DBG_LOG   
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
#define DEF_BTCMD_FN(fn)  static int fn(int argc, char** argv)
/*
********************************************************************************
********************************************************************************
*/
s32_t lan_package(u8_t *buff,u8_t buflen,u8_t flg,u8_t cmdid,u8_t *msg,u8_t msglen)
{
    u16_t packid=0;
    
    buff[0]=0xA5;
    buff[1]=msglen+4;
   
    buff[2]=(u8_t)packid; 
    buff[3]=(u8_t)(packid >> 8); 
    
    buff[4]=flg;  
    buff[5]=cmdid;  
    
    memcpy(buff+6,msg,msglen);
    
    buff[6+msglen]=check_sum(buff+1,msglen+5);  
    buff[7+msglen]=0x5A;  
    
    LOG_HEX("blue_send", 16, buff, msglen+8);

    return msglen+8;
}  
/*
********************************************************************************
Function      : blue_recv_demo
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void lan_recv_demo(void)
{
    u8_t buf[32];
    u8_t ulen=0;
    lan_msg_t msg;
    
    ulen = lan_package(buf,sizeof(buf),BLUE_SET, 0X01,0, 0);
   

    for(int i=0;i<ulen;i++){
    
       lan_msg_recv( &msg, buf[i]);
    }

}
/*
 * 激活设备
 */
DEF_BTCMD_FN(LanSetCmd01)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0X01,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);

    return 0;
}

/*
 * 飞行模式
 */
DEF_BTCMD_FN(LanSetCmd02)
{
    u8_t buf[32];
    u8_t ulen=0;
    u8_t mode =1;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0X02,&mode, 1);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * GPS设置
 */
DEF_BTCMD_FN(LanSetCmd03)
{
    u8_t buf[32];
    u8_t ulen=0;
    u8_t mode =1;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0X03,&mode, 1);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 陀螺仪设置
 */
DEF_BTCMD_FN(LanSetCmd04)
{
    u8_t buf[32];
    u8_t ulen=0;
    u8_t mode =1;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0X04,&mode, 1);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 时间设置
 */
DEF_BTCMD_FN(LanSetCmd05)
{
    u8_t buf[32];
    u8_t time[6];
    u8_t ulen=0;
    
    time[0]=2023-1900;
    time[1]=7;
    time[2]=9;
    
    time[3]=15;
    time[4]=10;
    time[5]=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0X05,time, 6);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}

/*
 * MQTT URL设置
 */
DEF_BTCMD_FN(LanSetCmdF0)
{
    u8_t buf[128];
    u8_t ulen=0;
    u8_t url[64]="192.168.1.1";
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0XF0,url, 64);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * MQTT PORT 设置
 */
DEF_BTCMD_FN(LanSetCmdF1)
{
    u8_t buf[32];
    u8_t ulen=0;
    u16_t port=1883;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0XF1,(u8_t*)&port, 2);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 恢复出厂化
 */
DEF_BTCMD_FN(LanSetCmdE0)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0XE0,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 设备重启
 */
DEF_BTCMD_FN(LanSetCmdE1)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0XE1,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 切换卡号
 */
DEF_BTCMD_FN(LanSetCmdE2)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0XE2,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 设备关机
 */
DEF_BTCMD_FN(LanSetCmdE3)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_SET, 0XE3,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 查询ICCID0
 */
DEF_BTCMD_FN(LanGetCmd01)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X01,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 查询ICCID1
 */
DEF_BTCMD_FN(LanGetCmd02)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X02,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 查询ICCID2
 */
DEF_BTCMD_FN(LanGetCmd03)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X03,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 查询IMEI
 */
DEF_BTCMD_FN(LanGetCmd04)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X04,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 查询RSSI
 */
DEF_BTCMD_FN(LanGetCmd05)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X05,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 查询序列号
 */
DEF_BTCMD_FN(LanGetCmd06)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X06,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 版本信息查询
 */
DEF_BTCMD_FN(LanGetCmd07)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X07,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 时间查询
 */
DEF_BTCMD_FN(LanGetCmd08)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X08,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * GPS信息查询
 */
DEF_BTCMD_FN(LanGetCmd11)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X11,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
 * 六轴查询
 */
DEF_BTCMD_FN(LanGetCmd12)
{
    u8_t buf[32];
    u8_t ulen=0;
    
    ulen =lan_package(buf,sizeof(buf),BLUE_GET, 0X12,0, 0);
   
    lan_recv_frame(0,(char*)buf,ulen);
    return 0;
}
/*
********************************************************************************
********************************************************************************
*/
static const struct list_cmd {
    char *cmdid;
    char *describe;
    int (*docmd)(int argc, char** argv);
} ble_cmd_table[] = {
    {"S01","activating device",         LanSetCmd01},    /* 1、激活设备 */
    {"S02","airplane mode",             LanSetCmd02},    /* 2、飞行模式 */
    {"S03","GPS Settings",              LanSetCmd03},    /* 3、GPS设置*/
    {"S04","Gyroscope setup",           LanSetCmd04},    /* 4、陀螺仪设置*/
    {"S05","timeset",                   LanSetCmd05},    /* 5、时间设置*/
    {"SF0","MQTT URL Settings",         LanSetCmdF0},    /* 6、MQTT URL设置 */
    {"SF1","MQTT PORT Settings",        LanSetCmdF1},    /* 7、MQTT PORT 设置*/
    {"SE0","factory reset",             LanSetCmdE0},    /* 8、恢复出厂化*/
    {"SE1","Device restart",            LanSetCmdE1},    /* 9、设备重启*/
    {"SE2","Switch Esim card num",      LanSetCmdE2},    /* 10、切换卡号 */
    {"SE3","Device shutdown",           LanSetCmdE3},    /* 11、设备关机 */
                                        
    {"G01","Query ICCID0",              LanGetCmd01},    /* 1、查询ICCID0 */
    {"G02","Query ICCID1",              LanGetCmd02},    /* 2、查询ICCID1 */
    {"G03","Query ICCID2",              LanGetCmd03},    /* 3、查询ICCID2 */
    {"G04","Query IMEI",                LanGetCmd04},    /* 4、查询IMEI   */
    {"G05","Query RSSI",                LanGetCmd05},    /* 5、查询RSSI   */
    {"G06","Query serial number",       LanGetCmd06},    /* 6、查询序列号 */
    {"G07","Version information query", LanGetCmd07},    /* 7、版本信息查询*/
    {"G08","Time query",                LanGetCmd08},    /* 8、时间查询 */
    {"G11","GPS information query",     LanGetCmd11},    /* 9、GPS信息查询 */
    {"G12","Query Gyroscope",           LanGetCmd12},    /* 10、六轴查询 */
    {0, 0}
};
/*
********************************************************************************
Function      : show_usage
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static void show_usage(void)
{    
    int table_size, i;
    
    rt_kprintf("Please input  \n"); 
   
    table_size = sizeof(ble_cmd_table)/sizeof(ble_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        rt_kprintf("[%s]:%s\n",ble_cmd_table[i].cmdid,ble_cmd_table[i].describe); 
    }
}
/*
********************************************************************************
*Function    : blue
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void blue(int argc, char** argv)
{
    const struct list_cmd *cmd;
    int table_size, i;
    
    if (argc < 2 ) {
        show_usage();
        return;
    }
    table_size = sizeof(ble_cmd_table)/sizeof(ble_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        if(strcmp(argv[1], ble_cmd_table[i].cmdid) == 0) {
            break;
        }
    }
    if (i == table_size) {
        show_usage();
        return;
    }
    cmd = &ble_cmd_table[i];
    
    if (cmd->docmd) {
        cmd->docmd(argc, argv);
    }
} 
MSH_CMD_EXPORT(blue,blue <help|all|...> );



  