/*
 *  bg77_ftp.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */

#include <stdio.h>
#include <string.h>
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <drivers/pin.h>
#include "app_lib.h"  
#include "at_client.h"
#include "bg77.h"
#include "bg77_mqtt.h"


#define DBG_TAG "bg77"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
extern at_manage_t bg77;
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
*Function    : bg77_qmt_open
*Description :
*Input       :
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      :
             AT+QMTCFG="recv/mode",<client_idx>,<msg_recv_mode>[,<msg_len_enable>]
             <msg_recv_mode> Integer type. Configure the MQTT message receiving mode.
                             0 MQTT message received from server will be contained in URC.
                             1 MQTT message received from server will not be contained in URC.
             <msg_len_enable>
                             0 Length of MQTT message received from server will not be contained in URC.
                             1 Length of MQTT message received from server will be contained in URC

             AT+QMTOPEN=<client_idx>,"<host_name>",<port>
             +QMTOPEN: <client_idx>,<result>
              <result> Integer type. Result of the command execution
                       -1 Failed to open network
                        0 Network opened successfully
                        1 Wrong parameter
                        2 MQTT identifier is occupied
                        3 Failed to activate PDP
                        4 Failed to parse domain name
                        5 Network disconnection error
********************************************************************************
*/
int bg77_qmt_open(int sd, at_addr_t *addr)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20];
    
    //MQTT message received from server will be contained in URC.
    // AT+QMTCFG="ssl",0,1,2\r
    at_snprintf(atcmd, sizeof(atcmd), "AT+QMTCFG=\"ssl\",%d,1,2\r", 0);
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"cacert\",2,\"cacert.pem\"\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"clientcert\",2,\"client.pem\"\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"clientkey\",2,\"user_key.pem\"\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"seclevel\",2,2\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"sslversion\",2,4\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"ciphersuite\",2,0XFFFF\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    //AT+QMTOPEN=0,"a1qc8sz6uwatlh-ats.iot.ap-northeast-2.amazonaws.com",8883\r
    at_snprintf(atcmd, sizeof(atcmd), "AT+QMTOPEN=%d,\"%s\",%u\r", sd, addr->ip, (unsigned int)(addr->port));
    //at_snprintf(atcmd, sizeof(atcmd), "AT+QMTOPEN=%d,\"%s\",%u\r", sd, "a1qc8sz6uwatlh-ats.iot.us-east-1.amazonaws.com", 8883);
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "+QMTOPEN:", xbuf, sizeof(xbuf), 30000, 1) != AT_CMD_DATA_GET_OK) {
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
*Function    : bg77_qmt_close
*Description :
*Input       :
*Output      :
*Return      : 0   -- if success, -1 -- if fail.
*Others      : AT+QMTCLOSE=<client_idx>
               \r\r\nOK
               +QMTCLOSE: <client_idx>,<result>
               <result> Integer type. Result of the command execution
                       -1 Failed to close network
                        0 Network closed successfully
               here we detect "\r\nOK",  +QMTCLOSE: need wait 2 minute
********************************************************************************
*/
int __bg77_qmt_close(int sd)
{
    char atcmd[32];

    at_snprintf(atcmd, sizeof(atcmd), "AT+QMTCLOSE=%d\r", sd);
    atcmd[sizeof(atcmd)-1] = '\0';

    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 5000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }

    return 0;
}
/*
********************************************************************************
*Function    : bg77_qmt_disc
*Description :
*Input       :
*Output      :
*Return      : 0   -- if success, -1 -- if fail.
*Others      : AT+QMTDISC=<client_idx>
               \r\nOK
               +QMTDISC: <client_idx>,<result>
               <result> Integer type. Result of the command execution
                        -1 Failed to close connection
                         0 Connection closed successfully
               here we detect "\r\nOK"
********************************************************************************
*/
int bg77_qmt_disc(int sd)
{
    char atcmd[32];

    at_snprintf(atcmd, sizeof(atcmd), "AT+QMTDISC=%d\r", sd);
    atcmd[sizeof(atcmd)-1] = '\0';

    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }

    return 0;
}

/*
********************************************************************************
*Function    : bg77_qmt_conn
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      : AT+QMTCONN=<client_idx>,"<clientID>"[,"<username>"[,"<password>"]]
               +QMTCONN: <client_idx>,<result>[,<ret_code>]\r\n
               <result> Integer type. Result of the command execution
                        0 Packet sent successfully and ACK received from server
                        1 Packet retransmission
                        2 Failed to send packet
********************************************************************************
*/
static int bg77_qmt_conn(int sd, const char *clientid, const char *username, const char *password)
{
    char *str;
    int res;
    char atcmd[100];
    char xbuf[20];

    if (username && *username) {
        if (password && *password) {
           // at_snprintf(atcmd, sizeof(atcmd), "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"\r", sd, clientid, username, password);
            at_snprintf(atcmd, sizeof(atcmd), "AT+QMTCONN=%d,\"%s\"\r", sd, clientid);
        } else {
            at_snprintf(atcmd, sizeof(atcmd), "AT+QMTCONN=%d,\"%s\",\"%s\"\r", sd, clientid, username);
        }
    } else {
        at_snprintf(atcmd, sizeof(atcmd), "AT+QMTCONN=%d,\"%s\"\r", sd, clientid);
    }
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "+QMTCONN:", xbuf, sizeof(xbuf), 30000, 1) != AT_CMD_DATA_GET_OK) {
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

    return 0;
}

/*
********************************************************************************
*Function    : bg77_mqtt_open
*Description :
*Input       :
*Output      :
*Return      : socket
*Others      :
********************************************************************************
*/
int bg77_mqtt_open(int connect_id, at_addr_t *addr, const char *clientid, const char *username, const char *password)
{
    int sd;

    if (addr->ip[0] == '\0' || clientid == RT_NULL) {
        return -2;
    }

    sd = bg77_qmt_open(connect_id, addr);
    if (sd < 0) {
        LOG_E("MQTT socket (peer addr %s:%u) open fail\n", addr->ip, (unsigned int)(addr->port));
        sd = -1;
    } else {
        if (bg77_qmt_conn(sd, clientid, username, password) != 0) {
            LOG_E("MQTT socket (peer addr %s:%u, clientid: %s, username: %s, password: %s) connect fail\n",
                    addr->ip, (unsigned int)(addr->port), clientid, username?username:"", password?password:"");
            __bg77_qmt_close(sd);
            sd = -1;
        }
    }

    return sd;
}
/*
********************************************************************************
*Function    : __bg77_mqtt_receive
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int __bg77_mqtt_receive(socket_t *socket, int msgid, const char *topic, int topic_len, int payload_len)
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

    //payload
    payload_len = bg77_cmd_line_get(0, mqtt_msg->payload, payload_len, 2000);
    if (payload_len == 0) {
        at_message_free(mqtt_msg);
    } else {
        mqtt_msg->payload_len = payload_len;
        LOG_E("%s\n",mqtt_msg->payload);
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
*Function    : __bg77_urc_qmtrecv
*Description :
*Input       :
*Output      :
*Return      :
*Others      : +QMTRECV: <client_idx>,<msgID>,"<topic>",[<payload_len>],”<payload>”
               +QMTRECV: 1,0,"ArcDetector/1234562/Reply/Alarms",8,"12345678"
********************************************************************************
*/
void __bg77_urc_qmtrecv(const char *data, int len)
{
    char *str = (char *)data;
    //mqtt_msg_t *mqtt_msg;
    socket_t *socket;
    int sd;
    int msgid;
    int topic_len;
    const char *topic;
    char suffix[5];
    char buf[10];

    /*client_idx*/
    sd = at_atoi(str);
    if (sd < 0) {
        return;
    }
    
    /*skip client_idx*/
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','

    //msgID. The message identifier of packet
    msgid = at_atoi(str);
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    //topic
    while (at_isspace(*str)) str++;
    if (*str == '\"') str++; //skip '"'
    topic = str;
    topic_len = at_strlen(str);
#if 0
    at_strncpy(suffix, ",\"", sizeof(suffix));
    suffix[sizeof(suffix)-1] = '\0';
    bg77_cmd_line_get(suffix, buf, sizeof(buf), 2000);
    if (suffix[0] == '\0') {
        len = at_atoi(buf);
        if (len > 0) {
            socket = bg77_get_socket(sd, SOCK_MQTT);
            if (socket != RT_NULL) {
                __bg77_mqtt_receive(socket, msgid, topic, topic_len, len);
            }
        }
    }
#else
    
     socket = bg77_get_socket(sd, SOCK_MQTT);
     if (socket != RT_NULL) {
          __bg77_mqtt_receive(socket, msgid, topic, topic_len, 1024);
     }
    
#endif
}

/*
***************************************************************************************************
*Function    : __bg77_urc_qmtstat
*Description :
*Input       :
*Output      :
*Return      :
*Others      : +QMTSTAT: <client_idx>,<err_code>
               <err_code> An error code. Please refer to the table below for details
                          1 Connection is closed or reset bypeer.
                          2 Sending PINGREQ packettimed out or failed
                          3 Sending CONNECT packet timed out or failed.
                          4 Receiving CONNECK packet timed out or failed
                          5 The client sends DISCONNECT packet to sever and
                            the server is initiative to close MQTT connection
                          6 The client is initiative to close MQTT connection
                            due to packet sending failure all the time
                          7 The link is not alive or the server is unavailable
                          8-255 Reserved for future use
***************************************************************************************************
*/
void __bg77_urc_qmtstat(const char *data, int len)
{
    char *str = (char *)data;
    socket_t *socket;
    int sd;
    int res;

    (void)len;

   //client_idx
    sd = at_atoi(str);
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    //err_code
    res = at_atoi(str);
    if (res == 1 || res == 2 || res == 3 || res == 4 || res == 6 || res == 7) {
        socket = bg77_get_socket(sd, SOCK_MQTT);
        if (socket != RT_NULL) {
            //for reconnect
            LOG_E("peer connect closed\n");
            __bg77_close(socket);
        }
    }
}
/*
********************************************************************************
*Function    : __bg77_mqtt_unsubscribe
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      : AT+QMTUNS=<client_idx>,<msgID>,"<topic1>"[,"<topic2>"…]
               +QMTUNS: <client_idx>,<msgID>,<result>
               <result> Integer type. Result of the command execution
                        0 Sent packet successfully and received ACK from server
                        1 Packet retransmission
                        2 Failed to send packet
********************************************************************************
*/
int __bg77_mqtt_unsubscribe(socket_t *socket, sub_msg_t *msg)
{
    mqtt_topic_t *topic;
    char *str;
    char *pbuf = bg77.tmpbuf;
    int size = sizeof(bg77.tmpbuf)-2;
    int res, curr_len;
    char xbuf[20];

    if (msg->topics == RT_NULL || msg->number <= 0) {
        return -1;
    }

    if (socket->type != SOCK_MQTT) {
        LOG_E("Wrong socket type, need MQTT socket\n");
        return -1;
    }

    if (socket->sd < 0) {
       if (__bg77_connect(socket, RT_NULL, RT_NULL) != 0) {
           LOG_E("MQTT socket connect fail\n");
           return -1;
       }
    }

    at_snprintf(pbuf, size, "AT+QMTUNS=%d,%d", socket->sd, msg->msgid);
    pbuf[size] = '\0';

    curr_len = at_strlen(pbuf);
    for (res = 0; res < msg->number; res++) {
        topic = &msg->topics[res];
        at_snprintf(pbuf + curr_len, size - curr_len, ",\"%s\"", topic->topic?topic->topic:"");
        curr_len = at_strlen(pbuf);
    }
    pbuf[curr_len++] = '\r';
    pbuf[curr_len] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if ((res = bg77_cmd_line(pbuf, "+QMTUNS:", xbuf, sizeof(xbuf), 8000, 1)) != AT_CMD_DATA_GET_OK) {
        socket->fail_count++;
        LOG_E("Fail to exec at cmd: %d, %s\n", res, pbuf);
        return -1;
    }
    str = xbuf;

    //client_idx
    while (*str && *str != ',') str++;
    if (*str) str++;   //skip ','

    //msgID
    while (*str && *str != ',') str++;
    if (*str) str++;   //skip ','

    //result
    res = at_atoi(str);
    if (res != 0) {
        socket->fail_count++;
        LOG_E("cmd (result = %d) exec fail(%s)\n", res, pbuf);
        return -1;
    }

    socket->fail_count = 0;


    return 0;
}

/*
********************************************************************************
*Function    : __bg77_mqtt_subscribe
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      : AT+QMTSUB=<client_idx>,<msgID>,"<topic1>",<qos1>[,"<topic2>",<qos2>…]
               +QMTSUB: <client_idx>,<msgID>,<result>[,<value>,<value2>…]
               <result> Integer type. Result of the command execution
                        0 Sent packet successfully and received ACK from server
                        1 Packet retransmission
                        2 Failed to send packet
********************************************************************************
*/
int __bg77_mqtt_subscribe(socket_t *socket, sub_msg_t *msg)
{
    mqtt_topic_t *topic;
    char *str;
    char *pbuf = bg77.tmpbuf;
    int size = sizeof(bg77.tmpbuf)-2;
    int res, curr_len;
    char xbuf[20];
    
    LOG_E("Sub process ....\n");

    if (msg->topics == RT_NULL || msg->number <= 0) {
        return -1;
    }

    if (socket->type != SOCK_MQTT) {
        LOG_E("Wrong socket type, need MQTT socket\n");
        return -1;
    }

    if (socket->sd < 0) {
       if (__bg77_connect(socket, RT_NULL, RT_NULL) != 0) {
           LOG_E("MQTT socket connect fail\n");
           return -1;
       }
    }

    at_snprintf(pbuf, size, "AT+QMTSUB=%d,%d", socket->sd, msg->msgid);
    pbuf[size] = '\0';
    
    curr_len = at_strlen(pbuf);
    for (res = 0; res < msg->number; res++) {
        topic = &msg->topics[res];
        topic->qos=0;
        at_snprintf(pbuf + curr_len, size - curr_len, ",\"%s\",%d",
                topic->topic?topic->topic:"", topic->qos);
        curr_len = at_strlen(pbuf);
    }
    pbuf[curr_len++] = '\r';
    pbuf[curr_len] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';

    if ((res = bg77_cmd_line(pbuf, "+QMTSUB:", xbuf, sizeof(xbuf), 8000, 1)) != AT_CMD_DATA_GET_OK) {
        socket->fail_count++;
        LOG_E("Fail to exec at cmd: %d, %s\n", res, pbuf);
        return -1;
    }
    LOG_D("sub: %s\n",pbuf);
    str = xbuf;

    //client_idx
    while (*str && *str != ',') str++;
    if (*str) str++;   //skip ','

    //msgID
    while (*str && *str != ',') str++;
    if (*str) str++;   //skip ','

    //result
    res = at_atoi(str);
    if (res != 0) {
        socket->fail_count++;
        LOG_E("cmd (result = %d) exec fail(%s)\n", res, pbuf);
        return -1;
    }

    socket->fail_count = 0;
    
    //LOG_E("Sub SUCCESS\n");

    return 0;
}

/*
********************************************************************************
*Function    : __bg77_mqtt_publish
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      : AT+QMTPUBEX=<client_idx>,<msgID>,<qos>,<retain>,"<topic>",<msg_length>
               +QMTPUBEX: <client_idx>,<msgID>,<result>[,<value>]
               <result> Integer type. Result of the command execution
                        0 Packet sent successfully and ACK received from server (message that
                          published when <qos>=0 does not require ACK)
                        1 Packet retransmission
                        2 Failed to send packet
********************************************************************************
*/
int __bg77_mqtt_publish(socket_t *socket, mqtt_msg_t *msg)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20];

    if (msg->topic == RT_NULL || msg->payload == RT_NULL || msg->payload_len == 0) {
        return -1;
    }

    if (socket->type != SOCK_MQTT) {
        LOG_E("Wrong socket type, need MQTT socket\n");
        return -1;
    }

    if (socket->sd < 0) {
        if (__bg77_connect(socket, RT_NULL, socket->notify) != 0) {
            LOG_E("MQTT socket connect fail\n");
            return -1;
        }
    }
    msg->msgid=0;
    msg->qos=0;
    at_snprintf(atcmd, sizeof(atcmd),"AT+QMTPUB=%d,%d,%d,%d,\"%s\",%d\r",
            socket->sd, msg->msgid, msg->qos, msg->retain, msg->topic, msg->payload_len);
    atcmd[sizeof(atcmd)-1] = '\0';

    if (bg77_cmd_line(atcmd, "\r\n>", 0, 0, 5000, 1) != AT_CMD_ACK_OK) {
        socket->fail_count++;
        LOG_E("Fail to exec at cmd: %s\n", atcmd);
        return -1;
    }
    
    //publish message
    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line_with_len(msg->payload, msg->payload_len, "+QMTPUB:", xbuf, sizeof(xbuf), 4000, 1) != AT_CMD_DATA_GET_OK) {
        socket->fail_count++;
        LOG_E("Fail to detect: %s\n", "+QMTPUB:");
        return -1;
    }
    str = xbuf;
    
    //client_idx
    while (*str && *str != ',') str++;
    if (*str) str++;   //skip ','

    //msgID
    while (*str && *str != ',') str++;
    if (*str) str++;   //skip ','

    //result
    res = at_atoi(str);
    if (res != 0) {
        socket->fail_count++;
        LOG_E("cmd (result = %d) exec fail\n", res);
        return -1;
    }
    
    //LOG_E("publish success\n", res);

    socket->fail_count = 0;


    return 0;
}

/*
********************************************************************************
*Function    : bg77_mqtt_connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_mqtt_connect(int sd, const char *peer_ip, unsigned short peer_port,
                     const char *clientid, const char *username, const char *password,
                     void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    connect_msg_t *connect_msg;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }

    if (peer_ip == RT_NULL || *peer_ip == '\0') {
        return -1;
    }

    if (clientid == RT_NULL) {
        return -1;
    }

    //socket not used
    if (atomic_read(&bg77.sockets[sd].used) != 1) {
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

    return __bg77_socket_msg_post(msg);
}

/*
********************************************************************************
*Function    : bg77_mqtt_unsubscribe
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_mqtt_unsubscribe(int sd, int msgid, mqtt_topic_t *topics, int count, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    sub_msg_t *sub_msg;
    char *buf;
    int i, number;
    unsigned int len = 0;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
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
    if (atomic_read(&bg77.sockets[sd].used) != 1) {
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

    return __bg77_socket_msg_post(msg);
}

/*
********************************************************************************
*Function    : bg77_mqtt_subscribe
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_mqtt_subscribe(int sd, int msgid, mqtt_topic_t *topics, int count, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    sub_msg_t *sub_msg;
    char *buf;
    int i, number;
    unsigned int len = 0;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
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
    if (atomic_read(&bg77.sockets[sd].used) != 1) {
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

    return __bg77_socket_msg_post(msg);
}

/*
********************************************************************************
*Function    : bg77_mqtt_publish
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_mqtt_publish(int sd, int msgid, int qos, int retain,
                      const char *topic,
                      const char *payload, int payload_len,
                      void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    mqtt_msg_t *mqtt_msg;
    int topic_len;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }

    if (topic == RT_NULL || payload == RT_NULL || payload_len == 0) {
        return -1;
    }

    //socket not used
    if (atomic_read(&bg77.sockets[sd].used) != 1) {
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

    return __bg77_socket_msg_post(msg);
}
 