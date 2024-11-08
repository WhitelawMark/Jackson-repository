/*
 * smart_port.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include "app_lib.h"
#include "version.h"


#define DBG_TAG "smart"
#define DBG_LVL DBG_LOG 
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
*Function    :  smart_mqtt_send_lowlayer 
*Description :  MQTT数据接收回调函数 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int smart_mqtt_send_lowlayer(const char *msg, int msglen, void *arg)
{
    return 0;// mqtt_send(msg, msglen,arg);
} 

/*
********************************************************************************
*Function    :  smart_mqtt_msg_process 
*Description :  MQTT数据接收处理函数 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void smart_mqtt_msg_process(void *data, int data_len)
{
    cjson_analy(0,data,data_len,smart_mqtt_send_lowlayer,0); 
}

/*
********************************************************************************
*Function    :  smart_mqtt_recv 
*Description :  MQTT数据接收函数 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int smart_mqtt_recv(char *buf, int len)
{
    oam_thread_post(smart_mqtt_msg_process, buf, len);
    return 0;
}
/*
********************************************************************************
*Function    :  smart_report_heartbeat
*Description :  开机上报 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int smart_report_heartbeat(void)
{
    cJSON* data; 

    data = cJSON_CreateObject();

    cJSON_AddStringToObject(data,"cmd", "/heartbeat");
    cJSON_AddStringToObject(data,"battery", "310");
    cJSON_AddStringToObject(data,"longitude", "31.84678");
    cJSON_AddStringToObject(data,"latitude", "117.19838");
    cJSON_AddStringToObject(data,"x", "122");
    cJSON_AddStringToObject(data,"y", "022");
    cJSON_AddStringToObject(data,"z", "002");

 
    return smart_oam_send("/dev/rpt",NULL,data,SMART_OK);
}
/*        
********************************************************************************
*Function    :  smart_report_poweron 
*Description :  开机上报 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int smart_report_poweron(void)
{
    cJSON* data; 
    u8_t tmpbuf[64];
    
    data = cJSON_CreateObject();

    cJSON_AddStringToObject(data,"cmd", "/poweron");
    
    cJSON_AddStringToObject(data,"soft_version", SYS_SOFE_VERSION);
    cJSON_AddStringToObject(data,"hw_version", SYS_BOARD_VERSION);
    cJSON_AddStringToObject(data,"pub_time", SYS_CREATE_CODE_TIME);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_DEVICE_ID, tmpbuf,  PPItemSize(PP_DEVICE_ID));
    cJSON_AddStringToObject(data,"hw_id", (char*)tmpbuf);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_PRD_MODEL, tmpbuf,  PPItemSize(PP_PRD_MODEL));
    cJSON_AddStringToObject(data,"model", (char*)tmpbuf);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_PRD_PSN, tmpbuf,  PPItemSize(PP_PRD_PSN));
    cJSON_AddStringToObject(data,"SN", (char*)tmpbuf);
    
    return smart_oam_send("/dev/rpt",NULL,data,SMART_OK);
}
/*
********************************************************************************
*Function    :  smart_report_poweron 
*Description :  升级上报 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int smart_report_ota(void)
{
    cJSON* data; 
    u8_t tmpbuf[64];

    data = cJSON_CreateObject();

    cJSON_AddStringToObject(data,"cmd", "/ota");
    cJSON_AddStringToObject(data,"status", "succeff");
    cJSON_AddStringToObject(data,"soft_version", SYS_SOFE_VERSION);
    cJSON_AddStringToObject(data,"hw_version", SYS_BOARD_VERSION);
    cJSON_AddStringToObject(data,"pub_time", SYS_CREATE_CODE_TIME);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_DEVICE_ID, tmpbuf,  PPItemSize(PP_DEVICE_ID));
    cJSON_AddStringToObject(data,"hw_id", (char*)tmpbuf);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_PRD_MODEL, tmpbuf,  PPItemSize(PP_PRD_MODEL));
    cJSON_AddStringToObject(data,"model", (char*)tmpbuf);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_PRD_PSN, tmpbuf,  PPItemSize(PP_PRD_PSN));
    cJSON_AddStringToObject(data,"SN", (char*)tmpbuf);
    
    return smart_oam_send("/dev/rpt",NULL,data,SMART_OK);
}
/*
********************************************************************************
*Function    :  smart_report_lowbattery 
*Description :  电池低电压上报 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int smart_report_lowbattery(void)
{
    cJSON* data; 

    data = cJSON_CreateObject();

    cJSON_AddStringToObject(data,"cmd", "/low/battery");
    cJSON_AddStringToObject(data,"battery", "alarm");
    cJSON_AddStringToObject(data,"voltage", "1700");
    
    return smart_oam_send("/dev/rpt",NULL,data,SMART_OK);
}
/*
********************************************************************************
*Function    :  smart_report_elecfence 
*Description :  电子围栏上报 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int smart_report_elecfence(void)
{
    cJSON* data; 

    data = cJSON_CreateObject();

    cJSON_AddStringToObject(data,"cmd", "/elec/fence");
    cJSON_AddStringToObject(data,"status", "alarm");
    cJSON_AddStringToObject(data,"longitude", "31.84678");
    cJSON_AddStringToObject(data,"latitude", "117.19838");
    
    return smart_oam_send("/dev/rpt",NULL,data,SMART_OK);
}
/*
********************************************************************************
*Function    :  smart_report_message 
*Description :  smart消息上报  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int smart_report_message(int evt)
{ 
    int result=-1;
   
    switch(evt){
        case SMART_RPT_POWERON:
            result = smart_report_poweron();
            break;
        case SMART_RPT_HEARTBEAT:
            result = smart_report_heartbeat();
            break;
        case SMART_RPT_OTA:
            result = smart_report_ota();
            break;
        case SMART_RPT_LOWBAT:
            result = smart_report_lowbattery();
            break;
        case SMART_RPT_ELECFENCE:
            result = smart_report_elecfence();
            break;
      default:
        break;
    }
    return result;
}
/*
********************************************************************************
*Function    :  smart_ota_response 
*Description :  ota状态回复 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void smart_ota_response(int ret)
{
    cJSON* data;
    int result= SMART_OK;
#if 0
    switch(ret){
        case OTA_ERR_NONE:
            result = SMART_OK;
            break;
        case OTA_ERR_UNKNOWN:
            result = SMART_FAIL;
            break;
        case OTA_ERR_ONGING:
            result = SMART_EXISTED;
            break;
        case OTA_ERR_MODEL:
            result = SMART_UAUTH;
            break;
        case OTA_ERR_CRC:
            result = SMART_UAUTH;
            break;
        case OTA_ERR_LEN:
            result = SMART_UAUTH;
            break;
        default:
            result = SMART_FAIL;
            break;
    }
#endif
  //  oam_ota_report_stop();
 
   // system_reboot();

    data = cJSON_CreateObject();

    cJSON_AddStringToObject(data,"cmd", "/ota_start");

    smart_oam_send("/dev/set",NULL,data,result);
}




