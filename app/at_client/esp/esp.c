/*
 * esp.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "rtthread.h"
#include "rthw.h"
#include "drivers/pin.h"
#include "type.h"
#include "ustring.h"
#include "at_client.h"
#include "at.h"
#include "esp.h"
#include "esp_bt.h"
#include "esp_mqtt.h"
#include "esp_https.h"
#include "esp_tcpip.h"

#include "lan.h"

#include <board.h>
/*
********************************************************************************
*
********************************************************************************
*/

#define DBG_TAG "esp"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>

/*
********************************************************************************
*
********************************************************************************
*/
#define ESP_OFF      0
#define ESP_ON       (!ESP_OFF)

#define esp_tick_from_ms(ms)             rt_tick_from_millisecond((rt_int32_t)(ms))
#define esp_timer_start(endtick, tick)   (endtick = rt_tick_get() + (tick))
#define esp_is_timer_end(endtick, tick)  ((tick = rt_tick_get() - (endtick)) < RT_TICK_MAX / 2)
/*
********************************************************************************
********************************************************************************
*/
extern int esp_mqtt_open(int connect_id, at_addr_t *addr, const char *clientid, const char *username, const char *password);
static int esp_socket_msg_process(socket_msg_t *msg);
/*
********************************************************************************
********************************************************************************
*/
extern void __esp_urc_bleconn(const char *data, int len);
extern void __esp_urc_bleauthcmpl(const char *data, int len);
extern void __esp_urc_bledisconn(const char *data, int len);

extern void __esp_urc_qmtrecv(const char *data, int len);
extern void __esp_urc_qmtdisconn(const char *data, int len);
/*
********************************************************************************
********************************************************************************
*/
static const at_urc_t urc_table[] = {
    /*prefix,              suffix,   func*/
    {"+BLECONN:",          "\r\n",  __esp_urc_bleconn  },  //The URC of BLE Connect AT commands
    {"+BLEDISCONN:",       "\r\n",  __esp_urc_bledisconn  },  //The URC of BLE Connect AT commands
    {"+BLEAUTHCMPL:",      "\r\n",  __esp_urc_bleauthcmpl  },
    {"+MQTTSUBRECV:",      "\r\n",  __esp_urc_qmtrecv  },
    {"+MQTTDISCONNECTED:", "\r\n",  __esp_urc_qmtdisconn  },
};
 
esp_at_t esp;

/*
********************************************************************************
*
********************************************************************************
*/
#define get_version_ptr()   (&esp.vs)    
#define get_lap_ptr()       (&esp.lap)    
#define get_sta_ptr()       (&esp.wifista)   

/*
********************************************************************************
*Function    :
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
#define ESP_EN_GPIO    GET_PIN(A, 12)
void esp_en_chip(void)
{
    rt_pin_mode(ESP_EN_GPIO, PIN_MODE_OUTPUT);
    rt_pin_write(ESP_EN_GPIO, PIN_HIGH);
    rt_thread_mdelay(500);
    rt_kprintf("esp_en_chip=%d \r\n",ESP_EN_GPIO);
}
/*
********************************************************************************
*Function    :
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
#define ESP_EN_3V    GET_PIN(B,4)
void esp_power_3v(void)
{
    rt_pin_mode(ESP_EN_3V, PIN_MODE_OUTPUT);
    rt_pin_write(ESP_EN_3V, PIN_HIGH);
    rt_thread_mdelay(500);
    rt_kprintf("esp_power_3v=%d \r\n",ESP_EN_3V);

}

/*
********************************************************************************
*Function    : esp_config_init
*Description : ESP 配置初始化
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功
               -1  -- 配置初始化失败
*Others      :
********************************************************************************
*/
static int esp_config_init(void)
{
    int i,j;
    static const struct  {
        const char *cmd;
        const char *ack;
        unsigned int timeout_ms;
    } table[3] = {
        "ATE0\r\n",             "OK",  2000, // 
        "AT+RESTORE\r\n",       "OK",  2000, // 
        "AT+SYSMSG=7\r\n",      "OK",  2000, // 
    };

    for (i = 0; i < sizeof(table)/sizeof(table[0]); i++) {
        //每条指令最多执行2次
        for (j = 2; j > 0; j--) {
            if (esp_cmd_line(table[i].cmd, table[i].ack, 0, 0, table[i].timeout_ms, 1) == AT_CMD_ACK_OK) {
                break;
            }
            if (j == 1) {
                LOG_E("Command: \"%s\" execute fail\n", table[i].cmd);
                return -1;
            }
        }
    }
    
    esp_gsm_detect();
    
    return 0;
}

/*
********************************************************************************
*Function    : esp_status_monitor
*Description : esp 模块状态监测
*Input       :
*Output      :
*Return      : 下次检测等待的时间
*Others      :
********************************************************************************
*/
static rt_tick_t esp_status_monitor(void)
{
    monitor_t *monitor = &esp.monitor;
    rt_tick_t timeout_tick;
    
    //系统重新初始化    
    if (monitor->restart) {
        monitor->restart = 0;

        LOG_D("ESP-AT module restart...");

        //ESP模块重启电源
        esp_restart();
        //硬件复位
        esp_reset();

        //需要重新初始化
        monitor->init_flag = 0;
        
        timeout_tick = esp_tick_from_ms(5000);
        esp_timer_start(monitor->timer, timeout_tick);

        LOG_D("ESP-AT module restart OK!");
    }

    //已初始化
    if (monitor->init_flag) {
        if (esp_is_timer_end(monitor->timer, timeout_tick)) {
            //检测信号信息
            
            monitor->signal_ready = 1;
            monitor->fail_times = 0;
            
            if(monitor->scan ){
                if(monitor->scan() ){
                    timeout_tick = esp_tick_from_ms(3000);
                    esp_timer_start(monitor->timer, timeout_tick);
                    return timeout_tick;
                }
            }

            if (monitor->fail_times >= 3) {
                monitor->restart = 1;
                timeout_tick = 0;
                LOG_D("ESP-AT  restart!");
            } else {
                //每180S 检测一次
                timeout_tick = esp_tick_from_ms(30*1000);
                esp_timer_start(monitor->timer, timeout_tick);
            }
        } else {
            timeout_tick = 0 - timeout_tick;
        }
       
        return timeout_tick;
    }
 
    //时间未到
    if (!esp_is_timer_end(monitor->timer, timeout_tick)) {
        return (0 - timeout_tick);
    }

    //1. AT命令响应检测
    if (esp_at_resp_detect() != 0) {
        LOG_E("ESP-AT command response fail");
        //5S 后再次进行初始化
        timeout_tick = esp_tick_from_ms(3000);
        esp_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    }

    //2. 配置初始化
    if (esp_config_init() != 0) {
        LOG_E("ESP-AT configure fail");
        //3S 后再次进行初始化
        timeout_tick = esp_tick_from_ms(3000);
        esp_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    }
    //3. 配置初始化
    if(monitor->init ){
        if(monitor->init() ){
            timeout_tick = esp_tick_from_ms(3000);
            esp_timer_start(monitor->timer, timeout_tick);
            return timeout_tick;
        }
    }else{
        timeout_tick = esp_tick_from_ms(5000);
        esp_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    }

    //初始化完成
    monitor->init_flag = 1;
    monitor->fail_times = 0;

    //120S 后进行信号检测
    timeout_tick = esp_tick_from_ms(30*000);
    esp_timer_start(monitor->timer, timeout_tick);
    LOG_D("ESP-AT configure ok");
    return timeout_tick;
}
/*
********************************************************************************
*Function    : esp_ls_atmode
*Description : esp 获取当前模式
*Input       :
*Output      :
*Return      :  
*Others      :
********************************************************************************
*/
int esp_ls_atmode(void)
{
    return esp.monitor.at_mode;
}

/*
********************************************************************************
*Function    : esp_get_atmode
*Description : esp 获取当前模式
*Input       :
*Output      :
*Return      :  
*Others      :
********************************************************************************
*/
void esp_set_atmode(int atmode)
{
    esp.monitor.at_mode = atmode;
}
/*
********************************************************************************
*Function    : esp_is_initok
*Description : esp 檢測是否初始化成功
*Input       :
*Output      :
*Return      :  
*Others      :
********************************************************************************
*/
int esp_is_initok(void)
{
    return esp.monitor.init_flag;
}
/*
********************************************************************************
*Function    : esp_socket_sync_result
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void esp_socket_sync_result(int res, void *arg)
{
    resp_msg_t *resp = (resp_msg_t *)arg;

    resp->result = res;

    //response semaphore
    rt_sem_release(&resp->resp_sem);
}
/*
********************************************************************************
*Function    : esp_thread_notify
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void esp_thread_notify(int evtype)
{
   (void)evtype;

   rt_sem_release(esp.client->rx_notice);
}
/*
********************************************************************************
*Function    : esp_get_socket
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
socket_t *esp_get_socket(int sd, int type)
{
    socket_t *socket;
    int i;

    for (i = 0; i < ESP_MAX_SOCKET; i++) {
        socket = &esp.sockets[i];
        if (atomic_read(&socket->used) == 1 &&
                (type & socket->type) == socket->type && sd == socket->sd) {
            return socket;
        }
    }

    return RT_NULL;
}
/*
********************************************************************************
*Function    : esp_socket_create
*Description :
*Input       :
*Output      :
*Return      : socket
*Others      :
********************************************************************************
*/
int esp_socket_create(int type)
{
    socket_t *socket;
    int sd;

    if (type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_MQTT && type !=SOCK_HTTPS) {
        return -1;
    }

    //get free socket
    for (sd = 0; sd < ESP_MAX_SOCKET; sd++) {
        if (atomic_cmpxchg(&esp.sockets[sd].used, 0, -1) == 0) {
            break;
        }
    }
    if (sd == ESP_MAX_SOCKET) {
        return -1;
    }
    socket = &esp.sockets[sd];

    //init
    socket->type = type;
    socket->index = sd;
    socket->sd = -1;
    socket->fail_count = 0;
    at_memset(&socket->remote_addr, 0, sizeof(socket->remote_addr));
    socket->notify = 0;
    socket->recv = 0;
    atomic_set(&socket->used, 1);

    return sd;
}
/*
********************************************************************************
*Function    : esp_set_ble
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void esp_set_ble(char *name,char *pin)
{
   if( name == NULL || pin == NULL ){
       return ;
   }
   strcpy(esp.server.name,name);
   strcpy(esp.server.pin,pin);
}
/*
********************************************************************************
*Function    : esp_set_wifi
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void esp_set_wifi(char *ssid,char *pwd)
{
   if( ssid == NULL || pwd == NULL ){
       return ;
   }
   strcpy(esp.wifista.ssid,ssid);
   strcpy(esp.wifista.pwd,pwd);
}
/*
********************************************************************************
*Function    : esp_set_function
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void esp_set_function(int (*init)(void),int (*scan)(void),int (*recv_process)(void *arg))
{
    esp.monitor.init = init;
    esp.monitor.scan = scan;
    esp.monitor.recv_process = recv_process;
    esp.monitor.restart = 1;
}
/*
********************************************************************************
*Function    : esp_set_notify
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void esp_set_notify(int sd, void (*notify)(int evtype))
{
    socket_t *socket;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return;
    }
    socket = &esp.sockets[sd];

    socket->notify = notify;
}

/*
********************************************************************************
*Function    : esp_set_recv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void esp_set_recv(int sd, void (*recv)(int sd, char *buf, int len))
{
    socket_t *socket;

    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return;
    }
    socket = &esp.sockets[sd];

    __atomic_set((volatile long *)&socket->recv, (long)recv);
}
/*
********************************************************************************
*Function    : esp_socket_msg_post
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 fail.
*Others      :
********************************************************************************
*/
int __esp_socket_msg_post(socket_msg_t *msg)
{
    int res;

    if (rt_thread_self() == esp.thread) {
         msg->res_cb = RT_NULL;
         msg->arg = RT_NULL;
         res = esp_socket_msg_process(msg);
    } else {
        if (msg->res_cb) {
            at_message_put(&esp.mq, msg, esp_thread_notify, EV_TX);
            res = 0;
        } else {
            resp_msg_t resp;
            resp.result = 0;
            rt_sem_init(&resp.resp_sem, "resp", 0, RT_IPC_FLAG_FIFO);

            msg->res_cb = esp_socket_sync_result;
            msg->arg = (void *)&resp;
            at_message_put(&esp.mq, msg, esp_thread_notify, EV_TX);

            //wait response semaphore
            rt_sem_take(&resp.resp_sem, RT_WAITING_FOREVER);

            rt_sem_detach(&resp.resp_sem);
            res = resp.result;
        }
    }

    return res;
}
/*
********************************************************************************
*Function    : esp_socket_detect
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void esp_socket_detect(void)
{
    socket_t *socket;
    void *msg;
    int i;
    int used;

    for (i = 0; i < ESP_MAX_SOCKET; i++) {
        socket = &esp.sockets[i];
        used = atomic_read(&socket->used);
        if (used == 2) {
            //close socket
            __esp_close(socket);
            //free all RX message
            while ((msg = at_message_get(&socket->rx_mq)) != RT_NULL) {
                at_message_free(msg);
            }
            atomic_set(&socket->used, 0);
        } else if (used == 1) {
            if (socket->sd < 0) {
                //连接断开不自动重连
                __esp_connect(socket, RT_NULL, (socket->type == SOCK_MQTT)? socket->notify:RT_NULL);
            }
            if (socket->fail_count > 9) {
                socket->fail_count = 0;
                esp.monitor.restart = 1;
                LOG_E("ESP socket fail detected, force to restart module");
            }
        }
    }
}
/*
********************************************************************************
*Function    : esp_open
*Description :
*Input       :
*Output      :
*Return      : >= 0 if socket, -1 operate if fail, < -1 other error
*Others      :
********************************************************************************
*/
static int esp_open(int type, int connect_id, unsigned short local_port, sockaddr_t *remote_addr)
{
    int sd = -2;

    switch (type) {
        case SOCK_DGRAM:
        case SOCK_STREAM:
            sd = __esp_socket_open(type, connect_id, local_port, &remote_addr->addr);
            break;
        case SOCK_MQTT:
            sd = __esp_mqtt_open(connect_id, &remote_addr->addr,
                    remote_addr->clientid, remote_addr->username, remote_addr->password);
            break;
        case SOCK_BLE:
            sd = __esp_mqtt_open(connect_id, &remote_addr->addr,
                    remote_addr->clientid, remote_addr->username, remote_addr->password);
            break;
        default:
            break;
    }

    return sd;
}
/*
********************************************************************************
*Function    : esp_socket_delete
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
void esp_socket_delete(int sd)
{
    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return;
    }
    if (atomic_cmpxchg(&esp.sockets[sd].used, 1, 2) == 1) {
        esp_thread_notify(EV_TX);
    }
}
/*
********************************************************************************
*Function    : __esp_close
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int __esp_close(socket_t *socket)
{
    int ret = -1;

    if (socket->sd >= 0) {
        switch (socket->type) {
            case SOCK_DGRAM:
            case SOCK_STREAM:
                __esp_socket_close(socket->sd);
                socket->sd = -1;
                ret = 0;
                break;
            case SOCK_MQTT:
                __esp_mqtt_close(socket->sd);
                socket->sd = -1;
                ret = 0;
                break;
            case SOCK_BLE:
                __esp_mqtt_close(socket->sd);
                socket->sd = -1;
                ret = 0;
                break;
            default:
                break;
        }
    }

    return ret;
}
/*
********************************************************************************
*Function    : __esp_connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 fail.
*Others      :
********************************************************************************
*/
int __esp_connect(socket_t *socket, connect_msg_t *msg, void (*notify)(int evtype))
{
    if (socket->sd >= 0) {
        __esp_close(socket);
    }

    if (msg != RT_NULL) {
        at_memcpy(&socket->remote_addr, &msg->remote_addr, sizeof(socket->remote_addr));
        socket->remote_addr.addr.ip[sizeof(socket->remote_addr.addr.ip)-1] = '\0';
        socket->remote_addr.clientid[sizeof(socket->remote_addr.clientid)-1] = '\0';
        socket->remote_addr.username[sizeof(socket->remote_addr.username)-1] = '\0';
        socket->remote_addr.password[sizeof(socket->remote_addr.password)-1] = '\0';
    }

    socket->sd = esp_open(socket->type, socket->connect_id, socket->local_port, &socket->remote_addr);
    if (socket->sd < 0) {
        if (socket->sd == -1) {
            socket->fail_count++;
            LOG_E("socket open fail(fail count %d)", socket->fail_count);
        }
        return -1;
    }
    socket->fail_count = 0;
    
    LOG_D("connect success");

    //connect notify
    if (notify) {
        notify(EV_CONNECT);
    }

    return 0;
}
/*
********************************************************************************
*Function    : esp_socket_msg_process
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail
*Others      :
********************************************************************************
*/
static int esp_socket_msg_process(socket_msg_t *msg)
{
    socket_t *socket;
    int res = -1;
    
    if (msg->sd < 0 || msg->sd >= ESP_MAX_SOCKET) {
        LOG_E("Socket %d out of range", msg->sd);
    } else {
         socket = &esp.sockets[msg->sd];
          switch (msg->type) {
              case MSG_UDP:
                  res = __esp_udp_send(socket, (udp_msg_t *)(msg->msg));
                  break;
              case MSG_TCP:
                  res = __esp_tcp_send(socket, (tcp_msg_t *)(msg->msg));
                  break;
              case MSG_MQTT_SUB:
                  res = __esp_mqtt_subscribe(socket, (sub_msg_t *)(msg->msg));
                  break;
              case MSG_MQTT_UNS:
                  res = __esp_mqtt_unsubscribe(socket, (sub_msg_t *)(msg->msg));
                  break;
              case MSG_MQTT_PUB:
                  res = __esp_mqtt_publish(socket, (mqtt_msg_t *)(msg->msg));
                  break;
              case MSG_CONNECT:
                  res = __esp_connect(socket, (connect_msg_t *)(msg->msg), RT_NULL);
                  break; 
              case MSG_HTTP_DOWNLOAD:
                  res = __esp_https_download(socket,  (https_msg_t *)(msg->msg));
                  break;
              case MSG_BLE:
                  res = __esp_ble_send(socket, (ble_msg_t *)(msg->msg));
                  break;  
              default:
                  break;
          }
    }
    //result callback
    if (msg->res_cb) {
        msg->res_cb(res, msg->arg);
    }

    //free message
    at_message_free(msg);

    return res;
}
/*
********************************************************************************
*Function    : esp_socket_msg_clean
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void esp_socket_msg_clean(void)
{
    socket_msg_t *msg;

    while ((msg = at_message_get(&esp.mq)) != RT_NULL) {
        //result callback
        if (msg->res_cb) {
            msg->res_cb(0, msg->arg);
        }

        //free message
        at_message_free(msg);
    }
}
/*
********************************************************************************
*Function    : esp_socket_msg_accept
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int esp_socket_msg_accept(void)
{
    socket_msg_t *msg;
    int fail_cnt = 0;

    while ((msg = at_message_get(&esp.mq)) != RT_NULL) {
        if (esp_socket_msg_process(msg) != 0) {
            if (++fail_cnt > 3) {
                return -1;
            }
        } else {
            fail_cnt = 0;
        }
    }

    return 0;
}
/*
********************************************************************************
*Function    : esp_socket_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int esp_socket_process(at_client_t client)
{
    (void)client;

    //socket 状态检测
    esp_socket_detect();
 
    //socket 消息处理
    return esp_socket_msg_accept();
}
/*
********************************************************************************
*Function    : esp_atcli_process
*Description : AT命令行 处理
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void esp_atcli_process(at_client_t client)
{
    if (atomic_cmpxchg(&esp.atcli.enable, 1, 2) == 1) {
        rt_device_t console = rt_console_get_device();
        char ch;

		while (atomic_read(&esp.atcli.enable) == 2) {
            //socket 消息清除
            esp_socket_msg_clean();
            
            rt_sem_take(client->rx_notice, RT_WAITING_FOREVER);

            while( rt_device_read(client->device, 0, &ch, 1) == 1){
               rt_device_write(console, 0, &ch, 1);
            }
        }

        //exit CLI mode, restart module
        //esp.monitor.restart = 1;
    }
}
/*
******************************************************************************** 
*Function    : console_rx_ind
*Description : AT命令
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
static rt_err_t console_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(esp.atcli.rx_sem);

    return RT_EOK;
}
/*
******************************************************************************** 
*Function    : espat
*Description : AT命令
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
static void espat(int argc, char **argv)
{
#define AT_CLI_NOP_KEY                 0x00
#define AT_CLI_SOH_KEY                 0x01
#define AT_CLI_ESC_KEY                 0x1B
#define AT_CLI_BACKSPACE_KEY           0x08
#define AT_CLI_DELECT_KEY              0x7F

    struct rt_semaphore console_rx_notice;
    rt_err_t (*old_rx_ind)(rt_device_t dev, rt_size_t size);
    rt_device_t console;
    rt_base_t int_lvl;
    rt_uint16_t old_open_flag;
    int i;
    char *disp="\\|/-";
    char ch;

    if (argc != 2 || strcmp(argv[1], "client") != 0) {
        rt_kprintf("Please input '<client>' \n");
        return;
    }

    rt_sem_init(&console_rx_notice, "atcli", 0, RT_IPC_FLAG_FIFO);
    esp.atcli.rx_sem = &console_rx_notice;

    //backup console configure
    int_lvl = rt_hw_interrupt_disable();
    console = rt_console_get_device();
    if (console) {
        old_open_flag = console->open_flag;
        console->open_flag &= (~RT_DEVICE_FLAG_STREAM);
        /* backup RX indicate */
        old_rx_ind = console->rx_indicate;
        rt_device_set_rx_indicate(console, console_rx_ind);
    }
    rt_hw_interrupt_enable(int_lvl);

    //entry to at mode
    atomic_set(&esp.atcli.enable, 1);
    esp_thread_notify(EV_TX);

    rt_kprintf("======== Welcome to using AT command client cli, press 'ESC' to exit========\n");
    rt_kprintf("Entry AT client cli mode  \n");
    i = 0;

    do {
        rt_kprintf("%c%c",AT_CLI_BACKSPACE_KEY, disp[(i++) & 0x03]);
        rt_thread_mdelay(300);
        if (rt_device_read(console, 0, &ch, 1) == 0) {
            ch = AT_CLI_NOP_KEY;
        }
    } while (ch != AT_CLI_ESC_KEY && atomic_read(&esp.atcli.enable) != 2);

    rt_kprintf("\n");
    if (ch != AT_CLI_ESC_KEY) {
        rt_device_write(esp.client->device, 0, "ATE1\r\n", 6);
        while (1) {
            while (rt_device_read(console, 0, &ch, 1) == 0) {
                rt_sem_take(&console_rx_notice, RT_WAITING_FOREVER);
            }
            if (ch == AT_CLI_ESC_KEY) {
                break;
            }
            if(ch == 0x0d){
                rt_device_write(esp.client->device, 0, "\r\n", 2);
            }else{
                rt_device_write(esp.client->device, 0, &ch, 1);
            }
        }
    }
    rt_kprintf("======== Exit AT client cli mode========\n");

    //exit at mode
    atomic_set(&esp.atcli.enable, 0);

    //restore console configure
    int_lvl = rt_hw_interrupt_disable();
    if (console) {
        console->open_flag = old_open_flag;
        /* restore RX indicate */
        if (old_rx_ind) {
            rt_device_set_rx_indicate(console, old_rx_ind);
        }
    }
    rt_hw_interrupt_enable(int_lvl);

    esp.atcli.rx_sem = RT_NULL;
    rt_sem_detach(&console_rx_notice);
}
FINSH_FUNCTION_EXPORT(espat, espat <client>)
MSH_CMD_EXPORT(espat, RT-Thread AT component cli: espat <client>);
/*
********************************************************************************
*Function    : esp_receive_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/   
static int esp_receive_process(at_client_t client)
{   
    monitor_t *monitor = &esp.monitor;
    
    if( monitor->recv_process != NULL ){
      
        monitor->recv_process(client);
        
    }else{
        //接收套接字信息
        at_client_recv(client);
        
        //清空信号量
        rt_sem_control(client->rx_notice, RT_IPC_CMD_RESET, 0);

        //再次接收套接字信息，以清空缓冲
        at_client_recv(client);
    }
    return 0;
}
/*
********************************************************************************
*Function    : esp_thread_entry
*Description : esp 线程入口
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void espat_thread_entry(at_client_t client)
{
    rt_tick_t timeout_tick;
    
    while (1) {
        timeout_tick = esp_status_monitor();
        
        if (esp_socket_process(client) == 0) {
            if (timeout_tick > esp_tick_from_ms(2000)) {
                timeout_tick = esp_tick_from_ms(2000);
            }
            rt_sem_take(client->rx_notice, timeout_tick);
        }
        
        esp_receive_process(client);
        
        esp_atcli_process(client);
    }
}
/*
********************************************************************************
*Function    : __esp_at_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int __esp_at_init(const char *dev_name)
{
    at_client_t client;

    //create at client
    client = at_client_create(dev_name, urc_table, sizeof(urc_table)/sizeof(urc_table[0]));
    if (client == RT_NULL) {
        LOG_E("AT client on device %s initialize failed.", dev_name);
        return -RT_ERROR;
    }
    esp.client = client;
    
    //message queue init
    at_mq_init(&esp.mq);
    
    //socket RX message queue init
    for (int i = 0; i < ESP_MAX_SOCKET; i++) {
        esp.sockets[i].connect_id = i;
        esp.sockets[i].local_port = ESP_BASE_PORT + i;
        at_mq_init(&esp.sockets[i].rx_mq);
    }
    atomic_set(&esp.atcli.enable, 0);
    //create thread
    esp.thread = rt_thread_create("esp",
                                   (void (*)(void *parameter))espat_thread_entry,
                                    client,
                                    1024 + 1024,
                                    7,
                                    5);
    if (esp.thread != RT_NULL){
        rt_thread_startup(esp.thread);
    }

    return RT_EOK;
}
/*
********************************************************************************
*Function    : esp_init
*Description :
*Input       : 
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int esp_init(void)
{
    memset(&esp, 0, sizeof(esp));

    esp.monitor.restart = 1;
    esp_power_3v();
    esp_en_chip();
    __esp_at_init("lpuart1");

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(esp_init);

