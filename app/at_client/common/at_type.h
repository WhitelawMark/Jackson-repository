/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __AT_TYPE_H                                                          
#define __AT_TYPE_H    

#include <stdio.h>
#include <string.h>
#include "rtthread.h"
#include "type.h"
#include "at.h"
#include "at_client.h"
/*
********************************************************************************
********************************************************************************
*/
 
typedef enum { 
    MSG_BLE,
    MSG_UDP,
    MSG_TCP,
    MSG_MQTT_PUB,
    MSG_MQTT_SUB,
    MSG_MQTT_UNS,
    MSG_CONNECT,
    MSG_FILE,
    MSG_HTTP_DOWNLOAD,
    MSG_HTTP_UPLOAD,
    MSG_FTP_UPLOAD,   
    MSG_FTP_DOWNLOAD,
    MSG_CERT_UPLOAD,
} msg_type_t;

/*
********************************************************************************
********************************************************************************
*/

typedef struct {
    int airmode;
    int restart;
    int powersave;
    int init_flag;
    int fail_times;
    int signal_ready;
    int at_mode;
    rt_tick_t timer;
    int (*init)(void);
    int (*scan)(void);
    int (*recv_process)(void *arg);
} monitor_t;

typedef struct {
    atomic_t enable;
    rt_sem_t rx_sem;
} atcli_t;

typedef struct {
    int sd;
    msg_type_t type;
    void(*res_cb)(int res, void *arg);
    void *arg;
    char msg[0];
} socket_msg_t;

typedef struct {
    at_addr_t addr;
    char clientid[128];
    char username[128];
    char password[128];
} sockaddr_t;

typedef struct {
    sockaddr_t remote_addr;
} connect_msg_t;

typedef struct {
    struct rt_semaphore resp_sem;
    int result;
} resp_msg_t;

/*
********************************************************************************
********************************************************************************
*/

#endif 
/* __AT_TYPE_H */
