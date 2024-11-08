/*
 * esp_client.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include <stdio.h>
#include <string.h>
#include <board.h>
#include <rtthread.h>
#include "type.h"
#include "bt_service.h" 
#include "oam_thread.h" 
#include "at.h"
#include "at_client.h"
#include "esp.h"
#include "esp_bt.h"   
#include "esp_mqtt.h"
#include "esp_https.h"
#include "PPItems.h"
#include "esp_client.h"


#include "cjson.h"
#include "smart_analy.h"

#define DBG_TAG "esp"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>

/*
********************************************************************************
********************************************************************************
*/ 

typedef struct  {
    int sd;
    int connect;
    char url[256];
    char name[64];
} HTTP_Service_t;

typedef struct
{
    int sd;
    int connect;
    int subflag;
    char  PubTopic[128];
    char  SubTopic[128];
    char  ClientID[128];
    char  username[128];
    char  password[128];
    char  url[128];
    unsigned short port;
} MQTT_CLient;
/*
********************************************************************************
********************************************************************************
*/  
static u8_t RxBuf[1024];  
static MQTT_CLient MQTTClient; 
static HTTP_Service_t HTTPService; 

#define get_mqtt_client()   (&MQTTClient)       
#define get_https_service() (&HTTPService) 
/*
********************************************************************************
********************************************************************************
*/ 
 
/*                                                                        
********************************************************************************
*Function    : esp_mqtt_send_lowlayer                                      
*Description :                                                            
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
static int esp_mqtt_send_lowlayer(const char *msg, int msglen, void *arg)
{
    (void)arg;

    return esp_mqtt_send(msg, msglen);
}
/*
********************************************************************************
*Function    : esp_mqtt_subscribe_result
*Description : 订阅结果
*Input       :
*Output      :
*Return      :
*Others      : 该 函数由AT 模块调用,用于通知订阅的最终结果
********************************************************************************
*/
static void esp_mqtt_subscribe_result(int res, void *arg)
{
    MQTT_CLient *client = get_mqtt_client();

    (void)arg;

    if (res == 0) {
        client->subflag = 1;
    } else {
        client->subflag = 0;
    }
}
/*
********************************************************************************
*Function    : esp_mqtt_msg_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void esp_mqtt_msg_process(void *data, int data_len)
{
    cjson_analy(0,data,data_len,esp_mqtt_send_lowlayer,0); 
}
/*
********************************************************************************
*Function    : esp_mqtt_recv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void esp_mqtt_recv(int sd, char *buf, int len)
{
    memcpy(RxBuf,buf,len);
    oam_thread_post(esp_mqtt_msg_process, RxBuf, len);
}
/*
********************************************************************************
*Function    : esp_mqtt_ev_notify
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void esp_mqtt_ev_notify(int evtype)
{
    MQTT_CLient *client = get_mqtt_client();

    if (evtype == EV_CONNECT) {
        //MQTT 服务器重新链接成功,需要重新订阅
        client->subflag = 0;
    }
}
/*
********************************************************************************
*Function    : esp_connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int esp_connect(MQTT_CLient *client)
{
    if (client->sd < 0) {
        client->sd = esp_socket_create(SOCK_MQTT);
        esp_set_recv(client->sd, esp_mqtt_recv);
        esp_set_notify(client->sd, esp_mqtt_ev_notify);
        client->connect = 0;
    }

    if (client->sd >= 0 && client->connect == 0) {
        if (esp_mqtt_connect(client->sd,
                           client->url, client->port,
                           client->ClientID,
                           client->username, client->password, 0, 0) == 0) {
            client->connect = 1;
        }
    }

    if (client->connect == 0) {
        return -1;
    }

    /*连接成功*/

    //定阅未成功,先进行订阅
    if (client->subflag == 0) {
        mqtt_topic_t topics;

        topics.qos = 1;
        topics.topic = client->SubTopic;

        client->subflag = 1;
        if (esp_mqtt_subscribe(client->sd, 1, &topics, 1, esp_mqtt_subscribe_result, 0) != 0) {
            client->subflag = 0;
        }
    }

    return 0;
}
/*
********************************************************************************
*Function    : esp_mqtt_send
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int esp_mqtt_send(const char *pmsg, int ulen)
{
    MQTT_CLient *client = get_mqtt_client();

    if (esp_connect(client) != 0) {
        return -1;
    }

    if (esp_mqtt_publish(client->sd, 1, 0, 0, client->PubTopic, (char const*)pmsg, ulen, 0, 0) != 0) {
        return -1;
    } 
    return 0;
}

/*
********************************************************************************
*Function    :  esp32_wifi_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :  
********************************************************************************
*/
int esp32_wifi_init(void)
{
    u8_t ssid[16];
    u8_t pwd[16];
    
    MQTT_CLient *client = get_mqtt_client();

    memset(client, 0, sizeof(MQTT_CLient));
    
    client->sd = -1;
    client->connect = 0;
    client->subflag = 0;
#if 0
    snprintf(client->PubTopic, sizeof(client->PubTopic)-1, "pubtopics");
    snprintf(client->SubTopic, sizeof(client->SubTopic)-1, "subtopics");
    snprintf(client->ClientID, sizeof(client->ClientID)-1, "ClientID");    
    
    client->port = 8883;
    strncpy(client->url,      "a1qc8sz6uwatlh-ats.iot.us-east-1.amazonaws.com", sizeof(client->url)-1);
    strncpy(client->username, " ",   sizeof(client->username)-1);
    strncpy(client->password, " ",sizeof(client->password)-1);
#else 
    
    PPItemRead(PP_WIFI_NAME,ssid,PPItemSize(PP_WIFI_NAME));
    PPItemRead(PP_WIFI_PWD,pwd,PPItemSize(PP_WIFI_PWD));
    
    PPItemRead(PP_PUB_TOPICS1,client->PubTopic,PPItemSize(PP_PUB_TOPICS1));
    PPItemRead(PP_SUB_TOPICS1,client->SubTopic,PPItemSize(PP_SUB_TOPICS1));
    
    PPItemRead(PP_MQT_PORT,(u8_t*)&client->port,2);
    PPItemRead(PP_MQT_URL,client->url,PPItemSize(PP_MQT_URL));
    PPItemRead(PP_MQT_CLIENTID,client->ClientID,PPItemSize(PP_MQT_CLIENTID));
    
    PPItemRead(PP_MQT_USERNAME,client->username,PPItemSize(PP_MQT_USERNAME));
    PPItemRead(PP_MQT_PASSWD,client->password,PPItemSize(PP_MQT_PASSWD));
#endif
    esp_mqtt_init((char*)ssid,(char*)pwd);
    
    return 0;
}
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : https_connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int https_connect(HTTP_Service_t *service)
{
    if (service->sd < 0) {
        service->sd = esp_socket_create(SOCK_HTTPS);
        esp_set_recv(service->sd, 0);
        service->connect = 1;
    }

    return 0;
}
/*
********************************************************************************
*Function    : https_disconnect
*Description :
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
void https_disconnect(void)
{
    HTTP_Service_t *service = get_https_service();
    if (service->sd >= 0) {
        service->sd = -1;
    }
}
/*
********************************************************************************
*Function    : https_download_result
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void https_download_result(int res, void *arg)
{

}
/*
********************************************************************************
*Function    : https_download
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int https_download(char *url,char *filename)
{
    HTTP_Service_t *service = get_https_service();
    
    if (https_connect(service) != 0) {
        return -1;
    }

    return esp_https_download(service->sd,url,filename, https_download_result, 0);
}
/*
********************************************************************************
*Function    :  https_service_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
int https_service_init(void)
{
    u8_t ssid[16];
    u8_t pwd[16];
    
	HTTP_Service_t *client = get_https_service();

    memset(client, 0, sizeof(HTTP_Service_t));

    client->sd = -1;
    
    PPItemRead(PP_WIFI_NAME,ssid,PPItemSize(PP_WIFI_NAME));
    PPItemRead(PP_WIFI_PWD,pwd,PPItemSize(PP_WIFI_PWD));
    
    esp_https_init((char*)ssid,(char*)pwd);
    
    return 0;
}