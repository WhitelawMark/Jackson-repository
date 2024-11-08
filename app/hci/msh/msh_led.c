
/*
 * lan_port.c
 *
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
 
#include "app_lib.h"
/*
********************************************************************************
********************************************************************************
"连接互联网  	    BLUE  慢闪，Nx100ms亮，Nx100ms灭，直到设备连网络或者持续Xmin后关闭
已连接互联网	    BLUE  长亮Ns，然后关闭
电池电量过低	    RED   长亮Ns，然后关闭
设备开机/power-on	GREEN 长亮Ns，然后关闭
充电正在进行	    GREEN 慢闪，Nx100ms亮，Nx100ms灭，一直持续直到设备充满
设备充满电	        GREEN 长亮Ns，然后关闭
设备关机过程	    GREEN 快闪，Nx10ms亮，Nx10ms灭，持续Xs后关闭
适合狗	            RED   熄灭
不适合狗或不适合狗	RED   快闪，Nx10ms亮，Nx10ms灭(直到电池没电或者用户APP点击确认)
*/

//static int sos_send_times = 0;
#define DEF_LEDCMD_FN(fn)  static int fn(int argc, char** argv)
/*
********************************************************************************
********************************************************************************
*/
#define QLED_BLUE_PIN      GET_PIN(B, 7) 

static int sos_send_times = 0;

static const u16_t connect_datas[] = 
{
    100, 100, 100, 100, 100, 100,100, 100, 100, 100,
};
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    :  qled_sos_cb
*Description :  
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
static void qled_sos_cb(void)
{  
    sos_send_times--;
    
    time_t now;
    
    rt_kprintf("sos_send_times =%d %s \n",sos_send_times, ctime(&now));
    if (sos_send_times > 0)
    {
        qled_set_special(QLED_BLUE_PIN, connect_datas, sizeof(connect_datas)/sizeof(u16), qled_sos_cb);
    }
    else
    {
        qled_remove(QLED_BLUE_PIN);
    }
}
static void qled_cb_off(void)
{  
    qled_remove(QLED_BLUE_PIN);
}
/*
********************************************************************************
*Function    :  LEDCmd0001
*Description :  连接互联网  
*Input       :
*Output      :
*Return      :
*Others      :BLUE  慢闪，Nx100ms亮，Nx100ms灭，直到设备连网络或者持续Xmin后关闭
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0001)
{
    qled_add(QLED_BLUE_PIN, 1);
    sos_send_times=15;
    qled_set_special(QLED_BLUE_PIN, connect_datas, sizeof(connect_datas)/sizeof(u16), qled_sos_cb);
    return 0;
}
/*
********************************************************************************
*Function    :  LEDCmd0002
*Description :  已连接互联网
*Input       :
*Output      :
*Return      :
*Others      :  BLUE  长亮Ns，然后关闭
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0002)
{
static const u16_t connected_datas[] = 
{
        5000, 100
};
  
    qled_add(QLED_BLUE_PIN, 1);
    sos_send_times=1;
    qled_set_special(QLED_BLUE_PIN, connected_datas, sizeof(connected_datas)/sizeof(u16), qled_cb_off);
    
    return 0;
}
/*
********************************************************************************
*Function    :  LEDCmd0003
*Description :  电池电量过低
*Input       :
*Output      :
*Return      :
*Others      : RED   长亮Ns，然后关闭
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0003)
{

    return 0;
}
/*
********************************************************************************
*Function    :  LEDCmd0004
*Description :  设备开机/power-on
*Input       :
*Output      :
*Return      :
*Others      : GREEN 长亮Ns，然后关闭
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0004)
{

    return 0;
}
/*
********************************************************************************
*Function    :  LEDCmd0005
*Description :  充电正在进行
*Input       :
*Output      :
*Return      :
*Others      : GREEN 慢闪，Nx100ms亮，Nx100ms灭，一直持续直到设备充满
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0005)
{

    return 0;
}
/*
********************************************************************************
*Function    :  LEDCmd0006
*Description :  设备充满电
*Input       :
*Output      :
*Return      :
*Others      :   GREEN 长亮Ns，然后关闭
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0006)
{

    return 0;
}
/*
********************************************************************************
*Function    :  LEDCmd0007
*Description :  设备关机过程
*Input       :
*Output      :
*Return      :
*Others      :  GREEN 快闪，Nx10ms亮，Nx10ms灭，持续Xs后关闭
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0007)
{

    return 0;
}
/*
********************************************************************************
*Function    :  LEDCmd0008
*Description :  适合狗
*Input       :
*Output      :
*Return      :
*Others      :  RED   熄灭
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0008)
{

    return 0;
}
/*
********************************************************************************
*Function    :  LEDCmd0009
*Description :  不适合狗或不适合狗
*Input       :
*Output      :
*Return      :
*Others      :  RED   快闪，Nx10ms亮，Nx10ms灭(直到电池没电或者用户APP点击确认)
********************************************************************************
*/
DEF_LEDCMD_FN(LEDCmd0009)
{

    return 0;
}
/*
********************************************************************************
********************************************************************************
*/
static const struct led_list_cmd {
    char *cmdid;
    char *describe;
    int (*docmd)(int argc, char** argv);
} led_cmd_table[] = {
    {"0001","$0001", LEDCmd0001},    /*  */
    {"0002","$0001", LEDCmd0002},    /*  */
    {"0003","$0001", LEDCmd0003},    /*  */
    {"0004","$0001", LEDCmd0004},    /*  */
    {"$0005","$0001", LEDCmd0005},    /*  */
    {"$0006","$0001", LEDCmd0006},    /*  */
    {"$0007","$0001", LEDCmd0007},    /*  */
    {"$0008","$0001", LEDCmd0008},    /*  */
    {"$0009","$0001", LEDCmd0009},    /*  */
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
   
    table_size = sizeof(led_cmd_table)/sizeof(led_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        rt_kprintf("[%s]:%s\n",led_cmd_table[i].cmdid,led_cmd_table[i].describe); 
    }
}
/*
********************************************************************************
*Function    : js
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void led(int argc, char** argv)
{
    const struct led_list_cmd *cmd;
    int table_size, i;
    
    if (argc < 2 ) {
        show_usage();
        return;
    }
    table_size = sizeof(led_cmd_table)/sizeof(led_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        if(strcmp(argv[1], led_cmd_table[i].cmdid) == 0) {
            break;
        }
    }
    if (i == table_size) {
        show_usage();
        return;
    }
    cmd = &led_cmd_table[i];
    
    if (cmd->docmd) {
        cmd->docmd(argc, argv);
    }
} 
MSH_CMD_EXPORT(led,led <help|all|...> );