/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __ESP_TCPIP_H                                                          
#define __ESP_TCPIP_H    

/*
********************************************************************************
********************************************************************************
*/
typedef struct {
    at_addr_t remote_addr;
    int len;
    char buf[0];
} udp_msg_t;

typedef struct {
    int len;
    char buf[0];
} tcp_msg_t;
/*
********************************************************************************
********************************************************************************
*/
int __esp_socket_open(int type, int connect_id, unsigned short local_port, at_addr_t *addr);

int __esp_socket_close(int sd);

int __esp_socket_recv(socket_t *socket, at_addr_t *addr, const char *buf, int len);

int __esp_tcp_send(socket_t *socket, tcp_msg_t *msg);

int __esp_udp_send(socket_t *socket, udp_msg_t *msg);
/*
********************************************************************************
********************************************************************************
*/
int esp_tcpip_init(char *ssid,char *pwd,int (*recv)(void *arg));

int esp_tcp_send(int sd, void *buf, at_size_t len, void(*res_cb)(int res, void *arg), void *arg);

int esp_tcp_connect(int sd, const char *peer_ip, unsigned short peer_port, void(*res_cb)(int res, void *arg), void *arg);

int esp_udp_send(int sd, const char *ip, unsigned short port, void *buf, at_size_t len, 
                 void(*res_cb)(int res, void *arg), void *arg);
#endif 
/* __ESP_TCPIP_H */
