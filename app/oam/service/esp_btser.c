/*
 * esp_btser.c
 *
 *  Created on: 2023年08月24日
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
#include "PPItems.h"
#include "lan.h"

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
    char name[64];
    char pin[7];
} bt_ser_t;


static lan_msg_t BleMsg;
static bt_ser_t BtSer; 
#define get_ble_service()   (&BtSer)
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : esp_btser_send
*Description : 蓝牙发送接口函数
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int esp_btser_send(u8_t *msg, u32_t msglen, void *arg)
{    
    bt_ser_t *service = get_ble_service();
    
    LOG_HEX("ble-send",16,(u8_t *)msg,msglen);
    
    return esp_ble_send(service->sd, (void *)msg, msglen, 0);
}
/*
********************************************************************************
*Function    : esp_btser_recv_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/   
static int esp_btser_recv_process(void *arg)
{    
    at_client_t client=(at_client_t)arg;
    unsigned char ch;
    
    if( esp_ls_atmode() == ESP_MODE_SPP ){
        return -1;
    }

    while( at_getchar(client, &ch, 1000, 1) == 0){
        rt_kprintf("0X%02X ",ch);
        lan_msg_recv( &BleMsg, ch);
    }
    return 0;
}
/*
********************************************************************************
*Function    : esp_btser_create
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int esp_btser_create(bt_ser_t *service)
{
    if (service->sd < 0) {
        service->sd = esp_socket_create(SOCK_BLE);
        esp_set_recv(service->sd, 0);
        BleMsg.from = esp_btser_send;
        service->connect = 1;
    }

    return 0;
}
/*
********************************************************************************
*Function    : esp_btser_disable
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
void esp_btser_disable(void)
{
    bt_ser_t *service = get_ble_service();
    if (service->sd >= 0) {
        service->sd = -1;
    }
}
/*
********************************************************************************
*Function    :  esp_btser_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
int esp_btser_init(void)
{    
    bt_ser_t *service = get_ble_service();
    
    PPItemRead(PP_BLE_NAME,service->name,PPItemSize(PP_BLE_NAME));
    
    PPItemRead(PP_BLE_PIN,service->pin,PPItemSize(PP_BLE_PIN));
    
    service->sd = -1;
    
    esp_btser_create(service);
    
    esp_ble_init(service->name,service->pin,esp_btser_recv_process);
    
    return 0;
}
