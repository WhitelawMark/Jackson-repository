/*
 *  esp_mqtt.c
 *
 *  Created on: 2023年08月26日
 *
 *      Author: lwp
 *     
 *      Note  : 该函数目前已对接亚马逊的aws的平台，烧录的at固件已是实际亚马逊的  
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <drivers/pin.h>
#include "at.h"
#include "at_client.h"
#include "esp.h"
#include "esp_mqtt.h"


#define DBG_TAG "esp"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
extern esp_at_t esp;
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
                           esp mqtt 内部接口函数
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : __esp_qmt_open
*Description :
*Input       :
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      : AT+MQTTCONN=<LinkID>,<"host">,<port>,<reconnect>
               AT+MQTTUSERCFG=<LinkID>,<scheme>,<"client_id">,<"username">,<"password">,<cert_key_ID>,<CA_ID>,<"path">
********************************************************************************
*/
int __esp_qmt_open(int sd, at_addr_t *addr, const char *clientid, const char *username, const char *password)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20];

    //ESP32-C2 MQTT 发布者：
    at_snprintf(atcmd, sizeof(atcmd), "AT+MQTTUSERCFG=0,5,\"%s\",\"%s\",\"%s\",0,0,\"%d\"\r\n",
                                      clientid,
                                      username,
                                      password,
                                      0,
                                      0,
                                      "");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (esp_cmd_line(atcmd, "OK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    //AT+QMTOPEN=client_idx,"host name",port
    at_snprintf(atcmd, sizeof(atcmd), "AT+MQTTCONN=%d,\"%s\",%d,%d\r\n",
                                      0,
                                      addr->ip,
                                      (unsigned int)(addr->port),
                                      1);
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (esp_cmd_line(atcmd, "OK", xbuf, sizeof(xbuf), 5000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    str = xbuf;

    /*skip client_idx*/
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    /*result*/
    res = at_atoi(str);
    if (res != 0) {
        return -1;
    }

    return sd;
}
/*
********************************************************************************
*Function    : __esp_mqtt_close
*Description :
*Input       :
*Output      :
*Return      : 0   -- if success, -1 -- if fail.
*Others      : AT+MQTTCLEAN=<LinkID>
               OK
********************************************************************************
*/
int __esp_mqtt_close(int sd)
{
    char atcmd[32];

    at_snprintf(atcmd, sizeof(atcmd), "AT+MQTTCLEAN=0\r\n");
    atcmd[sizeof(atcmd)-1] = '\0';

    if (esp_cmd_line(atcmd, "OK", 0, 0, 5000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }

    return 0;
}
/*
********************************************************************************
*Function    : esp_qmt_disc
*Description :
*Input       :
*Output      :
*Return      : 0   -- if success, -1 -- if fail.
*Others      : 
********************************************************************************
*/
int esp_qmt_disc(int sd)
{
    char atcmd[32];

    at_snprintf(atcmd, sizeof(atcmd), "AT+QMTDISC=%d\r", sd);
    atcmd[sizeof(atcmd)-1] = '\0';

    if (esp_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }

    return 0;
}
/*
********************************************************************************
*Function    : __esp_mqtt_open
*Description :
*Input       :
*Output      :
*Return      : socket
*Others      :
********************************************************************************
*/
int __esp_mqtt_open(int connect_id, at_addr_t *addr, const char *clientid, const char *username, const char *password)
{
    int sd;

    if (addr->ip[0] == '\0' || clientid == RT_NULL) {
        return -2;
    }

    sd = __esp_qmt_open(connect_id, addr,clientid,username,password);
    if (sd < 0) {
        LOG_E("MQTT socket (peer addr %s:%u) open fail\n", addr->ip, (unsigned int)(addr->port));
        sd = -1;
    } else {

    }
    return sd;
}
/*
********************************************************************************
*Function    : __esp_mqtt_receive
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int __esp_mqtt_receive(socket_t *socket, int msgid, const char *topic, int topic_len,  const char  *payload,int payload_len)
{
    mqtt_msg_t *mqtt_msg;
    void (*recv)(int sd, char *buf, int len);

    if (socket->type != SOCK_MQTT) {
        return -1;
    }

    mqtt_msg = at_message_alloc(sizeof(mqtt_msg_t) + topic_len + payload_len);
    if (mqtt_msg == RT_NULL) {
        LOG_E("mqtt_msg_t alloc fail\n");
        return -1;
    }
    at_memset(mqtt_msg, 0, sizeof(mqtt_msg_t));
    at_message_type(mqtt_msg) = SOCK_MQTT;

    mqtt_msg->msgid = msgid;
    mqtt_msg->retain = 0;
    mqtt_msg->qos = 0;
    mqtt_msg->topic_len = topic_len;
    mqtt_msg->topic = (char *)(mqtt_msg + 1);
    mqtt_msg->payload = mqtt_msg->topic + topic_len;

    //topic
    at_memcpy(mqtt_msg->topic, topic, topic_len);
    at_memcpy(mqtt_msg->payload, payload, payload_len);

    //payload
    if (payload_len == 0) {
        at_message_free(mqtt_msg);
    } else {
        mqtt_msg->payload_len = payload_len;

        //put message
        recv = (void (*)(int,char*,int))__atomic_read((volatile long *)&socket->recv);
        if (recv) {
            recv(socket->index, mqtt_msg->payload, mqtt_msg->payload_len);
            at_message_free(mqtt_msg);
        } else {
            at_message_put(&socket->rx_mq, mqtt_msg, socket->notify, EV_RX);
        }
    }

    return 0;
}

/*
********************************************************************************
*Function    : __esp_urc_qmtrecv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
              +MQTTSUBRECV:0,"topic/esp32at",10,hello aws!
              OK
********************************************************************************
*/
void __esp_urc_qmtrecv(const char *data, int len)
{
    char *str = (char *)data;
    socket_t *socket;
    int sd;
    int payload_len,topic_len,ulen;
    const char *topic;
    const char *payload;
    
    /*client_idx*/
    sd = at_atoi(str);
    if (sd < 0) {
        return;
    }
    //0,"subtopics",238,
    /*skip client_idx*/
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    
    //topic
    while (at_isspace(*str)) str++;
    if (*str == '\"') str++; //skip '"'
    topic = str;
   

    /*skip client_idx*/
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','

    //msglen. The message identifier of packet
    payload_len = at_atoi(str);
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    ulen  = at_strlen(str);
    topic_len = at_strlen(str);
    payload = str;
    
    if (ulen == payload_len ) {
        socket = esp_get_socket(sd, SOCK_MQTT);
        if (socket != RT_NULL) {
            __esp_mqtt_receive(socket, 0, topic, topic_len,payload, payload_len);
        }
    }
}
/*
********************************************************************************
*Function    : __esp_urc_qmtdisconn
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
               +MQTTDISCONNECTED:0
********************************************************************************
*/
void __esp_urc_qmtdisconn(const char *data, int len)
{
    char *str = (char *)data;
    socket_t *socket;
    int sd;
    
    /*client_idx*/
    sd = at_atoi(str);
    if (sd < 0) {
        return;
    }
    LOG_E("MQTT disconnects the link sd:%d \n",sd);
    socket = esp_get_socket(sd, SOCK_MQTT);
    if (socket != RT_NULL) {
        //for reconnect
        LOG_E("peer connect closed\n");
        __esp_close(socket);
    }
}
/*
********************************************************************************
*Function    : __esp_mqtt_unsubscribe
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      : AT+MQTTUNSUB=<LinkID>,<"topic">
               NO UNSUBSCRIBE

               OK
********************************************************************************
*/
int __esp_mqtt_unsubscribe(socket_t *socket, sub_msg_t *msg)
{
    mqtt_topic_t *topic;
    char *pbuf = esp.tmpbuf;
    int size = sizeof(esp.tmpbuf)-2;
    int res, curr_len;

    if (msg->topics == RT_NULL || msg->number <= 0) {
        return -1;
    }

    if (socket->type != SOCK_MQTT) {
        LOG_E("Wrong socket type, need MQTT socket\n");
        return -1;
    }

    if (socket->sd < 0) {
       if (__esp_connect(socket, RT_NULL, RT_NULL) != 0) {
           LOG_E("MQTT socket connect fail\n");
           return -1;
       }
    }

    at_snprintf(pbuf, size, "AT+MQTTUNSUB=%d", socket->sd);
    pbuf[size] = '\0';

    curr_len = at_strlen(pbuf);
    for (res = 0; res < msg->number; res++) {
        topic = &msg->topics[res];
        at_snprintf(pbuf + curr_len, size - curr_len, ",\"%s\"", topic->topic?topic->topic:"");
        curr_len = at_strlen(pbuf);
    }
    pbuf[curr_len++] = '\r';
    pbuf[curr_len++] = '\n';
    pbuf[curr_len] = '\0';

    if ((res = esp_cmd_line(pbuf, "OK", 0, 0, 8000, 1)) != AT_CMD_ACK_OK) {
        socket->fail_count++;
        LOG_E("Fail to exec at cmd: %d, %s\n", res, pbuf);
        return -1;
    }

    socket->fail_count = 0;

    return 0;
}

/*
********************************************************************************
*Function    : __esp_mqtt_subscribe
*Description : esp mqtt 订阅处理函数 
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      : cmd : AT+MQTTSUB=<LinkID>,<"topic">,<qos>
               ack : OK
********************************************************************************
*/
int __esp_mqtt_subscribe(socket_t *socket, sub_msg_t *msg)
{
    mqtt_topic_t *topic;
    char *pbuf = esp.tmpbuf;
    int size = sizeof(esp.tmpbuf)-2;
    int res, curr_len;
 
    LOG_D("AT Subscribe process ....\n");

    if (msg->topics == RT_NULL || msg->number <= 0) {
        return -1;
    }

    if (socket->type != SOCK_MQTT) {
        LOG_E("Wrong socket type, need MQTT socket\n");
        return -1;
    }

    if (socket->sd < 0) {
       if (__esp_connect(socket, RT_NULL, RT_NULL) != 0) {
           LOG_E("MQTT socket connect fail\n");
           return -1;
       }
    }
    for (res = 0; res < msg->number; res++) {
      
        topic = &msg->topics[res];
        topic->qos = 1;
        
        memset(pbuf,0,size);
        at_snprintf(pbuf, size, "AT+MQTTSUB=%d",0);
        curr_len = at_strlen(pbuf);

        at_snprintf(pbuf + curr_len, size - curr_len, ",\"%s\",%d\r\n",
                    topic->topic?topic->topic:"", topic->qos);
       
        if ((res = esp_cmd_line(pbuf, "OK", 0, 0, 8000, 1)) != AT_CMD_ACK_OK) {
            socket->fail_count++;
            LOG_E("Fail to exec at cmd: %d, %s\n", res, pbuf);
            return -1;
        }
        
    }
    socket->fail_count = 0;
    
    LOG_D("MQTT Subscribe SUCCESS");

    return 0;
}
/*
********************************************************************************
*Function    : __esp_mqtt_publish
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      : 

               cmd : AT+MQTTPUB=<LinkID>,<"topic">,<"data">,<qos>,<retain>   
*              ack : OK 

               每条 AT 命令的总长度不能超过 256 字节。
               本命令不能发送数据 \0，若需要发送该数据，请使用 AT+MQTTPUBRAW 命令。

               AT+MQTTPUBRAW=<LinkID>,<"topic">,<length>,<qos>,<retain>

********************************************************************************
*/
int __esp_mqtt_publish(socket_t *socket, mqtt_msg_t *msg)
{
    char atcmd[256]; 

    if (msg->topic == RT_NULL || msg->payload == RT_NULL || msg->payload_len == 0) {
        return -1;
    }

    if (socket->type != SOCK_MQTT) {
        LOG_E("Wrong socket type, need MQTT socket\n");
        return -1;
    }
    msg->msgid=1;
    msg->qos = 1;
    msg->retain = 0;

    /* 长数据 
     * AT+MQTTPUBRAW=<LinkID>,<"topic">,<length>,<qos>,<retain>
     */
    at_snprintf(atcmd, sizeof(atcmd),"AT+MQTTPUBRAW=%d,\"%s\",%d,%d,%d\r\n",socket->sd,msg->topic,msg->payload_len,msg->qos,msg->retain);
    atcmd[sizeof(atcmd)-1] = '\0';
    
    if (esp_cmd_line(atcmd, ">", 0, 0, 8000, 1) != AT_CMD_ACK_OK) {
        socket->fail_count++;
        LOG_E("Fail to exec at cmd: %s\n", atcmd);
        return -1;
    }
    //publish message
    if (esp_cmd_line_with_len(msg->payload, msg->payload_len, "+MQTTPUB:OK", 0, 0, 8000, 1) != AT_CMD_ACK_OK) {
        socket->fail_count++;
        LOG_E("Fail to detect: %s\n", "+MQTTPUB:FAIL");
        return -1;
    }

    LOG_D("MQTT Publish Success");

    socket->fail_count = 0;

    return 0;
}

/*
********************************************************************************
*Function    : __esp_mqtt_scan
*Description : 定时查询网络参数
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
static int __esp_mqtt_scan(void)
{     
    int res=-1;
    
    res = esp_cwstate_detect();
    if( res != 0 ){
      
        return -1;
    }
    
    res = esp_rfpower_detect();
    if( res != 0 ){
      
        return -1;
    }
    res = esp_sysram_detect();
    if( res != 0 ){
        
        return -1;
    }
    
    return 0;
}
/*
********************************************************************************
                                mqtt 对外接口函数
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : esp_mqtt_connect
*Description : mqtt 链接
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int esp_mqtt_connect(int sd, const char *peer_ip, unsigned short peer_port,
                     const char *clientid, const char *username, const char *password,
                     void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    connect_msg_t *connect_msg;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }

    if (peer_ip == RT_NULL || *peer_ip == '\0') {
        return -1;
    }

    if (clientid == RT_NULL) {
        return -1;
    }

    //socket not used
    if (atomic_read(&esp.sockets[sd].used) != 1) {
        return -1;
    }

    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(connect_msg_t));
    if (msg == RT_NULL) {
        return -1;
    }
    connect_msg = (connect_msg_t *)(msg->msg);
    at_memset(connect_msg, 0, sizeof(connect_msg_t));

    at_strncpy(connect_msg->remote_addr.addr.ip, peer_ip, sizeof(connect_msg->remote_addr.addr.ip)-1);
    connect_msg->remote_addr.addr.port = peer_port;
    at_strncpy(connect_msg->remote_addr.clientid, clientid, sizeof(connect_msg->remote_addr.clientid)-1);
    if (username) {
        at_strncpy(connect_msg->remote_addr.username, username, sizeof(connect_msg->remote_addr.username)-1);
    } else {
        connect_msg->remote_addr.username[0] = '\0';
    }
    if (password) {
        at_strncpy(connect_msg->remote_addr.password, password, sizeof(connect_msg->remote_addr.password)-1);
    } else {
        connect_msg->remote_addr.password[0] = '\0';
    }

    //put message into TX queue
    msg->sd = sd;
    msg->type = MSG_CONNECT;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : esp_mqtt_unsubscribe
*Description : esp mqtt 取消订阅函数
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int esp_mqtt_unsubscribe(int sd, int msgid, mqtt_topic_t *topics, int count, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    sub_msg_t *sub_msg;
    char *buf;
    int i, number;
    unsigned int len = 0;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }

    if (topics == RT_NULL) {
        return -1;
    }

    number = 0;
    for (i = 0; i < count; i++) {
        if (topics[i].topic) {
            len += (at_strlen(topics[i].topic) + 1);
            number++;
        }
    }
    if (number == 0) {
        return -1;
    }

    //socket not used
    if (atomic_read(&esp.sockets[sd].used) != 1) {
        return -1;
    }

    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(sub_msg_t) +
                number*sizeof(mqtt_topic_t) + len);
    if (msg == RT_NULL) {
        return -1;
    }
    sub_msg = (sub_msg_t *)(msg->msg);
    at_memset(sub_msg, 0, sizeof(sub_msg_t));

    sub_msg->msgid = msgid;
    sub_msg->topics = (mqtt_topic_t *)(sub_msg + 1);
    buf =  (char *)(sub_msg->topics + number);
    number = 0;
    for (i = 0; i < count; i++) {
        if (topics[i].topic) {
            strcpy(buf, topics[i].topic);
            sub_msg->topics[number].topic = buf;
            sub_msg->topics[number].qos = topics[i].qos;
            buf += (at_strlen(buf) + 1);
            number++;
        }
    }
    sub_msg->number = number;

    msg->sd = sd;
    msg->type = MSG_MQTT_UNS;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}

/*
********************************************************************************
*Function    : esp_mqtt_subscribe
*Description : esp mqtt 订阅函数
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int esp_mqtt_subscribe(int sd, int msgid, mqtt_topic_t *topics, int count, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    sub_msg_t *sub_msg;
    char *buf;
    int i, number;
    unsigned int len = 0;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }

    if (topics == RT_NULL) {
        return -1;
    }

    number = 0;
    for (i = 0; i < count; i++) {
        if (topics[i].topic) {
            len += (at_strlen(topics[i].topic) + 1);
            number++;
        }
    }
    if (number == 0) {
        return -1;
    }

    //socket not used
    if (atomic_read(&esp.sockets[sd].used) != 1) {
        return -1;
    }

    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(sub_msg_t) +
                number*sizeof(mqtt_topic_t) + len);
    if (msg == RT_NULL) {
        return -1;
    }
    sub_msg = (sub_msg_t *)(msg->msg);
    at_memset(sub_msg, 0, sizeof(sub_msg_t));

    sub_msg->msgid = msgid;
    sub_msg->topics = (mqtt_topic_t *)(sub_msg + 1);
    buf =  (char *)(sub_msg->topics + number);
    number = 0;
    for (i = 0; i < count; i++) {
        if (topics[i].topic) {
            strcpy(buf, topics[i].topic);
            sub_msg->topics[number].topic = buf;
            sub_msg->topics[number].qos = topics[i].qos;
            buf += (at_strlen(buf) + 1);
            number++;
        }
    }
    sub_msg->number = number;

    msg->sd = sd;
    msg->type = MSG_MQTT_SUB;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}

/*
********************************************************************************
*Function    : esp_mqtt_publish
*Description : esp mqtt 发布
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int esp_mqtt_publish(int sd, int msgid, int qos, int retain,
                      const char *topic,
                      const char *payload, int payload_len,
                      void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    mqtt_msg_t *mqtt_msg;
    int topic_len;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }

    if (topic == RT_NULL || payload == RT_NULL || payload_len == 0) {
        return -1;
    }

    //socket not used
    if (atomic_read(&esp.sockets[sd].used) != 1) {
        return -1;
    }

    topic_len = at_strlen(topic) + 1;
    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(mqtt_msg_t) + topic_len + payload_len);
    if (msg == RT_NULL) {
        return -1;
    }
    mqtt_msg = (mqtt_msg_t *)(msg->msg);
    at_memset(mqtt_msg, 0, sizeof(mqtt_msg_t));


    mqtt_msg->msgid = msgid;
    mqtt_msg->retain = retain;
    mqtt_msg->qos = qos;
    mqtt_msg->topic_len = topic_len;
    mqtt_msg->payload_len = payload_len;
    mqtt_msg->topic = (char *)(mqtt_msg + 1);
    mqtt_msg->payload = mqtt_msg->topic + topic_len;
    at_memcpy(mqtt_msg->topic,   topic,   topic_len);
    at_memcpy(mqtt_msg->payload, payload, payload_len);

    msg->sd = sd;
    msg->type = MSG_MQTT_PUB;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : esp_mqtt_init
*Description : esp mqtt初始化函数
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
int esp_mqtt_init(char *ssid,char *pwd)
{   
  
    esp_set_wifi(ssid,pwd);
    
    esp_set_function(__esp_wifi_sta_init,__esp_mqtt_scan,0);
    
    return 1;
}