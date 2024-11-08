/*
 * ble_service.c
 *
 *  Created on: 2023年08月24日
 *      Author: lwp
 */
#include <stdio.h>
#include <string.h>
#include <board.h>
#include <rtthread.h>
#include "type.h"
#include "bluetooth_service.h" 
#include "oam_thread.h" 
#include "at.h"
#include "at_client.h"
#include "esp.h"
#include "esp_ble.h"   
#include "PPItems.h"
#include "bluetooth.h"

#define DBG_TAG "bluetooth"
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
} bluetooth_service_t;


static blue_msg_t BleMsg;

static bluetooth_service_t BLEService; 

#define get_ble_service()   (&BLEService)
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : bluetooth_send
*Description : 蓝牙发送接口函数
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int bluetooth_send(u8_t *msg, u32_t msglen, void *arg)
{    
    BLE_Service_t *service = get_ble_service();
    
    LOG_HEX("ble-send",16,(u8_t *)msg,msglen);
    
    return esp_ble_send(service->sd, (void *)msg, msglen, 0);
}
/*
********************************************************************************
*Function    : bluetooth_recv_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/   
static int bluetooth_recv_process(void *arg)
{    
    at_client_t client=(at_client_t)arg;
    unsigned char ch;
    
    if( esp_ls_atmode() == ESP_MODE_SPP ){
        return -1;
    }

    while( at_getchar(client, &ch, 1000, 1) == 0){
        rt_kprintf("0X%02X ",ch);
        blue_msg_recv( &BleMsg, ch);
    }
    return 0;
}
/*
********************************************************************************
*Function    : bluetooth_sevice_create
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int bluetooth_sevice_create(BLE_Service_t *service)
{
    if (service->sd < 0) {
        service->sd = esp_socket_create(SOCK_BLE);
        esp_set_recv(service->sd, 0);
        BleMsg.from = bluetooth_send;
        service->connect = 1;
    }

    return 0;
}
/*
********************************************************************************
*Function    : ble_service_disable
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
void bluetooth_service_disable(void)
{
    BLE_Service_t *service = get_ble_service();
    if (service->sd >= 0) {
        service->sd = -1;
    }
}
/*
********************************************************************************
*Function    :  bluetooth_service_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
int bluetooth_service_init(void)
{    
    BLE_Service_t *service = get_ble_service();
    
    PPItemRead(PP_BLE_NAME,service->name,PPItemSize(PP_BLE_NAME));
    
    PPItemRead(PP_BLE_PIN,service->pin,PPItemSize(PP_BLE_PIN));
    
    service->sd = -1;
    
    bluetooth_sevice_create(service);
    
    esp_ble_init(service->name,service->pin,bluetooth_recv_process);
    
    return 0;
}
