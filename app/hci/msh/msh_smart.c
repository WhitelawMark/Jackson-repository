/*
 *  show.c
 *
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
#include "app_lib.h"
#include "smart_port.h"

#define DBG_TAG "misc"
#define DBG_LVL DBG_LOG   
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
#define DEF_JSCMD_FN(fn)  static int fn(int argc, char** argv)
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    :  jsonCmd0001
*Description :  
*Input       :
*Output      :
*Return      :device/set
*Others      :
********************************************************************************
*/
/*
 * 心跳设置
 *
*/
DEF_JSCMD_FN(jsonCmd0001)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/heartbeat/set\",\
                \"tick\":360\
              }\
          }\
    }";
    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 * 飞行模式设置
 */
DEF_JSCMD_FN(jsonCmd0002)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/airplane/set\",\
                \"mode\": \"on\"\
              }\
          }\
    }";
    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 * 工作模式设置
 */
DEF_JSCMD_FN(jsonCmd0003)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/work/mode\",\
                \"mode\":\"auto\",   \
                \"start_time\":\"13-25-00\",\
                \"stop_time\":\"17-25-00\"\
              }\
          }\
    }";
    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 * （4）系统时间
 */
DEF_JSCMD_FN(jsonCmd0004)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/time/set\",\
                \"time\":\"2023-08-3 11-45-00\"\
              }\
          }\
    }";
    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 * （5）LED设置
 */
DEF_JSCMD_FN(jsonCmd0005)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/led/set\",\
                \"switch\":\"on\"\
              }\
          }\
    }";
    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 * （6）GPS设置
 */
DEF_JSCMD_FN(jsonCmd0006)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/gps/set\",\
                \"mode\" :\"/gps/set\",\
                \"cycle\" :\"/gps/set\",\
                \"start_time\" :\"2023-08-10\",\
                \"stop_time\" :\"2023-08-10\",\
                \"period\":360\
              }\
          }\
    }";
    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *（7）陀螺仪设置
 */
DEF_JSCMD_FN(jsonCmd0007)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/heartbeat/set\",\
                \"mode\":\"on\",\
                \"period\":360\
              }\
          }\
    }";
    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *（8）电子围栏
 */
DEF_JSCMD_FN(jsonCmd0008)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/elec/fence\",\
                \"switch\":\"on\",\
                \"distance\":360\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *（9）固件升级
 */
DEF_JSCMD_FN(jsonCmd0009)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/ota/start\",\
                \"url\":\"192.168.1.1\"\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *（10）版本回退
 */
DEF_JSCMD_FN(jsonCmd0010)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/heartbeat/set\",\
                \"tick\":360\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *（11）卡号切换
 */
DEF_JSCMD_FN(jsonCmd0011)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"/card/set\",\
                \"card\":1\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *（12）设备重启
 */
DEF_JSCMD_FN(jsonCmd0012)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"reboot\"\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *（13）恢复出厂化
 */
DEF_JSCMD_FN(jsonCmd0013)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/set\",\
             \"data\":{\
                \"cmd\" :\"factory\"\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
********************************************************************************
********************************************************************************
*/
/*
 *
 */
DEF_JSCMD_FN(jsonCmd1001)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/get\",\
             \"data\":{\
                \"cmd\" :\"/dev/info\"\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *
 */
DEF_JSCMD_FN(jsonCmd1002)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/get\",\
             \"data\":{\
                \"cmd\" :\"/networks\"\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *
 */
DEF_JSCMD_FN(jsonCmd1003)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/get\",\
             \"data\":{\
                \"cmd\" :\"/system/info\"\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *
 */
DEF_JSCMD_FN(jsonCmd1004)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/get\",\
             \"data\":{\
                \"cmd\" :\"/rtinfo\"\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 *
 */
DEF_JSCMD_FN(jsonCmd1005)
{
    char *cmdlist="{\
        \"from\":\"app\",\
        \"to\":0,\
        \"encrypt\":\"none\",\
        \"payload\":{\
             \"seq\":\"0000000002\",  \
             \"action\":\"/dev/get\",\
             \"data\":{\
                \"cmd\" :\"/elec/fence\"\
              }\
          }\
    }";

    return smart_mqtt_recv(cmdlist, strlen(cmdlist));
}
/*
 * 开机上报
 */
DEF_JSCMD_FN(jsonCmd2001)
{
   return smart_report_message(SMART_RPT_POWERON); 
}
/*
 * 心跳上报
 */
DEF_JSCMD_FN(jsonCmd2002)
{
   return smart_report_message(SMART_RPT_HEARTBEAT); 
}
/*
 * OTA主动上报
 */
DEF_JSCMD_FN(jsonCmd2003)
{
   return smart_report_message(SMART_RPT_OTA); 
}
/*
 * 电池低电压上报
 */
DEF_JSCMD_FN(jsonCmd2004)
{
   return smart_report_message(SMART_RPT_LOWBAT); 
}
/*
 * 电子围栏上报
 */
DEF_JSCMD_FN(jsonCmd2005)
{
   return smart_report_message(SMART_RPT_ELECFENCE); 
}
/*
********************************************************************************
********************************************************************************
*/
static const struct js_list_cmd {
    char *cmdid;
    char *describe;
    int (*docmd)(int argc, char** argv);
} js_cmd_table[] = {
    {"$0001","</heartbeat/set>",       jsonCmd0001 },    /* 1、设置产品序列号 */
    {"$0002","</airplane/set>",        jsonCmd0002 },    /* 2、行模式 */
    {"$0003","</work/mode>",           jsonCmd0003 },    /* 3、运行模式*/
    {"$0004","</time/set>",            jsonCmd0004 },    /* 4、时间设置*/
    {"$0005","</led/set>",             jsonCmd0005 },    /* 5、LED设置*/
    {"$0006","</gps/set>",             jsonCmd0006 },    /* 6、GPS设置*/
    {"$0007","</gyroscope/set>",       jsonCmd0007 },    /* 7、六轴设置*/
    {"$0008","</elec/fence>",          jsonCmd0008 },    /* 8、电子围栏*/
    {"$0009","</ota/start>",           jsonCmd0009 },    /* 9、升级开始*/
    {"$0010","</fw/rollback>",         jsonCmd0010 },    /* 10、版本回退 */
    {"$0011","</card/set>",            jsonCmd0011 },    /* 11、卡号切换 */
    {"$0012","<reboot>",               jsonCmd0012 },    /* 12、设备重启*/
    {"$0013","<factory>",              jsonCmd0013 },    /* 13、恢复出厂化 */
     /* 查询命令 */                                
    {"$1001","</dev/info>",            jsonCmd1001 },    /* 1、设备信息 */
    {"$1002","</networks>",            jsonCmd1002 },    /* 2、蜂窝信息 */
    {"$1003","</system/info>",         jsonCmd1003 },    /* 3、系统配置*/
    {"$1004","</rtinfo>",              jsonCmd1004 },    /* 4、实时信息 */
    {"$1005","</elec/fence>",          jsonCmd1005 },    /* 5、电子围栏 */
     /* 主动上报命令 */                            
    {"$2001","</poweron/report>",      jsonCmd2001 },    /* 1、开机上报 */
    {"$2002","</heartbeat/report>",    jsonCmd2002 },    /* 2、心跳上报 */
    {"$2003","</ota/report>",          jsonCmd2003 },    /* 3、 OTA上报 */
    {"$2004","</lowbattery/report>",   jsonCmd2004 },    /* 4、电池低电压上报 */
    {"$2005","</elecfence/report>",    jsonCmd2005 },    /* 5、电子围栏上报 */
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
    
    rt_kprintf("cmd describe  \n"); 
   
    table_size = sizeof(js_cmd_table)/sizeof(js_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        rt_kprintf("[%s]:%s\n",js_cmd_table[i].cmdid,js_cmd_table[i].describe); 
    }
}
/*
********************************************************************************
*Function    : smart
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void smart(int argc, char** argv)
{
    const struct js_list_cmd *cmd;
    int table_size, i;
    
    if (argc < 2 ) {
        show_usage();
        return;
    }
    table_size = sizeof(js_cmd_table)/sizeof(js_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        if(strcmp(argv[1], js_cmd_table[i].cmdid) == 0) {
            break;
        }
    }
    if (i == table_size) {
        show_usage();
        return;
    }
    cmd = &js_cmd_table[i];
    
    if (cmd->docmd) {
        cmd->docmd(argc, argv);
    }
} 
MSH_CMD_EXPORT(smart,smart <help|all|...> );



  