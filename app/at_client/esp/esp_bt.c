/*
 *  esp_ble.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
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


#define DBG_TAG "esp"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
extern esp_at_t esp;

static int __esp_bleadv_start(void);
/*
********************************************************************************
                            内部功能函数
********************************************************************************
*/
/*
********************************************************************************
*Function    : esp_bleinit_config
*Description : ESP 配置初始化
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功-1  -- 配置初始化失败
*Others      : AT+BLEINIT=%d
********************************************************************************
*/
static int esp_bleinit_config(int mode)
{
    char cmdbuf[128] = {'\r', '\n', 'O', 'K', '\0'}; 
   
    sprintf(cmdbuf,"AT+BLEINIT=%d\r\n",mode);
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return 0;
    }
    
    return 0;
}
/*
********************************************************************************
*Function    : esp_bleaddr_detect
*Description : 蓝牙mac地址提取
*Input       :
*Output      :
*Return      : 
*Others      : AT+BLEADDR? "08:3a:8d:48:82:46"
********************************************************************************
*/
static int esp_bleaddr_detect(void)
{
    char tmpbuf[40] = {'\r', '\n', 'O', 'K', '\0'};
    int mac[6]={0};

    if (esp_cmd_line("AT+BLEADDR?\r\n", "+BLEADDR:", tmpbuf, sizeof(tmpbuf), 3000, 1) == AT_CMD_DATA_GET_OK) {

        sscanf(tmpbuf,"\"%x:%x:%x:%x:%x:%x\"",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);

        //copy data
        esp.server.mac[0] = mac[0];
        esp.server.mac[1] = mac[1];
        esp.server.mac[2] = mac[2];
        esp.server.mac[3] = mac[3];
        esp.server.mac[4] = mac[4];
        esp.server.mac[5] = mac[5];

        return 0;
    }
 
    return -1;
}
/*
********************************************************************************
*Function    : esp_bleadv_packet
*Description : 蓝牙广播包组包函数
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
static int esp_bleadv_packet(char *adv,char advlen, char *ble_name)
{
    char adv_hex[64];
    char ble_name_len;

    
    adv_hex[0] = 0x02;
    adv_hex[1] = 0x01;
    adv_hex[2] = 0x06;
    ble_name_len = strlen(ble_name);
    
    adv_hex[3] = ble_name_len+1; 
    adv_hex[4] = 0x09;
    
    strncpy(adv_hex+5,ble_name,ble_name_len);
    
    adv_hex[5+ble_name_len] = 0x03;
    adv_hex[6+ble_name_len] = 0x03;
    adv_hex[7+ble_name_len] = 0x02;
    adv_hex[8+ble_name_len] = 0xA0;
    
    
    advlen = ustrhextostr(adv, adv_hex,9+ble_name_len);
    
    return advlen;
}
/*
********************************************************************************
*Function    : esp_bleadv_config
*Description : 蓝牙配置函数
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功 -1  -- 配置初始化失败  
*Others      :
********************************************************************************
*/
static int esp_bleadv_config(char *pincode,char *ble_name)
{
    char cmdbuf[128] = {'\r', '\n', 'O', 'K', '\0'}; 
    char adv_data[64];
    
    sprintf(cmdbuf,"AT+BLEADVPARAM=50,50,0,0,7,0,,\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return 0;
    }
    
    esp_bleadv_packet(adv_data, sizeof(adv_data),ble_name);
    sprintf(cmdbuf,"AT+BLEADVDATA=\"%s\"\r\n",adv_data);
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return 0;
    }
    
    sprintf(cmdbuf,"AT+BLESECPARAM=1,0,16,3,3\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return 0;
    }
    
    if(pincode == NULL){
        sprintf(cmdbuf,"AT+BLESETKEY=%s\r\n","123456");
    }else{
        sprintf(cmdbuf,"AT+BLESETKEY=%s\r\n",pincode);
    }

    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return 0;
    }
    
    sprintf(cmdbuf,"AT+BLEADVSTART\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return 0;
    }

    return 1;
}
/*
********************************************************************************
*Function    : __esp_urc_bledisconn
*Description : 蓝牙断开回调函数，退出后重新进行广播处理
*Input       : NONE
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
void __esp_urc_bledisconn(const char *data, int len)
{
    ble_server_t *server= & esp.server;
    
    LOG_D("[%s] : [%s]",__func__,data);
     
    if (esp_cmd_line("+++", "ready", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"+++\" execute fail\n"); 
        return ;
    }
    
    server->connect=FALSE;
    
    esp_set_atmode(ESP_MODE_AT);
 
    esp_at_resp_detect();
    
    __esp_bleadv_start();

    return ;
}
/*
********************************************************************************
*Function    : __esp_urc_bleconn
*Description : 蓝牙链接回调函数
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
void __esp_urc_bleconn(const char *data, int len)
{
    ble_server_t *server= &esp.server;
    char cmdbuf[128] = {0}; 

    server->connect=TRUE;
    
    LOG_D("[%s] : [%s]",__func__,data);

    sprintf(cmdbuf,"AT+BLEENC=0,3\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, sizeof(0), 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("[%s]Command: \"%s\" execute fail\n",__func__,cmdbuf); 
        return ;
    }
#if 0
    sprintf(cmdbuf,"AT+BLESPPCFG=1,1,7,1,5\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return ;
    }

    sprintf(cmdbuf,"AT+BLESPP\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return ;
    }
     esp_set_atmode(ESP_MODE_SPP);
#endif 
    return ;
}
/*
********************************************************************************
*Function    : __esp_urc_bleauthcmpl
*Description : 1、蓝牙加密鉴权处理，
               2、进入透传模式功能函数
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void __esp_urc_bleauthcmpl(const char *data, int len)
{    
    ble_server_t *server= & esp.server;
    char cmdbuf[128] = {0}; 
    int res[2];
    
    sscanf(data,"%d,%d",&res[0],&res[1]);
    
    if(res[0] != 0 ){
        LOG_E("[%s] Authentication failure [%s]\n",__func__,data); 
        server->connect=FALSE;
        return ;
    }
    sprintf(cmdbuf,"AT+BLESPPCFG=1,1,7,1,5\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return ;
    }

    sprintf(cmdbuf,"AT+BLESPP\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return ;
    }
    
    esp_set_atmode(ESP_MODE_SPP);
 
    LOG_D("__urc_ble_authcmpl : [%s]",data);
    return ;
}
/*
********************************************************************************
*Function    : __esp_ble_send
*Description : 透传发送函数
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/  
int __esp_ble_send(socket_t *socket, ble_msg_t *msg)
{   
    LOG_HEX("__esp_ble_send", 16, (uint8_t const *)msg->buf, msg->len);
    
    return at_sendstr( esp.client,(unsigned  char const *)msg->buf,  msg->len );
}
/*
********************************************************************************
*Function    : __esp_ble_open
*Description : 蓝牙打开函数
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
static int __esp_ble_open(void)
{    
    ble_server_t *server= & esp.server;
    
    esp_cwmode_config(ESP_CWMODE_NONE);
      
    esp_bleinit_config(ESP_BLEINIT_SER);
  
    esp_bleaddr_detect();
    
    esp_bleadv_config(server->pin,server->name);
        
    return 0;
}
/*
********************************************************************************
*Function    : __esp_bleadv_start
*Description : ESP  
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功
               -1  -- 配置初始化失败  
*Others      :
********************************************************************************
*/
static int __esp_bleadv_start(void)
{   
    char cmdbuf[128] = {'\r', '\n', 'O', 'K', '\0'};
 
    if( esp_ls_atmode() == ESP_MODE_SPP ){
        return 0;
    }

    sprintf(cmdbuf,"AT+BLEADVSTART\r\n");
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
        return 0;
    }

    return 1;
}
/*
********************************************************************************
                               外部接口函数
********************************************************************************
*/
/*
********************************************************************************
*Function    : esp_ble_send
*Description : 蓝牙发送对外接口函数
*Input       :
*Output      :
*Return      : 0 if success, -1 fail.
*Others      :
********************************************************************************
*/
int esp_ble_send(int sd, u8_t *buf, u32_t len, void *arg)
{
    socket_msg_t *msg;
    ble_msg_t *ble_msg;
    
    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }
    //socket not used
    if (atomic_read(&esp.sockets[sd].used) != 1) {
        return -1;
    }
    
    if( esp_ls_atmode() == ESP_MODE_SPP ){
        return -2;
    }
    
    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(ble_msg_t) + len);
    if (msg == RT_NULL) {
        return -1;
    }
    ble_msg = (ble_msg_t *)(msg->msg);
    at_memset(ble_msg, 0, sizeof(ble_msg_t));

    at_memcpy(ble_msg->buf, buf, len);
    ble_msg->len = len;

    msg->sd = 0;
    msg->type = MSG_BLE;
    msg->res_cb = 0;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : esp_ble_init
*Description : 蓝牙初始化功能函数
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int esp_ble_init(char *name, char *pin,int (*recv)(void *arg))
{
    esp_set_ble(name,pin);

    esp_set_function(__esp_ble_open,__esp_bleadv_start,recv);
    
    return 0;
}
