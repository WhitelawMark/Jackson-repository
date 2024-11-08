/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __ESP_H                                                          
#define __ESP_H  

#include "esp_cmd.h"
#include "at_type.h"
   
/*
********************************************************************************
********************************************************************************
*/ 
#define ESP_MAX_SOCKET       3
#define ESP_BASE_PORT        8090

/*
********************************************************************************
********************************************************************************
*/ 
typedef enum {
    ESP_NORMEL,
    ESP_SLEEP
}wakeup_type_t;

typedef enum {
    ESP_MODE_AT,
    ESP_MODE_SPP
}cmd_mode_t;

typedef enum {

    ESP_BLEINIT_NONE,
    ESP_BLEINIT_CLI,
    ESP_BLEINIT_SER
}bleinit_t;

typedef enum {
    ESP_WORK_NONE,
    ESP_WORK_BLE_SER,
    ESP_WORK_BLE_CLI,
    ESP_WORK_WIFI_STA,
    ESP_WORK_WFFI_APSTA,
    ESP_WORK_WFFI_AP,
}esp_work_mode_t;
    
typedef enum {
    ESP_CWMODE_NONE,
    ESP_CWMODE_STA,
    ESP_CWMODE_AP,
    ESP_CWMODE_STA_AP
}cmd_cwmode_t;

/*
********************************************************************************
********************************************************************************
*/ 

typedef struct {
    int len;
    char buf[0];
} ble_msg_t;

typedef struct {
    char        name[32];
    char        pin[16];
    char        mac[6];
    char        status;
    char        connect;
} ble_server_t;


/*
********************************************************************************
********************************************************************************
*/ 
typedef struct {
    char url[128];
    char filename[32];
    int  packlen;
    int  mode;
    
    void (*write)(int offer,char buf,int ulen);
    void (*notify)(int evtype);
} https_msg_t;

/*
********************************************************************************
********************************************************************************
*/ 
typedef struct {
    int  stat;
    char ssid[32];
    char pwd[16];
    char ip[32];
    char gateway[32];
    char netmask[32];
} wifi_sta_t;

/*
********************************************************************************
********************************************************************************
*/ 
typedef struct {
    int wifi_power;
    int ble_adv_power;
    int ble_scan_power;
    int ble_conn_power;
} rfpower_t;

typedef struct {
    char mac[32];
    int ecn;
    int rssi;
    int channel;
    int freq_offset;
    int freqcal_val;
    int pairwise_cipher;
    int group_cipher;
    int bgn;
    int wps;
} cwlap_t;
    
typedef struct {
    int ram_size;
    int heap_size;
} system_t;

typedef struct {
    char at_version[64];
    char sdk_version[64];
    char compile_time[64];
    char bin_version[64];
} version_t;

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

typedef struct {
    char username[128];
    char password[128];
    int (*init)(void);
    int (*scan)(void);
} config_msg_t;

typedef struct {
    at_client_t   client;
    rt_thread_t   thread;
    monitor_t     monitor;
    at_mq_t       mq;
    atcli_t       atcli;
    cwlap_t       lap; 
    version_t     vs;
    system_t      sys;
    rfpower_t     rfpower;
    ble_server_t  server;
    wifi_sta_t    wifista;
    socket_t      sockets[ESP_MAX_SOCKET];
    char          tmpbuf[640];
} esp_at_t;
/*
********************************************************************************
********************************************************************************
*/

int __esp_close(socket_t *socket);

int __esp_connect(socket_t *socket, connect_msg_t *msg, void (*notify)(int evtype));

int __esp_socket_msg_post(socket_msg_t *msg);

/*
********************************************************************************
********************************************************************************
*/
void esp_set_atmode(int atmode);
int esp_ls_atmode(void);

void esp_set_ble(char *name,char *pin);
void esp_set_wifi(char *ssid,char *pwd);

void esp_set_function(int (*init)(void),int (*scan)(void),int (*recv_process)(void *arg));
void esp_set_notify(int sd, void (*notify)(int evtype));
void esp_set_recv(int sd, void (*recv)(int sd, char *buf, int len));

int esp_socket_create(int type);
void esp_socket_delete(int sd);
socket_t *esp_get_socket(int sd, int type);

#endif 
/* __ESP_H */
