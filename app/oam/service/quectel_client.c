/*
 * quectel_client.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include <stdio.h>
#include <string.h>
#include <board.h>
#include <rtthread.h>
#include "type.h"
#include "bg77.h"
#include "quectel_client.h" 
#include "oam_thread.h"
#include "app_lib.h"   


/*
********************************************************************************
********************************************************************************
*/
#define DBG_TAG "bg77"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
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
} ATMQTT_CLient;
/*
********************************************************************************
********************************************************************************
*/ 
static ATMQTT_CLient MQTTClient;                                          

#define get_mqtt_client()   (&MQTTClient)                                 
/*
********************************************************************************
********************************************************************************
*/

/*                                                                        
********************************************************************************
*Function    : quectel_mqtt_send_lowlayer                                      
*Description :                                                            
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
static int quectel_mqtt_send_lowlayer(const char *msg, int msglen, void *arg)
{
    (void)arg;

    return quectel_mqtt_send((u8_t*)msg, msglen);
}
/*
********************************************************************************
*Function    : quectel_mqtt_msg_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void quectel_mqtt_msg_process(void *data, int data_len)
{
    LOG_D("playload,%d,[%s]\n",data_len,data);

    cjson_analy(0,data,data_len,quectel_mqtt_send_lowlayer,0); 
}
/*
********************************************************************************
*Function    : quectel_mqtt_recv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void quectel_mqtt_recv(int sd, char *buf, int len)
{
    oam_thread_post(quectel_mqtt_msg_process, buf, len+1);
}
/*
********************************************************************************
*Function    : quectel_mqtt_ev_notify
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void quectel_mqtt_ev_notify(int evtype)
{
    ATMQTT_CLient *client = get_mqtt_client();

    if (evtype == EV_CONNECT) {
        //MQTT 服务器重新链接成功,需要重新订阅
        client->subflag = 0;
    }
}
/*
********************************************************************************
*Function    : quectel_mqtt_subscribe_result
*Description : 订阅结果
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
static void quectel_mqtt_subscribe_result(int res, void *arg)
{
    ATMQTT_CLient *client = get_mqtt_client();

    (void)arg;

    if (res == 0) {
        client->subflag = 1;
    } else {
        client->subflag = 0;
    }
}
/*
********************************************************************************
*Function    : quectel_mqtt_connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int quectel_mqtt_connect(ATMQTT_CLient *client)
{
    if (client->sd < 0) {
        client->sd = at_socket_create(SOCK_MQTT);
        at_set_recv(client->sd, quectel_mqtt_recv);
        at_set_notify(client->sd, quectel_mqtt_ev_notify);
        client->connect = 0;
    }

    //Use SIM id as client id
    if (strcmp(client->ClientID, at_get_sim_id()) != 0) {
        //ClientID 发生变化,需要进行重新连接.
       /// strncpy(client->ClientID, at_get_sim_id(), sizeof(client->ClientID)-1);
        client->connect = 0;
        LOG_D("connect request\n");
    }

    if (client->sd >= 0 && client->connect == 0) {
        if (at_qmt_connect(client->sd,
                           client->url, client->port,
                           client->ClientID,
                           client->username, client->password, AT_NBLOCK, 0) == 0) {
            client->connect = 1;
        }
    }

    if (client->connect == 0) {
        return -1;
    }

    /*
     * 连接成功
     */

    //定阅未成功,先进行订阅
    if (client->subflag == 0) {
        mqtt_topic_t topics;

        topics.qos = 0;
        topics.topic = client->SubTopic;

        client->subflag = 1;
        if (at_qmt_subscribe(client->sd, 1, &topics, 1, quectel_mqtt_subscribe_result, 0) != 0) {
            client->subflag = 0;
        }
    }

    return 0;
}

/*
********************************************************************************
*Function    : quectel_mqtt_is_connected
*Description :
*Input       :
*Output      :
*Return      : 1 if connect, 0 if not
*Others      :
********************************************************************************
*/
int quectel_mqtt_is_connected(void)
{
    return bg77_is_connected(get_mqtt_client()->sd);
}
/*
********************************************************************************
*Function    : quectel_mqtt_send
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int quectel_mqtt_send(u8_t *pmsg, int ulen)
{
    ATMQTT_CLient *client = get_mqtt_client();

    if (quectel_mqtt_connect(client) != 0) {
        return -1;
    }

    if (at_qmt_publish(client->sd, 1, 0, 0, client->PubTopic, (char const*)pmsg, ulen, AT_NBLOCK, 0) != 0) {
        return -1;
    } 
    return 0;
}
/*
********************************************************************************
*Function    : quectel_mqtt_upl_result
*Description : 订阅结果
*Input       :
*Output      :
*Return      :
*Others      :  
********************************************************************************
*/
static void quectel_certificate_upload_result(int res, void *arg)
{
    ATMQTT_CLient *client = get_mqtt_client();

    (void)arg;
    
}
/*
********************************************************************************
*Function    :  quectel_mqtt_cert_upload
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
int quectel_certificate_upload(void)
{
    ATMQTT_CLient *client = get_mqtt_client();
    
    if (client->sd < 0) {
        client->sd = at_socket_create(SOCK_MQTT);
        at_set_recv(client->sd, quectel_mqtt_recv);
        at_set_notify(client->sd, quectel_mqtt_ev_notify);
        client->connect = 0;
    }
    
    bg77_cert_upload(client->sd,CART_UPL ,"cacert.pem","client.pem","user_key.pem",
                     quectel_certificate_upload_result, 0);
    
    return 0;
}
/*
********************************************************************************
*Function    :  quectel_mqtt_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
int quectel_mqtt_init(void)
{
    u32_t *udid;
    
    ATMQTT_CLient *client = get_mqtt_client();

    memset(client, 0, sizeof(ATMQTT_CLient));

    client->sd = -1;
    client->connect = 0;
    client->subflag = 0;

    client->port = 1883;
    
    udid = board_get_cpuid();
    
    sprintf(client->ClientID, "%d%d%d",udid[0],udid[1],udid[2]);
    
  //  PPItemRead(PP_MQTTOPIC,topic,10);

    snprintf(client->PubTopic, sizeof(client->PubTopic)-1, "pubtopics1");
    snprintf(client->SubTopic, sizeof(client->SubTopic)-1, "subtopics");
#if 1
    client->port = 8883;
    strncpy(client->url,      "a1qc8sz6uwatlh-ats.iot.us-east-1.amazonaws.com", sizeof(client->url)-1);
    strncpy(client->username, "\0",   sizeof(client->username)-1);
    strncpy(client->password, "\0",sizeof(client->password)-1);
#else 
    //PPItemRead(PP_MQT_PORT,(u8_t*)&client->port,2);
   // PPItemRead(PP_MQT_URL,client->url,64);
   // PPItemRead(PP_MQT_USR,client->username,20);
    //PPItemRead(PP_MQT_PWD,client->password,20);
#endif

    return 0;
}
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : quectel_https_download_result
*Description : 订阅结果
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
static void quectel_https_download_result(int res, void *arg)
{
    ATMQTT_CLient *client = get_mqtt_client();

    (void)arg;


}
/*
********************************************************************************
*Function    :  quectel_https_download
*Description :
*Input       :
*Output      :
*Return      :
*Others      :  
********************************************************************************
*/
int quectel_https_download(char *url)
{
    ATMQTT_CLient *client = get_mqtt_client();
    
    if (client->sd < 0) {
        client->sd = at_socket_create(SOCK_MQTT);
        at_set_recv(client->sd, quectel_mqtt_recv);
        at_set_notify(client->sd, quectel_mqtt_ev_notify);
        client->connect = 0;
    }
    
    return bg77_https_download(client->sd,url ,"cacert.pem","client.pem","user_key.pem",
                        quectel_https_download_result, 0);
}

/*
********************************************************************************
*Function    : quectel_https_upload_result
*Description : 
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
static void quectel_https_upload_result(int res, void *arg)
{
    ATMQTT_CLient *client = get_mqtt_client();

    (void)arg;

}
/*
********************************************************************************
*Function    :  quectel_https_upload
*Description :
*Input       :
*Output      :
*Return      :
*Others      :  
********************************************************************************
*/
int quectel_https_upload(char *url)
{
    ATMQTT_CLient *client = get_mqtt_client();
    
    if (client->sd < 0) {
        client->sd = at_socket_create(SOCK_MQTT);
        at_set_recv(client->sd, quectel_mqtt_recv);
        at_set_notify(client->sd, quectel_mqtt_ev_notify);
        client->connect = 0;
    }
    
    return bg77_https_upload(client->sd,url,"cacert.pem","client.pem","user_key.pem",
                            quectel_https_upload_result, 0);
}
/*
********************************************************************************
*Function    : quectel_https_upload_result
*Description : 
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
static void quectel_file_download_result(int res, void *arg)
{
    ATMQTT_CLient *client = get_mqtt_client();

    (void)arg;

}
/*
********************************************************************************
*Function    :  quectel_file_download
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
int quectel_file_download(char *srcfile,char *dscfile)
{
    ATMQTT_CLient *client = get_mqtt_client();
    
    if (client->sd < 0) {
        client->sd = at_socket_create(SOCK_MQTT);
        at_set_recv(client->sd, quectel_mqtt_recv);
        at_set_notify(client->sd, quectel_mqtt_ev_notify);
        client->connect = 0;
    }
    
    return bg77_file_export(client->sd,"cacert.pem","cacert.pem", 
                            quectel_file_download_result, 0);
}


/*
********************************************************************************
*Function    : quectel_file_upload_result
*Description : 
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
static void quectel_file_upload_result(int res, void *arg)
{
    ATMQTT_CLient *client = get_mqtt_client();

    (void)arg;


}
/*
********************************************************************************
*Function    :  quectel_file_upload
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
int quectel_file_upload(char *srcfile,char *dscfile)
{
    ATMQTT_CLient *client = get_mqtt_client();
    
    if (client->sd < 0) {
        client->sd = at_socket_create(SOCK_MQTT);
        at_set_recv(client->sd, quectel_mqtt_recv);
        at_set_notify(client->sd, quectel_mqtt_ev_notify);
        client->connect = 0;
    }
    return bg77_file_import(  client->sd , "cacert.pem",  "cacert.pem", quectel_file_upload_result, 0);
}
