/*
 *  esp_tcpip.c
 *
 *  Created on: 2023年08月24日
 *
 *      Author: lwp
 *
 *       Note ：ESP32-TCPIP(UDP/TCP)该功能未开始调试，以下的指令并未校验过。
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <drivers/pin.h>
#include "type.h"
#include "ustring.h"
#include "at.h"
#include "at_client.h"
#include "esp.h"
#include "esp_tcpip.h"



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
*Function    : __esp_socket_recv
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int __esp_socket_recv(socket_t *socket, at_addr_t *addr, const char *buf, int len)
{
    void (*recv)(int sd, char *buf, int len);
    int ret = -1;

    if (socket->type == SOCK_STREAM) {
        tcp_msg_t *tcp_msg = at_message_alloc(sizeof(tcp_msg_t) + len);
        if (tcp_msg == RT_NULL) {
            return -1;
        }
        at_message_type(tcp_msg) = SOCK_STREAM;
        tcp_msg->len = len;
        at_memcpy(tcp_msg->buf, buf, len);

        //put message
        recv = (void (*)(int, char*, int))__atomic_read((volatile long *)&socket->recv);
        if (recv) {
            recv(socket->index, tcp_msg->buf, tcp_msg->len);
            at_message_free(tcp_msg);
        } else {
            at_message_put(&socket->rx_mq, tcp_msg, socket->notify, EV_RX);
        }
        ret = 0;
    }
    else if (socket->type == SOCK_DGRAM) {
        udp_msg_t *udp_msg = at_message_alloc(sizeof(udp_msg_t) + len);
        if (udp_msg == RT_NULL) {
            return -1;
        }
        at_message_type(udp_msg) = SOCK_DGRAM;
        at_memcpy(&udp_msg->remote_addr, addr, sizeof(udp_msg->remote_addr));
        udp_msg->len = len;
        at_memcpy(udp_msg->buf, buf, len);

        //put message
        recv = (void (*)(int, char*,int))__atomic_read((volatile long *)&socket->recv);
        if (recv) {
            recv(socket->index, udp_msg->buf, udp_msg->len);
            at_message_free(udp_msg);
        } else {
            at_message_put(&socket->rx_mq, udp_msg, socket->notify, EV_RX);
        }
        ret = 0;
    }

    return ret;
}

/*
********************************************************************************
*Function    : __esp_socket_close
*Description :
*Input       :
*Output      :
*Return      : 0 -- if success, -1 -- if fail.
*Others      :
********************************************************************************
*/
int __esp_socket_close(int sd)
{
    char atcmd[32];

    at_snprintf(atcmd, sizeof(atcmd), "AT+CIPCLOSE\r\n");
    atcmd[sizeof(atcmd)-1] = '\0';

    esp_cmd_line(atcmd, "OK", 0, 0, 2000, 1);

    return 0;
}

/*
********************************************************************************
*Function    : __esp_socket_open
*Description :
*Input       :
*Output      :
*Return      : >=0 -- socket index, < 0 -- if fail.
*Others      : AT+CIPSTART="TCP","192.168.3.102",8080
********************************************************************************
*/
int __esp_socket_open(int type, int connect_id, unsigned short local_port, at_addr_t *addr)
{
    char *str;
    int res;
    char atcmd[64];
    char xbuf[20];

    if (type == SOCK_STREAM) {
        if (addr->ip[0] == '\0') {
            return -2;
        }
        at_snprintf(atcmd, sizeof(atcmd), " AT+CIPSTART=\"TCP\",\"%s\",%u,%u,%u\r\n",
                "192.168.3.102",8080,1112,0);
    } else {
        at_snprintf(atcmd, sizeof(atcmd), " AT+CIPSTART=\"UDP\",\"%s\",%u,%u,%u\r\n",
                "192.168.3.102",8080,1112,0);
    }
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';

    if (esp_cmd_line(atcmd, "+QIOPEN:", xbuf, sizeof(xbuf), 5000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    str = xbuf;

    /*+QIOPEN: <connectID>,<err>*/
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    //result
    res = atoi(str);
    if (res != 0) {
        return -1;
    }

    return connect_id;
}
/*
********************************************************************************
*Function    : __esp_udp_send
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int __esp_udp_send(socket_t *socket, udp_msg_t *msg)
{
    char atcmd[64];

    if (msg->len <= 0) {
        return -1;
    }

    if (socket->type != SOCK_DGRAM) {
        LOG_E("Wrong socket type, need UDP socket\n");
        return -1;
    }

    if (socket->sd < 0) {
        if (__esp_connect(socket, RT_NULL, RT_NULL) != 0) {
            LOG_E("UDP socket open fail\n");
            return -1;
        }
    }

    //AT+QISEND=<connectID>,<send_length>,"<remoteIP>",<remote_port>
    at_snprintf(atcmd, sizeof(atcmd), "AT+QISEND=%d,%d,\"%s\",%u\r",
            socket->sd, msg->len, msg->remote_addr.ip, msg->remote_addr.port);
    atcmd[sizeof(atcmd)-1] = '\0';

    if (esp_cmd_line(atcmd, "\r\n>", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        socket->fail_count++;
        LOG_E("Fail to exec at cmd: %s\n", atcmd);
        return -1;
    }

    if (esp_cmd_line_with_len(msg->buf, msg->len, "SEND OK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        socket->fail_count++;
        LOG_E("UDP send fail\n");
        return -1;
    }

    socket->fail_count = 0;

    return 0;
}
/*
********************************************************************************
*Function    : __esp_tcp_send
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int __esp_tcp_send(socket_t *socket, tcp_msg_t *msg)
{
    char atcmd[32];

    if (msg->len <= 0) {
        return -1;
    }

    if (socket->type != SOCK_STREAM) {
        LOG_E("Wrong socket type, need TCP socket\n");
        return -1;
    }

    if (socket->sd < 0) {
        if (__esp_connect(socket, RT_NULL, RT_NULL) != 0) {
            LOG_E("TCP socket connect fail\n");
            return -1;
        }
    }

    //AT+QISEND=<connectID>,<send_length>
    at_snprintf(atcmd, sizeof(atcmd), "AT+QISEND=%d,%d\r", socket->sd, msg->len);
    atcmd[sizeof(atcmd)-1] = '\0';

    if (esp_cmd_line(atcmd, "\r\n>", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        socket->fail_count++;
        LOG_E("Fail to exec at cmd: %s\n", atcmd);
        return -1;
    }

    if (esp_cmd_line_with_len(msg->buf, msg->len, "SEND OK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        socket->fail_count++;
        LOG_E("TCP send fail\n");
        return -1;
    }

    socket->fail_count = 0;

    return 0;
}
/*
********************************************************************************
*Function    : __esp_tcpip_open
*Description : 
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
int __esp_tcpip_init(void)
{    
    int res=-1;
    
    esp_cwmode_config(ESP_CWMODE_STA);
    
    res = esp_cwlap_detect(esp.wifista.ssid);
    if( res != 0 ){
        LOG_E("The SSID signal for WIFI could not be found\n");
        return -1;
    }
    res = esp_cwap_config(esp.wifista.ssid,esp.wifista.pwd);
    if( res != 0 ){
        LOG_E("wifi Network connection failure\n");
        return -1;
    }
    LOG_D("wifi connection successful\n");
    
    res = esp_cipsta_detect();
    if( res != 0 ){
        return -1;
    }
    return 0;
}
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    : esp_udp_send
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 fail.
*Others      :
********************************************************************************
*/
int esp_udp_send(int sd, const char *ip, unsigned short port, void *buf, at_size_t len, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    udp_msg_t *udp_msg;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }

    if (ip == RT_NULL || *ip == '\0' || len == 0) {
        return -1;
    }

    //socket not used
    if (atomic_read(&esp.sockets[sd].used) != 1) {
        return -1;
    }

    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(udp_msg_t) + len);
    if (msg == RT_NULL) {
        return -1;
    }
    udp_msg = (udp_msg_t *)(msg->msg);
    at_memset(udp_msg, 0, sizeof(udp_msg_t));

    at_strncpy(udp_msg->remote_addr.ip, ip, sizeof(udp_msg->remote_addr.ip));
    udp_msg->remote_addr.ip[sizeof(udp_msg->remote_addr.ip)-1] = '\0';
    udp_msg->remote_addr.port = port;
    at_memcpy(udp_msg->buf, buf, len);
    udp_msg->len = len;

    msg->sd = sd;
    msg->type = MSG_UDP;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : esp_tcp_send
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 fail.
*Others      :
********************************************************************************
*/
int esp_tcp_send(int sd, void *buf, at_size_t len, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    tcp_msg_t *tcp_msg;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }

    if (len == 0) {
        return -1;
    }

    //socket not used
    if (atomic_read(&esp.sockets[sd].used) != 1) {
        return -1;
    }

    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(tcp_msg_t) + len);
    if (msg == RT_NULL) {
        return -1;
    }
    tcp_msg = (tcp_msg_t *)(msg->msg);
    at_memset(tcp_msg, 0, sizeof(tcp_msg_t));

    at_memcpy(tcp_msg->buf, buf, len);
    tcp_msg->len = len;

    msg->sd = sd;
    msg->type = MSG_TCP;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : esp_tcp_connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int esp_tcp_connect(int sd, const char *peer_ip, unsigned short peer_port, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    connect_msg_t *connect_msg;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }

    if (peer_ip == RT_NULL || *peer_ip == '\0') {
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

    //put message into TX queue
    msg->sd = sd;
    msg->type = MSG_CONNECT;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : esp_tcpip_init
*Description : 
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
int esp_tcpip_init(char *ssid,char *pwd,int (*recv)(void *arg))
{   
    esp_set_wifi(ssid,pwd);
    
    esp_set_function(__esp_wifi_sta_init,0,0);
    
    return 1;
}