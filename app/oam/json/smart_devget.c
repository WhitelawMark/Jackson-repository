/*
 * smart_devget.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include "app_lib.h"
#include "version.h"

/*
********************************************************************************
********************************************************************************
*/
#define DBG_TAG "smart"
#define DBG_LVL DBG_LOG 
#include <rtdbg.h>
 /*
 * dev-get 函数声明 
 */
#define DEVGET_DEF(pname,operate) \
s32_t operate(cJSON* data,smart_msg_t *psmsg,u8_t offset,js_msg_t *to,void *arg);
#define ACTION_DEF(pname,operate)  
#define DEVSET_DEF(pname,operate)  
#include "smart.def"
/*
********************************************************************************
********************************************************************************
*/
const struct _DEVGET_CMD
{      
    const char *pname;
    s32_t (*CmdAnaly)(cJSON* data,smart_msg_t *psmsg,u8_t offset,js_msg_t *to,void *arg);  /* 功能处理 */
} CmdGetTable[] = { 
    /* cmdid, size,att, ,operate*/
    #define ACTION_DEF(pname,operate)  
    #define DEVGET_DEF(pname,operate)    {pname,operate},
    #define DEVSET_DEF(pname,operate)    
    #include "smart.def"
};
/*
********************************************************************************
********************************************************************************
*/
#define DEF_GETCMD_FN(fn)  s32_t fn(cJSON* data,smart_msg_t *psmsg,u8_t offset,js_msg_t *to,void *arg)

/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    :  smart_devinfo_get 
*Description :  获取网关信息
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_GETCMD_FN(smart_devinfo_get)
{
    cJSON* subdata;
    u8_t tmpbuf[64];
    
    subdata = cJSON_CreateObject();

    cJSON_AddStringToObject(subdata,"cmd", CmdGetTable[offset].pname);

    cJSON_AddStringToObject(subdata,"hw_version", SYS_BOARD_VERSION);
    cJSON_AddStringToObject(subdata,"soft_version", SYS_SOFE_VERSION);
    cJSON_AddStringToObject(subdata,"pub_time", SYS_CREATE_CODE_TIME);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_DEVICE_ID, tmpbuf,  PPItemSize(PP_DEVICE_ID));
    cJSON_AddStringToObject(subdata,"hw_id", (char*)tmpbuf);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_PRD_MODEL, tmpbuf,  PPItemSize(PP_PRD_MODEL));
    cJSON_AddStringToObject(subdata,"model", (char*)tmpbuf);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_PRD_PSN, tmpbuf,  PPItemSize(PP_PRD_PSN));
    cJSON_AddStringToObject(subdata,"SN", (char*)tmpbuf);
    
    return smart_response_send("/dev/get",subdata,psmsg,0,to,arg);
} 
/*
********************************************************************************
*Function    :  smart_networks_get 
*Description :  获取网关信息
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_GETCMD_FN(smart_networks_get)
{
    cJSON* subdata;
    char tmpbuf[64];

    subdata = cJSON_CreateObject();

    cJSON_AddStringToObject(subdata,"cmd", CmdGetTable[offset].pname);

    cJSON_AddStringToObject(subdata,"model", at_get_model());
    cJSON_AddNumberToObject(subdata,"rssi", at_get_rssi());
    cJSON_AddStringToObject(subdata,"imei", at_get_imei());
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_ESIM_CCID0, tmpbuf,  PPItemSize(PP_ESIM_CCID0));
    cJSON_AddStringToObject(subdata,"iccid0",tmpbuf);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_ESIM_CCID1, tmpbuf,  PPItemSize(PP_ESIM_CCID1));
    cJSON_AddStringToObject(subdata,"iccid1",tmpbuf);
    
    memset(tmpbuf,0,64);
    PPItemRead(PP_ESIM_CCID2, tmpbuf,  PPItemSize(PP_ESIM_CCID2));
    cJSON_AddStringToObject(subdata,"iccid2",tmpbuf);

    return smart_response_send("/dev/get",subdata,psmsg,0,to,arg);
}
/*
********************************************************************************
*Function    :  smart_networks_get 
*Description :  获取网关信息
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_GETCMD_FN(smart_system_get)
{
    cJSON* subdata;
    char gwid[36];

    PPItemRead(PP_DEVICE_ID, gwid,  PPItemSize(PP_DEVICE_ID));
    subdata = cJSON_CreateObject();

    cJSON_AddStringToObject(subdata,"cmd", CmdGetTable[offset].pname);

    cJSON_AddStringToObject(subdata,"model", "bg77");
    cJSON_AddNumberToObject(subdata,"rssi", -87);
    cJSON_AddStringToObject(subdata,"imei", "imei");
    cJSON_AddStringToObject(subdata,"iccid0","iccid0");
    cJSON_AddStringToObject(subdata,"iccid1","iccid1");
    cJSON_AddStringToObject(subdata,"iccid2","iccid2");


    return smart_response_send("/dev/get",subdata,psmsg,0,to,arg);
}
/*
********************************************************************************
*Function    :  smart_rtinfo_get 
*Description :  获取网关信息
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_GETCMD_FN(smart_rtinfo_get)
{
    cJSON* subdata;
    char gwid[36];

    PPItemRead(PP_DEVICE_ID, gwid,  PPItemSize(PP_DEVICE_ID));
    subdata = cJSON_CreateObject();

    cJSON_AddStringToObject(subdata,"cmd", CmdGetTable[offset].pname);
#if 0
    cJSON_AddStringToObject(subdata,"hw_version", SYS_BOARD_VERSION);
    cJSON_AddStringToObject(subdata,"soft_version", SYS_SOFT_VERSION);
    cJSON_AddStringToObject(subdata,"pub_time", SYS_CREATE_CODE_TIME);
    cJSON_AddStringToObject(subdata,"hw_id", app_get_hw_id());
    cJSON_AddNumberToObject(subdata,"rssi", wifi_get_rssi());
#endif
    cJSON_AddStringToObject(subdata,"gwid", gwid);
    return smart_response_send("/dev/get",subdata,psmsg,0,to,arg);
}
/*
********************************************************************************
*Function    :  smart_rtinfo_get 
*Description :  获取网关信息
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_GETCMD_FN(smart_mqtt_get)
{
    cJSON* subdata;
    char gwid[36];

    PPItemRead(PP_DEVICE_ID, gwid,  PPItemSize(PP_DEVICE_ID));
    subdata = cJSON_CreateObject();

    cJSON_AddStringToObject(subdata,"cmd", CmdGetTable[offset].pname);

    cJSON_AddStringToObject(subdata,"hw_version", SYS_BOARD_VERSION);
    cJSON_AddStringToObject(subdata,"soft_version", SYS_SOFE_VERSION);
    cJSON_AddStringToObject(subdata,"pub_time", SYS_CREATE_CODE_TIME);

    cJSON_AddStringToObject(subdata,"gwid", gwid);
    return smart_response_send("/dev/get",subdata,psmsg,0,to,arg);
}
/*
********************************************************************************
*Function    :  smart_elecfence_get 
*Description :   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_GETCMD_FN(smart_elecfence_get)
{
    cJSON* subdata;
    u8_t value;
    u32_t distance;
    

    subdata = cJSON_CreateObject();

    cJSON_AddStringToObject(subdata,"cmd", CmdGetTable[offset].pname);

    PPItemRead(PP_ELEFEN_OFFON, &value,  PPItemSize(PP_ELEFEN_OFFON));
    if(value==0){
         cJSON_AddStringToObject(subdata,"switch", "off");
    }else{
         cJSON_AddStringToObject(subdata,"switch", "on");
    }
    PPItemRead(PP_ELEFEN_DISTANCE, &distance,  PPItemSize(PP_ELEFEN_DISTANCE));
    cJSON_AddNumberToObject(subdata,"distance",  distance);    
    cJSON_AddStringToObject(subdata,"longitude", "31.84678");
    cJSON_AddStringToObject(subdata,"latitude", "117.19838");

    return smart_response_send("/dev/get",subdata,psmsg,0,to,arg);
}
/*
********************************************************************************
*Function    :  devget_analy 
*Description :  dev-get 系列解析函数
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
s32_t devget_analy(cJSON* payload,smart_msg_t *psmsg,js_msg_t *to,void *arg)
{
    cJSON* data;
    cJSON* cmd;

    data = cJSON_GetObjectItem(payload, "data");
    if ( data == NULL){
        LOG_E("%s data ",__func__);
        return 0;
    }
    
    cmd = cJSON_GetObjectItem(data, "cmd");
    if ( cmd == NULL || cmd->valuestring == NULL){
        LOG_E("%s cmd ",__func__);
        return 0;
    }
   
    for(int i=0 ; i < OS_COUNTOF(CmdGetTable) ; i++ ){
        if( strcmp(cmd->valuestring,CmdGetTable[i].pname)==0){
            if(CmdGetTable[i].CmdAnaly){
                CmdGetTable[i].CmdAnaly(data,psmsg,i,to,arg);
                return 1;
            }
        }
    }
    LOG_E("%s Unable to find command [%s]",__func__,cmd->valuestring);
    return 0;
}