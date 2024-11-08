/*
 * tcp_client.c
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
#include "ble_service.h" 
#include "oam_thread.h" 
#include "at.h"
#include "at_client.h"
#include "esp.h"
#include "esp_ble.h"   
#include "esp_mqtt.h"
#include "PPItems.h"
#include "esp_tcpip.h"

#include "cjson.h"
#include "smart_analy.h"

#define DBG_TAG "tcpip"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/ 
 
typedef struct  {
    int sd;
    int connect;
    char url[64];
    unsigned short port;
} ATTCP_CLient;
/*
********************************************************************************
********************************************************************************
*/ 
   
static u8_t RxBuf[1024];  
static ATTCP_CLient TCPClient; 
#define get_tcp_client()   (&TCPClient)       
/*
********************************************************************************
********************************************************************************
*/ 
static int tcp_send(const char *pmsg, int ulen);
/*                                                                        
********************************************************************************
*Function    : tcp_send_lowlayer                                      
*Description :                                                            
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
static int tcp_send_lowlayer(const char *msg, int msglen, void *arg)
{
    (void)arg;

    return tcp_send(msg, msglen);
}
/*
********************************************************************************
*Function    : tcp_msg_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void tcp_msg_process(void *data, int data_len)
{
    cjson_analy(0,data,data_len,tcp_send_lowlayer,0); 
}
/*
********************************************************************************
*Function    : tcp_recv_frame
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void tcp_recv_frame(int sd, char *buf, int len)
{
    memcpy(RxBuf,buf,len);
    oam_thread_post(tcp_msg_process, RxBuf, len);
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
static int connect(ATTCP_CLient *client)
{
    if (client->sd < 0) {
        client->sd = esp_socket_create(SOCK_STREAM);
        esp_set_recv(client->sd, tcp_recv_frame);
        client->connect = 0;
    }

    if (client->sd >= 0 && client->connect == 0) {
        if (esp_tcp_connect(client->sd, client->url, client->port, 0, 0) == 0) {
            client->connect = 1;
        }
    }

    if (client->connect == 0) {
        return -1;
    }

    return 0;
}
/*
********************************************************************************
*Function    : disconnect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
void tcpip_disconnect(void)
{
    ATTCP_CLient *client = get_tcp_client();
    if (client->sd >= 0) {
        esp_socket_delete(client->sd);
        client->sd = -1;
    }
}
/*
********************************************************************************
*Function    : tcp_send
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int tcp_send(const char *pmsg, int ulen)
{
    ATTCP_CLient *client = get_tcp_client();
    
    if (connect(client) != 0) {
        return -1;
    }
   // LOG_HEX("tcp-send",16,(u8_t *)buf,ulen);
    return esp_tcp_send(client->sd, (void *)pmsg, ulen, 0, 0);
}
/*
********************************************************************************
*Function    :  tcp_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 无线的域名不能加tcp://
********************************************************************************
*/
int tcp_init(void)
{
	ATTCP_CLient *client = get_tcp_client();

    memset(client, 0, sizeof(ATTCP_CLient));

    snprintf(client->url,64,"121.41.42.58");
    client->port = 19019; 

    client->sd = -1;
    
    return 0;
}