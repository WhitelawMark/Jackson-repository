/*
 * smart_analy.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include "app_lib.h"

static const char *TAG = "smart";

#define cJSON_free rt_free

#define DBG_TAG "smart"
#define DBG_LVL DBG_LOG 
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
/*
 * smart action 函数声明 
 */ 
#define ACTION_DEF(pname,operate) \
s32_t operate(cJSON* data,smart_msg_t *psmsg,js_msg_t *to,void *arg);
#define DEVGET_DEF(pname,operate)    
#define DEVSET_DEF(pname,operate)    
#include "smart.def"
 
/*
 * smart action Table 
 */
const struct _SMART_ACTION
{      
    const char *pname;
    s32_t (*ActionAnaly)(cJSON* data,smart_msg_t *psmsg,js_msg_t *to,void *arg);  /* 功能处理 */
} ActionTable[] = {
    /* cmdid, size,att,pmem,func*/
    #define ACTION_DEF(pname,operate) {pname,operate},
    #define DEVGET_DEF(pname,operate)    
    #define DEVSET_DEF(pname,operate)    
    #include "smart.def"
};
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    :  gwid_cfg_analy 
*Description :   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
s32_t gwid_cfg_analy(cJSON* payload,smart_msg_t *psmsg,js_msg_t *to,void *arg)
{
    int result = SMART_OK;
    cJSON* data;
    cJSON* gwid;

    data = cJSON_GetObjectItem(payload, "data");
    if ( data == NULL){
        LOG_E("%s data ",__func__);
        return 0;
    }
    
    gwid = cJSON_GetObjectItem(data, "gwid");
    if ( gwid == NULL || gwid->valuestring == NULL){
        LOG_E("%s ssid ",__func__);
        return 0;
    }
	PPItemWrite(PP_DEVICE_ID, gwid->valuestring,  strlen(gwid->valuestring));

    return smart_response_send("/gwid/set",NULL,psmsg,result,to,arg);
}
/*
********************************************************************************
*Function    :  smart_response_send 
*Description :  封装好的数据接口
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
s32_t wifiset_analy(cJSON* payload,smart_msg_t *psmsg,js_msg_t *to,void *arg)
{
    cJSON* data;
    cJSON* ssid;
    cJSON* key;
    int result = SMART_OK;

    data = cJSON_GetObjectItem(payload, "data");
    if ( data == NULL){
        LOG_E("%s data ",__func__);
        return 0;
    }
    
    ssid = cJSON_GetObjectItem(data, "ssid");
    if ( ssid == NULL || ssid->valuestring == NULL){
        LOG_E("%s ssid ",__func__);
        return 0;
    }
    LOG_E("%s ssid: %s",__func__,ssid->valuestring);
    key = cJSON_GetObjectItem(data, "key");
    if ( key == NULL || key->valuestring == NULL){
        LOG_E(TAG,"%s key ",__func__);
        return 0;
    }
    LOG_I("%s key: %s",__func__,key->valuestring);

	//PPItemWrite(PP_WIFI_SSID, ssid->valuestring,  strlen(ssid->valuestring));

    //PPItemWrite(PP_WIFI_PWD, key->valuestring, strlen(key->valuestring));
   
    return smart_response_send("/wifi/set",NULL,psmsg,result,to,arg);
}
/*
********************************************************************************
*Function    :  wifiget_analy 
*Description :  wifi配置读取
*Input       :  payload: payload json信息
**              psmsg  : smart相关参数
**              to     : 回复参数
**              arg    : 预留接口
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
s32_t wifiget_analy(cJSON* subdata,smart_msg_t *psmsg,js_msg_t *to,void *arg)
{
    cJSON* data;
    char ssid[36]={0};
    char password[36]={0};
    int result = SMART_OK;

   // PPItemRead(PP_WIFI_SSID, ssid,  PPItemSize(PP_WIFI_SSID)); 

  //  PPItemRead(PP_WIFI_PWD, password,  PPItemSize(PP_WIFI_PWD)); 

    data = cJSON_CreateObject();

    cJSON_AddStringToObject(data,"ssid", ssid);

    cJSON_AddStringToObject(data,"key", password);

    return smart_response_send("/wifi/get",data,psmsg,result,to,arg);
} 
/*
********************************************************************************
*Function    :  gwdiscover_analy 
*Description :  封装好的数据接口
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
s32_t gwdiscover_analy(cJSON* subdata,smart_msg_t *psmsg,js_msg_t *to,void *arg)
{
    cJSON* data;
    int result = SMART_OK;
    char gwid[36]={0};
    
  
    data = cJSON_CreateObject();

    PPItemRead(PP_DEVICE_ID, gwid,  PPItemSize(PP_DEVICE_ID));  

    cJSON_AddStringToObject(data,"gwid", gwid);

    //cJSON_AddStringToObject(data,"mac", app_get_hw_id());

    //cJSON_AddStringToObject(data,"ip", get_ip_address());

    cJSON_AddStringToObject(data,"port", "8023");

    return smart_response_send("/gw/discover",data,psmsg,result,to,arg);
}
/*
********************************************************************************
*Function    :  smart_analy 
*Description :  数据解析接口函数
*Input       :  msgtype ：消息类型
*               root   ：json消息
*               from   : 回调函数
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u8_t smart_analy(u8_t msgtype,cJSON* root, json_lowlayer *from,void *arg)
{
    cJSON* payload;
    cJSON* pitems;
    smart_msg_t smsg; 
    smart_msg_t *psmsg; 
    char gwid[36];
    js_msg_t from_msg;



    psmsg = &smsg;
    from_msg.from = from;
    from_msg.msgtype = msgtype;
    from_msg.arg =NULL;

    /*1、源地址 from */
    pitems = cJSON_GetObjectItem(root, "from");
    if (pitems==NULL || pitems->valuestring == NULL){
        LOG_E("%s from ",__func__);
        goto exit;
    }
    strcpy(psmsg->from,pitems->valuestring);   
    /*2、目标地址 to */
    pitems = cJSON_GetObjectItem(root, "to");
    if ( pitems == NULL ){
        LOG_E("%s to ",__func__);
        goto exit;
    }
    if( pitems->valuestring != NULL ){
        PPItemRead(PP_DEVICE_ID, gwid,  PPItemSize(PP_DEVICE_ID));  
        if( strncmp(pitems->valuestring,gwid,32) != 0 ){
            LOG_E("%s gwid Mismatch",__func__);
            goto exit;
        }
        psmsg->broadcast = 0;
    }else{
        psmsg->broadcast = 1;
    }

    /*3、密钥鉴权 encrypt */
    pitems = cJSON_GetObjectItem(root, "encrypt");
    if (pitems == NULL || pitems->valuestring == NULL){
        LOG_E("%s encrypt",__func__);
        goto exit;
    }
    if(strcmp(pitems->valuestring, "none") == 0) {
        psmsg->encrypt = 0;
    }
    /*4、数据单元 payload */
    payload = cJSON_GetObjectItem(root, "payload");
    if ( payload == NULL){
        LOG_E("%s payload ",__func__);
        goto exit;
    }
    
    /*5、包序号 seq*/
    pitems = cJSON_GetObjectItem(payload, "seq");
    if ( pitems == NULL || pitems->valuestring == NULL){
        LOG_E("%s seq ",__func__);
        goto exit;
    }
    /*6、命令属性 action*/
    pitems = cJSON_GetObjectItem(payload, "action");
    if ( pitems == NULL || pitems->valuestring == NULL){
        LOG_E("%s Illegal action ",__func__);
        goto exit;
    }

    for(int i=0 ; i < OS_COUNTOF(ActionTable) ; i++ ){
        if( strcmp(pitems->valuestring,ActionTable[i].pname)==0){
            if(ActionTable[i].ActionAnaly){
                ActionTable[i].ActionAnaly(payload,psmsg,&from_msg,(void*)from);
            }
            return 1;
        }
    }
    LOG_E("%s  Action %s ",__func__,pitems->valuestring);

exit:

    LOG_E("%s  Illegal protocol format",__func__);
    return 0;
}  
/*
********************************************************************************
*Function    :  smart_send 
*Description :  通用数据接口
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static u8_t smart_send(char *from,char* to,char encrypt,cJSON* payload,js_msg_t *ptomsg,void *arg)
{
    cJSON* root;
    char *result;

    root=cJSON_CreateObject();

    cJSON_AddStringToObject(root,"from", from);

    if( to == NULL ){
        cJSON_AddNullToObject(root,"to");
    }else{
        cJSON_AddStringToObject(root,"to", to);
    }

    cJSON_AddStringToObject(root,"encrypt", "none");

    cJSON_AddItemToObject(root,"payload",payload);

    result=cJSON_PrintUnformatted(root);

    LOG_I("%s ",result); 

    cJSON_Delete(root);

    if(ptomsg && ptomsg->from){
        ptomsg->from(result,strlen(result),ptomsg->arg);
    }
    cJSON_free(result);

    return 0;
}
/*
********************************************************************************
*Function    :  smart_send_report 
*Description :  封装好的数据接口
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static u8_t smart_send_report(char *action,char *from,cJSON* data,int result,js_msg_t *to,void *arg)
{
    cJSON* payload;
    int encrypt=0;
    char msgid[69]={0};

    snprintf(msgid,68,"%x",rt_rng_read());

    payload=cJSON_CreateObject();

    cJSON_AddStringToObject(payload,"seq", msgid);

    cJSON_AddStringToObject(payload,"action", action);

    if( result != 0 ){
        cJSON_AddNumberToObject(payload,"result",result);
    }
    if( data != 0 ){
        cJSON_AddItemToObject(payload,"payload",data);
    }

    smart_send(get_gwid(),from,encrypt,payload,to,0);
    
    return 0;
}
/*
********************************************************************************
*Function    :  smart_response_send 
*Description :  封装好的数据接口
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u8_t smart_response_send(char *action,cJSON* data,smart_msg_t *psmsg,int result,js_msg_t *to,void *arg)
{
    cJSON* payload;
    char msgid[69]={0};
 
    snprintf(msgid,68,"%x",rt_rng_read());

    payload=cJSON_CreateObject();

    cJSON_AddStringToObject(payload,"seq", msgid);

    cJSON_AddStringToObject(payload,"action", action);

    if( result != 0 ){
        cJSON_AddNumberToObject(payload,"result",result);
    }
    if( data != 0 ){
        cJSON_AddItemToObject(payload,"payload",data);
    }

    smart_send(get_gwid(),psmsg->from,psmsg->encrypt,payload,to,arg);

    return 0;
}
/*
********************************************************************************
*Function    :  smart_oam_send 
*Description :  封装好的数据接口
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u8_t smart_oam_send(char *action,char *from,cJSON* data,int result)
{
    js_msg_t layer;    

    layer.from = 0;  
    layer.arg = 0;        
    layer.msgtype = 0;  

    smart_send_report(action,from,data,result,&layer,NULL);
    return 0;
}
/*
********************************************************************************
*Function    :  cjson_analy 
*Description :  smart数据解析接口函数
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u8_t cjson_analy(u8_t msgtype,const char *pmsg,u16_t ulen, json_lowlayer *from,void *arg)
{
    cJSON* root;
    cJSON* pitems;

    root  = cJSON_Parse(pmsg+1);
    if (root == NULL){
        LOG_E("%s cJSON Parse",__func__);
        return -1;
    }

    pitems = cJSON_GetObjectItem(root, "from");
    if (pitems != NULL  ){
        smart_analy(msgtype,root,from,arg);
    } 
    
    LOG_I("%s cJSON_Delete",__func__);

    cJSON_Delete(root);

    return 0;
}   
