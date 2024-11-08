/*
 * udp_client.c
 *
 *  Created on: 2023年08月26日
 *
 *      Author: lwp
 *  
 *        Note: 接口未调试
 */
#include <stdio.h>
#include <string.h>
#include <board.h>                                                     
#include <rtthread.h>          
#include "type.h"   
#include "at.h"
#include "at_client.h"
#include "esp.h"
#include "esp_ble.h"   
#include "esp_mqtt.h"
#include "PPItems.h"
#include "esp_tcpip.h"
#include "oam_thread.h" 
#include "cjson.h"
#include "smart_analy.h"

#define DBG_TAG "udp"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
typedef struct  {
    int sd;
    char url[64];
    unsigned short port;
} ATUDP_CLient;
 
/*
********************************************************************************
********************************************************************************
*/
static ATUDP_CLient UDPClient;

#define get_udp_client()   (&UDPClient)
/*
********************************************************************************
********************************************************************************
*/
int udp_send_frame(u8_t *buf, int ulen);
/*
********************************************************************************
********************************************************************************
*/
/*
******************************************************************************** 
*Function    : udp_is_connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
******************************************************************************** 
*/
int udp_is_connect(void)
{
    if (UDPClient.sd < 0) {
        return  0;
    }
    return 1;
}
/*
******************************************************************************** 
*Function    : at_udp_send_lowlayer
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
******************************************************************************** 
*/
int udp_send_lowlayer(u8_t *msg, u32_t msglen, void *arg)
{
    (void)arg;
 
    return udp_send_frame(msg, msglen);
} 

/*
********************************************************************************
*Function    : udp_msg_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void udp_msg_process(void *data, int data_len)
{
    //lan_msg_analy((u8_t *)data, data_len, at_udp_send_lowlayer, 0);
}
/*
********************************************************************************
*Function    : udp_recv_frame
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void udp_recv_frame(int sd, char *buf, int len)
{
	oam_thread_post(udp_msg_process, buf, len);
}
/*
********************************************************************************
*Function    : connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int connect(ATUDP_CLient *client)
{
    if (client->sd < 0) {
        client->sd = esp_socket_create(SOCK_DGRAM);
        esp_set_recv(client->sd, udp_recv_frame);
    }


    if (client->sd < 0) {
        return -1;
    }

    return 0;
}
/*
********************************************************************************
*Function    : udp_send_frame
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int udp_send_frame(u8_t *buf, int ulen)
{
    ATUDP_CLient *client = get_udp_client();
#if 0
    char ip[64];
    unsigned short port;
#endif
    if (connect(client) != 0) {
        return -1;
    }

    //get IP and port
#if 0
    strncpy(ip, "121.204.204.8", sizeof(ip));
    ip[sizeof(ip)-1] = '\0';
    port = 8070;
#endif
 
    if (esp_udp_send(client->sd, client->url, client->port, buf, ulen, 0, 0) != 0) {
        return -1;
    }

    return ulen;
}
/*
********************************************************************************
*Function    : udp_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int udp_init(void)
{
	ATUDP_CLient *client = get_udp_client();

    memset(client, 0, sizeof(ATUDP_CLient));
    
//    PPItemRead( PP_ROOT_URL, (u8_t*)&client->url,64); 
 
  //  PPItemRead( PP_ROOT_PORT, (u8_t*)&client->port, 2); 
 
    client->sd = -1;
 
    return 0;
}

