/*
 * bg77.h
 *
 *  Created on: 2019��1��2��
 *      Author: root
 */

#ifndef BG77_H_
#define BG77_H_

#include "at.h"
#include "at_type.h"
#include "at_client.h"
#include "bg77_cmd.h"
#include "bg77_tcpip.h"
#include "bg77_file.h"
#include "bg77_cart.h"
#include "bg77_ftp.h"
#include "bg77_mqtt.h"
#include "bg77_https.h"

#include <board.h>
/*
********************************************************************************
********************************************************************************
*/
#define BG77_BASE_PORT        8090
#define BG77_MAX_SOCKET       3

#define BG77_POWER_BAT_PIN    GET_PIN(A,15)        
#define BG77_POWER_PIN        GET_PIN(B,7)  
#define BG77_RESET_PIN        GET_PIN(B,6)  
// #define BG77_STATUS_PIN       GET_PIN(B,7)   //PD6
#define BG77_DTR_PIN          77  //PB9 唤醒硬件没有接

/*
********************************************************************************
********************************************************************************
*/
#define bg77_tick_from_ms(ms)             rt_tick_from_millisecond((rt_int32_t)(ms))
#define bg77_timer_start(endtick, tick)   (endtick = rt_tick_get() + (tick))
#define bg77_is_timer_end(endtick, tick)  ((tick = rt_tick_get() - (endtick)) < RT_TICK_MAX / 2)
/*
********************************************************************************
********************************************************************************
*/
 
#define AT_BLOCK             0
#define AT_NBLOCK            bg77_socket_ignore_result
/*
********************************************************************************
********************************************************************************
*/
 
#define at_socket_create     bg77_socket_create
#define at_socket_delete     bg77_socket_delete
#define at_set_notify        bg77_set_notify
#define at_set_recv          bg77_set_recv

#define at_tcp_connect       bg77_tcp_connect
#define at_tcp_send          bg77_tcp_send
#define at_tcp_recv          bg77_tcp_recv

#define at_udp_send          bg77_udp_send
#define at_udp_recv          bg77_udp_recv

#define at_qmt_connect       bg77_mqtt_connect
#define at_qmt_unsubscribe   bg77_mqtt_unsubscribe
#define at_qmt_subscribe     bg77_mqtt_subscribe
#define at_qmt_publish       bg77_mqtt_publish
#define at_qmt_recv          bg77_mqtt_recv

#define at_get_sim_id        bg77_get_iccid
#define at_get_imei          bg77_get_imei
#define at_get_model         bg77_get_model
#define at_get_rssi          bg77_get_rssi
#define at_is_connected      bg77_is_connected
#define at_is_initok         bg77_is_initok
#define at_init              bg77_init
#define at_sys_ctrl          bg77_wakeup_ctrl
 
/*
********************************************************************************
********************************************************************************
*/
 
typedef enum {
    BG77_NORMEL,
    BG77_SLEEP
}wakeup_type_t;


/*
********************************************************************************
********************************************************************************
*/
typedef struct {
    char latitude[32]; 
    char longitude[32];
    int  UTC;
    int  HDOP;
    int  altitude;
    int  fix;
    int  COG;
    int  spkm;
    int  spkn;
    int  date;
    int  nsat;
} gpsloc_t;


typedef struct {
    at_client_t   client;
    rt_thread_t   thread;
    monitor_t     monitor;
    char          imei[32];
    char          model[32];
    char          iccid[32];
    char          rssi;
    socket_t      sockets[BG77_MAX_SOCKET];
    at_mq_t       mq;
    atcli_t       atcli;
    char          tmpbuf[640];
} at_manage_t;
/*
********************************************************************************
********************************************************************************
*/
int __bg77_connect(socket_t *socket, connect_msg_t *msg, void (*notify)(int evtype));
int __bg77_close(socket_t *socket);
int __bg77_socket_msg_post(socket_msg_t *msg);
/*
********************************************************************************
********************************************************************************
*/
int bg77_socket_create(int type);

void bg77_socket_delete(int sd);

void bg77_set_notify(int sd, void (*notify)(int evtype));

void bg77_set_recv(int sd, void (*recv)(int sd, char *buf, int len));

void bg77_socket_ignore_result(int res, void *arg);

int bg77_is_connected(int sd);

int bg77_is_initok(void);

const char *bg77_get_iccid(void);

const char *bg77_get_imei(void);

const char *bg77_get_model(void);

char bg77_get_rssi(void);

void bg77_wakeup_ctrl(int power);

void bg77_airmode_config(int mode);

int bg77_init(void);

socket_t *bg77_get_socket(int sd, int type);
#endif /* BG77_H_ */
