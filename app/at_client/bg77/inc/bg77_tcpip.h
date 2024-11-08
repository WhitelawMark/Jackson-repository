 /*
 *  bg77_tcpip.h
 *
 *  Created on: 2022年08月08日
 *      Author: root
 */
#ifndef _BG77_TCPIP_H_
#define _BG77_TCPIP_H_
 
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


typedef struct {
    atomic_t used;
    int connect_id;
    int fail_count;
    int index;
    int type;
    int sd;
    unsigned short local_port;
    sockaddr_t remote_addr;
    at_mq_t rx_mq;
    void (*notify)(int evtype);
    void (*recv)(int sd, char *buf, int len);
} socket_t;
    


/*
********************************************************************************
********************************************************************************
*/
int __bg77_socket_recv(socket_t *socket, at_addr_t *addr, const char *buf, int len);
int __bg77_socket_open(int type, int connect_id, unsigned short local_port, at_addr_t *addr);
int __bg77_socket_close(int sd);

int __bg77_udp_send(socket_t *socket, udp_msg_t *msg);
int __bg77_tcp_send(socket_t *socket, tcp_msg_t *msg);
/*
********************************************************************************
********************************************************************************
*/
//tcp
int bg77_tcp_connect(int sd, const char *peer_ip, unsigned short peer_port, void(*res_cb)(int res, void *arg), void *arg);
int bg77_tcp_send(int sd, void *buf, at_size_t len, void(*res_cb)(int res, void *arg), void *arg);
at_ssize_t bg77_tcp_recv(int sd, void *buf, at_size_t len);


//udp
int bg77_udp_send(int sd, const char *ip, unsigned short port, void *buf, at_size_t len, void(*res_cb)(int res, void *arg), void *arg);
at_ssize_t bg77_udp_recv(int sd, at_addr_t *ipaddr, void *buf, at_size_t len);

#endif /* _BG77_TCPIP_H_ */

