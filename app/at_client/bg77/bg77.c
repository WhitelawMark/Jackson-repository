/*
 * bg77.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */

#include <string.h>
#include <stdlib.h>
#include "rtthread.h"
#include "rthw.h"
#include "drivers/pin.h"
#include "at_client.h"
#include "bg77.h"
#include "bg77_cmd.h"
#include "bg77_ftp.h"
#include "bg77_https.h"

/*
********************************************************************************
*
********************************************************************************
*/
#define DBG_TAG "bg77"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
*
********************************************************************************
*/
#define BG77_OFF      0
#define BG77_ON       (!BG77_OFF)

/*
********************************************************************************
*
********************************************************************************
*/
static void bg77_thread_notify(int evtype);
static void bg77_run_reset(void);

static void bg77_urc_qiurc(const char *data, int len);
void bg77_urc_qmtrecv(const char *data, int len);
void bg77_urc_qmtstat(const char *data, int len);
/*
********************************************************************************
*
********************************************************************************
*/
static const at_urc_t urc_table[] = {
    /*prefix, suffix, func*/
    {"+QIURC:",   "\r\n",   bg77_urc_qiurc  },  //The URC of TCP/IP AT commands
    {"+QMTRECV:", "\",",    __bg77_urc_qmtrecv},  //Receive MQTT packet data
    {"+QMTSTAT:", "\r\n",   __bg77_urc_qmtstat}   //MQTT Link Layer State Change indicator
};

at_manage_t bg77;
/*
********************************************************************************
*Function    : bg77_wakeup_ctrl
*Description : bg77 休眠控制
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void bg77_wakeup_ctrl(int power)
{
    if(power == BG77_NORMEL || power == BG77_SLEEP){
         bg77.monitor.powersave = power;
    }
}

/*
********************************************************************************
*Function    : bg77_airmode_config
*Description : bg77 飞行模式
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void bg77_airmode_config(int mode)
{
    if(mode == 0 || mode == 1){
         bg77.monitor.airmode = mode;
    }
}
/*
********************************************************************************
*Function    : bg77_power_ctrl
*Description : bg77 电源控制
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_power_ctrl(int flag)
{
    rt_pin_write(BG77_POWER_PIN, PIN_HIGH);
    rt_thread_delay(RT_TICK_PER_SECOND*3);
    rt_pin_write(BG77_POWER_PIN, PIN_LOW);
    rt_thread_delay(RT_TICK_PER_SECOND*1);
    
    //while (rt_pin_read(BG77_STATUS_PIN) != PIN_LOW);
    
    if (flag == BG77_OFF) {
        rt_pin_write(BG77_POWER_PIN, PIN_HIGH);
    } else {
        rt_pin_write(BG77_POWER_PIN, PIN_LOW);
    }
}
/*
********************************************************************************
*Function    : bg77_reset
*Description : bg77 硬件复位
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_reset(void)
{
    rt_pin_write(BG77_RESET_PIN, PIN_HIGH);
    rt_thread_mdelay(460);
    rt_pin_write(BG77_RESET_PIN, PIN_LOW);
    rt_thread_mdelay(5000);
}
/*
********************************************************************************
*Function    : bg77_sleep
*Description : bg77 休眠
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void bg77_sleep(void)
{
    rt_pin_write(BG77_DTR_PIN, PIN_HIGH);
}
/*
********************************************************************************
*Function    : bg77_wakeup
*Description : bg77 唤醒
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void bg77_wakeup(void)
{
    rt_pin_write(BG77_DTR_PIN, PIN_LOW);
    rt_thread_mdelay(RT_TICK_PER_SECOND/50);
}
/*
********************************************************************************
*Function    : bg77_restart
*Description : bg77 重启电源
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_restart(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (bg77_cmd_line("AT+CFUN=1,1\r", "OK", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {
            return ;
        }
    }
    return ;
}
/*
********************************************************************************
*Function    : bg77_module_init
*Description : bg77 模块初始化
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_module_init(void)
{
    //初始化模块控制引脚
    rt_pin_mode(BG77_POWER_PIN,  PIN_MODE_OUTPUT);
    rt_pin_mode(BG77_RESET_PIN,  PIN_MODE_OUTPUT);
   // rt_pin_mode(BG77_DTR_PIN,    PIN_MODE_OUTPUT);

//    rt_pin_mode(BG77_STATUS_PIN, PIN_MODE_INPUT_PULLUP);
 
    bg77.monitor.powersave = BG77_NORMEL;
    bg77.monitor.restart = 1;
}
/*
********************************************************************************
*Function    : bg77_config_init
*Description : BG77 配置初始化
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功
               -1  -- 配置初始化失败
*Others      :
********************************************************************************
*/
static int bg77_config_init(void)
{
    int i,j;
    static const struct  {
        const char *cmd;
        const char *ack;
        unsigned int timeout_ms;
    } table[3] = {
        "ATE1\r",     "OK",  2000, //关闭回显功能
        "AT+QICSGP=1,1,\"CMNBIOT\",\"\",\"\",1\r",     "OK",  2000, // 
        "AT+QIACT=1\r",     "OK",  5000
    };
    

    for (i = 0; i < sizeof(table)/sizeof(table[0]); i++) {
        //每条指令最多执行2次
        for (j = 2; j > 0; j--) {
            if (bg77_cmd_line(table[i].cmd, table[i].ack, 0, 0, table[i].timeout_ms, 1) == AT_CMD_ACK_OK) {
                break;
            }
            if (j == 1) {
                LOG_E("Command: \"%s\" execute fail\n", table[i].cmd);
                return -1;
            }
        }
    }

    return 0;
}
/*
********************************************************************************
*Function    : bg77_socket_receive
*Description :
*Input       :
*Output      :
*Return      : >= 0 size of data receive, -1 if fail.
*Others      :
********************************************************************************
*/
static int bg77_socket_receive(int sd, at_addr_t *addr, char *buf, int size)
{
    char *str;
    int i, len;
    char atcmd[32];
    char xbuf[48];

    if (size <= 0) {
        return -1;
    }

    at_snprintf(atcmd, sizeof(atcmd), "AT+QIRD=%d,%d\r", sd, size);
    atcmd[sizeof(atcmd)-1] = '\0';

    //+QIRD: 4,"47.92.74.102",20190\r\n OR +QIRD: 4\r\n
    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "+QIRD:", xbuf, sizeof(xbuf), 2000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    str = xbuf;

    //length
    len = at_atoi(str);
    if (len <= 0) {
        return -1;
    }

    /*skip length*/
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    //address
    if (addr) {
        at_memset(addr, 0, sizeof(*addr));

        //skip '"'
        while (*str && *str != '\"') str++;
        if (*str) str++; //skip '"'

        //IP
        i = 0;
        while (*str && *str != '\"') {
            if (i < sizeof(addr->ip)-1) {
                addr->ip[i++] = *str;
            }
            str++;
        }

        //skip IP
        while (*str && *str != ',') str++;
        if (*str) str++; //skip ','

        //PORT
        if (*str) {
            addr->port = at_atoi(str);
        }
    }

    //data
    if (len > size) {
        len = size;
    }
    len = bg77_cmd_line_get(0, buf, len, 2000);

    return len;
}
/*
********************************************************************************
*Function    : bg77_signal_detect
*Description :
*Input       :
*Output      :
*Return      : 0 if detect OK, -1 if fail.
*Others      :
********************************************************************************
*/
static int bg77_signal_detect(void)
{
    if (bg77_csq_detect() != 0) {
        return -1;
    }

    if (bg77_ps_attach_detect() != 0) {
        return -1;
    }
    return 0;
}
/*
********************************************************************************
*Function    : bg77_is_initok
*Description : bg77 檢測是否初始化成功
*Input       :
*Output      :
*Return      :  
*Others      :
********************************************************************************
*/
int bg77_is_initok(void)
{
    return bg77.monitor.init_flag;
}
/*
********************************************************************************
*Function    : bg77_status_monitor
*Description : bg77 模块状态监测
*Input       :
*Output      :
*Return      : 下次检测等待的时间
*Others      :
********************************************************************************
*/
static rt_tick_t bg77_status_monitor(void)
{
    monitor_t *monitor = &bg77.monitor;
    rt_tick_t timeout_tick;
    
    //飞行模式
    if (monitor->airmode) {
        timeout_tick = bg77_tick_from_ms(60000);
        bg77_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    }
    //休眠模式处理
    if (monitor->powersave) {
        if (rt_pin_read(BG77_POWER_PIN)) {
            bg77_power_ctrl(BG77_OFF);
            monitor->restart = 1;
        }
        //10S 后再次进行初始化
        timeout_tick = bg77_tick_from_ms(10000);
        bg77_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    }
    //系统重新初始化    
    if (monitor->restart) {
        monitor->restart = 0;

        LOG_D("BG77 module restart...");

        //BG77模块重启电源
        bg77_restart();
        //硬件复位
        bg77_reset();

        //运行状态重置
        bg77_run_reset();

        //需要重新初始化
        monitor->init_flag = 0;
        monitor->signal_ready = 0;
        timeout_tick = bg77_tick_from_ms(3000);
        bg77_timer_start(monitor->timer, timeout_tick);

        LOG_D("BG77 module restart OK!");
    }

    //已初始化
    if (monitor->init_flag) {
        if (bg77_is_timer_end(monitor->timer, timeout_tick)) {
            //检测信号信息
            if (bg77_signal_detect() != 0) {
                monitor->fail_times++;
            } else {
                monitor->signal_ready = 1;
                monitor->fail_times = 0;
            }

            if (monitor->fail_times >= 3) {
                monitor->restart = 1;
                timeout_tick = 0;
            } else {
                //每180S 检测一次
                timeout_tick = bg77_tick_from_ms(180000);
                bg77_timer_start(monitor->timer, timeout_tick);
            }
        } else {
            timeout_tick = 0 - timeout_tick;
        }
        return timeout_tick;
    }

    //时间未到
    if (!bg77_is_timer_end(monitor->timer, timeout_tick)) {
        return (0 - timeout_tick);
    }

    //1. AT命令响应检测
    if (bg77_at_resp_detect() != 0) {
        LOG_E("AT command response fail");
        //5S 后再次进行初始化
        timeout_tick = bg77_tick_from_ms(5000);
        bg77_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    }

    //2. 配置初始化
    if (bg77_config_init() != 0) {
        LOG_E("BG77 configure fail");
        //3S 后再次进行初始化
        timeout_tick = bg77_tick_from_ms(3000);
        bg77_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    }

    //3. SIM卡识别检测
    if (bg77_sim_card_detect() != 0) {
        LOG_E("SIM card detect fail");
        //5S 后再次进行初始化
        timeout_tick = bg77_tick_from_ms(5000);
        bg77_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    }
    //4.检测信号信息
    if (bg77_signal_detect() != 0) {
        timeout_tick = bg77_tick_from_ms(180000);
        bg77_timer_start(monitor->timer, timeout_tick);
        return timeout_tick;
    } 
    //初始化完成
    monitor->init_flag = 1;
    monitor->fail_times = 0;

    //120S 后进行信号检测
    timeout_tick = bg77_tick_from_ms(120000);
    bg77_timer_start(monitor->timer, timeout_tick);

    return timeout_tick;
}

/*
********************************************************************************
*Function    : bg77_open
*Description :
*Input       :
*Output      :
*Return      : >= 0 if socket, -1 operate if fail, < -1 other error
*Others      :
********************************************************************************
*/
static int bg77_open(int type, int connect_id, unsigned short local_port, sockaddr_t *remote_addr)
{
    int sd = -2;

    switch (type) {
        case SOCK_DGRAM:
        case SOCK_STREAM:
            sd = __bg77_socket_open(type, connect_id, local_port, &remote_addr->addr);
            break;
        case SOCK_MQTT:
            sd = bg77_mqtt_open(connect_id, &remote_addr->addr,
                    remote_addr->clientid, remote_addr->username, remote_addr->password);
            break;
        default:
            break;
    }

    return sd;
}

/*
********************************************************************************
*Function    : __bg77_close
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int __bg77_close(socket_t *socket)
{
    int ret = -1;

    if (socket->sd >= 0) {
        switch (socket->type) {
            case SOCK_DGRAM:
            case SOCK_STREAM:
                __bg77_socket_close(socket->sd);
                socket->sd = -1;
                ret = 0;
                break;
            case SOCK_MQTT:
                __bg77_qmt_close(socket->sd);
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
*Function    : __bg77_connect
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 fail.
*Others      :
********************************************************************************
*/
int __bg77_connect(socket_t *socket, connect_msg_t *msg, void (*notify)(int evtype))
{
    if (socket->sd >= 0) {
        __bg77_close(socket);
    }

    if (msg != RT_NULL) {
        at_memcpy(&socket->remote_addr, &msg->remote_addr, sizeof(socket->remote_addr));
        socket->remote_addr.addr.ip[sizeof(socket->remote_addr.addr.ip)-1] = '\0';
        socket->remote_addr.clientid[sizeof(socket->remote_addr.clientid)-1] = '\0';
        socket->remote_addr.username[sizeof(socket->remote_addr.username)-1] = '\0';
        socket->remote_addr.password[sizeof(socket->remote_addr.password)-1] = '\0';
    }

    socket->sd = bg77_open(socket->type, socket->connect_id, socket->local_port, &socket->remote_addr);
    if (socket->sd < 0) {
        if (socket->sd == -1) {
            socket->fail_count++;
            LOG_E("socket open fail(fail count %d)", socket->fail_count);
        }
        return -1;
    }
    socket->fail_count = 0;
    
    LOG_E("connect success");


    //connect notify
    if (notify) {
        notify(EV_CONNECT);
    }

    return 0;
}


/*
********************************************************************************
*Function    : bg77_get_socket
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
socket_t *bg77_get_socket(int sd, int type)
{
    socket_t *socket;
    int i;

    for (i = 0; i < BG77_MAX_SOCKET; i++) {
        socket = &bg77.sockets[i];
        if (atomic_read(&socket->used) == 1 &&
                (type & socket->type) == socket->type && sd == socket->sd) {
            return socket;
        }
    }

    return RT_NULL;
}

/*
********************************************************************************
*Function    : bg77_urc_parse
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_urc_parse(const char *name, char *value)
{
    if (at_strcmp(name, "\"closed\"") == 0) {
        if (*value) {
            socket_t *socket;
            LOG_E("connectId %s close detected", value);
            socket = bg77_get_socket(at_atoi(value), SOCK_DGRAM | SOCK_STREAM);
            if (socket != RT_NULL) {
                __bg77_close(socket);
            }
        }
    }
}

/*
********************************************************************************
*Function    : bg77_urc_qiurc
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_urc_qiurc(const char *data, int len)
{
    char *str = (char *)data;
    char *name, *ptr;

    (void)len;

    //skip name
    name = str;
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','

    while (at_isspace(*name)) name++;

    ptr = name + at_strlen(name) - 1;
    while (ptr > name && at_isspace(*ptr)) {
        *ptr-- = '\0';
    }

    bg77_urc_parse(name, str);
}


/*
********************************************************************************
*Function    : bg77_run_reset
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_run_reset(void)
{
    socket_t *socket;
    int i;

    for (i = 0; i < BG77_MAX_SOCKET; i++) {
        socket = &bg77.sockets[i];
        if (atomic_read(&socket->used) == 1) {
            __bg77_close(socket);
            //simple sd = -1 ?
        }
    }
}

/*
********************************************************************************
*Function    : bg77_socket_detect
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_socket_detect(void)
{
    socket_t *socket;
    void *msg;
    int i;
    int used;

    for (i = 0; i < BG77_MAX_SOCKET; i++) {
        socket = &bg77.sockets[i];
        used = atomic_read(&socket->used);
        if (used == 2) {
            //close socket
            __bg77_close(socket);
            //free all RX message
            while ((msg = at_message_get(&socket->rx_mq)) != RT_NULL) {
                at_message_free(msg);
            }
            atomic_set(&socket->used, 0);
        } else if (used == 1) {
            if (socket->sd < 0) {
                //连接断开不自动重连
                //bg77_connect(socket, RT_NULL, (socket->type == SOCK_MQTT)? socket->notify:RT_NULL);
            }
            if (socket->fail_count > 9) {
                socket->fail_count = 0;
                bg77.monitor.restart = 1;
                LOG_E("BG77 socket fail detected, force to restart module");
            }
        }
    }
}

/*
********************************************************************************
*Function    : bg77_socket_msg_process
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail
*Others      :
********************************************************************************
*/
static int bg77_socket_msg_process(socket_msg_t *msg)
{
    socket_t *socket;
    int res = -1;

    if (msg->sd < 0 || msg->sd >= BG77_MAX_SOCKET) {
        LOG_E("Socket %d out of range", msg->sd);
    } else {
        socket = &bg77.sockets[msg->sd];
        if (atomic_read(&socket->used) == 1) {
            switch (msg->type) {
            case MSG_UDP:
                res = __bg77_udp_send(socket, (udp_msg_t *)(msg->msg));
                break;
            case MSG_TCP:
                res = __bg77_tcp_send(socket, (tcp_msg_t *)(msg->msg));
                break;
            case MSG_MQTT_SUB:
                res = __bg77_mqtt_subscribe(socket, (sub_msg_t *)(msg->msg));
                break;
            case MSG_MQTT_UNS:
                res = __bg77_mqtt_unsubscribe(socket, (sub_msg_t *)(msg->msg));
                break;
            case MSG_MQTT_PUB:
                res = __bg77_mqtt_publish(socket, (mqtt_msg_t *)(msg->msg));
                break;
            case MSG_CONNECT:
                res = __bg77_connect(socket, (connect_msg_t *)(msg->msg), RT_NULL);
                break;
            case MSG_FILE:
                res = __bg77_file_operation(socket, (file_msg_t *)(msg->msg));
                break;
            case MSG_CERT_UPLOAD:
                res = __bg77_cert_upload(socket, (cart_msg_t *)(msg->msg));
                break;
            case MSG_FTP_DOWNLOAD:
                res = __bg77_ftp_download(socket, (ftp_msg_t *)(msg->msg));
                break;
            case MSG_FTP_UPLOAD:
                res = __bg77_ftp_upload(socket, (ftp_msg_t *)(msg->msg));
                break;
            case MSG_HTTP_DOWNLOAD:
                res = __bg77_https_download(socket, (https_msg_t *)(msg->msg));
                break;
            case MSG_HTTP_UPLOAD:
                res = __bg77_https_upload(socket, (https_msg_t *)(msg->msg));
                break;
                
            default:
                break;
            }
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
*Function    : bg77_socket_msg_clean
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_socket_msg_clean(void)
{
    socket_msg_t *msg;

    while ((msg = at_message_get(&bg77.mq)) != RT_NULL) {
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
*Function    : bg77_socket_msg_accept
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int bg77_socket_msg_accept(void)
{
    socket_msg_t *msg;
    int fail_cnt = 0;

    while ((msg = at_message_get(&bg77.mq)) != RT_NULL) {
        if (bg77_socket_msg_process(msg) != 0) {
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
*Function    : bg77_socket_sync_result
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_socket_sync_result(int res, void *arg)
{
    resp_msg_t *resp = (resp_msg_t *)arg;

    resp->result = res;

    //response semaphore
    rt_sem_release(&resp->resp_sem);
}

/*
********************************************************************************
*Function    : bg77_socket_ignore_result
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void bg77_socket_ignore_result(int res, void *arg)
{
    (void)res;
    (void)arg;
}

/*
********************************************************************************
*Function    : bg77_socket_msg_post
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 fail.
*Others      :
********************************************************************************
*/
int __bg77_socket_msg_post(socket_msg_t *msg)
{
    int res;

    if (rt_thread_self() == bg77.thread) {
         msg->res_cb = RT_NULL;
         msg->arg = RT_NULL;
         res = bg77_socket_msg_process(msg);
    } else {
        if (msg->res_cb) {
            at_message_put(&bg77.mq, msg, bg77_thread_notify, EV_TX);
            res = 0;
        } else {
            resp_msg_t resp;
            resp.result = 0;
            rt_sem_init(&resp.resp_sem, "resp", 0, RT_IPC_FLAG_FIFO);

            msg->res_cb = bg77_socket_sync_result;
            msg->arg = (void *)&resp;
            at_message_put(&bg77.mq, msg, bg77_thread_notify, EV_TX);

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
*Function    : bg77_socket_delete
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
void bg77_socket_delete(int sd)
{
    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return;
    }
    if (atomic_cmpxchg(&bg77.sockets[sd].used, 1, 2) == 1) {
        bg77_thread_notify(EV_TX);
    }
}

/*
********************************************************************************
*Function    : bg77_create_socket
*Description :
*Input       :
*Output      :
*Return      : socket
*Others      :
********************************************************************************
*/
int bg77_socket_create(int type)
{
    socket_t *socket;
    int sd;

    if (type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_MQTT && type !=SOCK_FTP) {
        return -1;
    }

    //get free socket
    for (sd = 0; sd < BG77_MAX_SOCKET; sd++) {
        if (atomic_cmpxchg(&bg77.sockets[sd].used, 0, -1) == 0) {
            break;
        }
    }
    if (sd == BG77_MAX_SOCKET) {
        return -1;
    }
    socket = &bg77.sockets[sd];

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
*Function    : bg77_tcp_recv
*Description :
*Input       :
*Output      :
*Return      : length
*Others      :
********************************************************************************
*/
at_ssize_t bg77_tcp_recv(int sd, void *buf, at_size_t len)
{
    socket_t *socket;
    tcp_msg_t *msg;
    at_ssize_t ssize;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }
    socket = &bg77.sockets[sd];

    while ((msg = at_message_get(&socket->rx_mq)) != RT_NULL) {
        if (at_message_type(msg) == SOCK_STREAM) {
            if (msg->len < len) {
                ssize = msg->len;
            } else {
                ssize = len;
            }
            if (ssize > 0) {
                at_memcpy(buf, msg->buf, ssize);
            }
            at_message_free(msg);
            return ssize;
        }
        at_message_free(msg);
    }

    return -1;
}

/*
********************************************************************************
*Function    : bg77_udp_recv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
at_ssize_t bg77_udp_recv(int sd, at_addr_t *ipaddr, void *buf, at_size_t len)
{
    socket_t *socket;
    udp_msg_t *msg;
    at_ssize_t ssize;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }
    socket = &bg77.sockets[sd];

    while ((msg = at_message_get(&socket->rx_mq)) != RT_NULL) {
        if (at_message_type(msg) == SOCK_DGRAM) {
            if (msg->len < len) {
                ssize = msg->len;
            } else {
                ssize = len;
            }
            if (ssize > 0) {
                at_memcpy(buf, msg->buf, ssize);
            }
            if (ipaddr != RT_NULL) {
                at_memcpy(ipaddr, &msg->remote_addr, sizeof(at_addr_t));
                ipaddr->ip[sizeof(ipaddr->ip)-1] = '\0';
            }
            at_message_free(msg);
            return ssize;
        }
        at_message_free(msg);
    }

    return -1;
}

/*
********************************************************************************
*Function    : bg77_mqtt_recv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
at_ssize_t bg77_mqtt_recv(int sd, int *msgid, void *topic, at_size_t topic_len, void *payload, at_size_t payload_len)
{
    socket_t *socket;
    mqtt_msg_t *msg;
    at_ssize_t ssize;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }
    socket = &bg77.sockets[sd];

    while ((msg = at_message_get(&socket->rx_mq)) != RT_NULL) {
        if (at_message_type(msg) == SOCK_MQTT) {
            //MsgId
            if (msgid) {
                *msgid = msg->msgid;
            }
            //topic
            if (topic && topic_len > 0) {
                topic_len -= 1;
                if (topic_len > msg->topic_len) {
                    topic_len = msg->topic_len;
                }
                at_memcpy(topic, msg->topic, topic_len);
                ((char *)topic)[topic_len] = '\0';
            }
            //payload
            if (payload_len > msg->payload_len) {
                ssize = msg->payload_len;
            } else {
                ssize = payload_len;
            }
            at_memcpy(payload, msg->payload, ssize);
            at_message_free(msg);
            return ssize;
        }
        at_message_free(msg);
    }

    return -1;
}

/*
********************************************************************************
*Function    : bg77_set_notify
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void bg77_set_notify(int sd, void (*notify)(int evtype))
{
    socket_t *socket;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return;
    }
    socket = &bg77.sockets[sd];

    socket->notify = notify;
}

/*
********************************************************************************
*Function    : bg77_set_recv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void bg77_set_recv(int sd, void (*recv)(int sd, char *buf, int len))
{
    socket_t *socket;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return;
    }
    socket = &bg77.sockets[sd];

    __atomic_set((volatile long *)&socket->recv, (long)recv);
}

/*
********************************************************************************
*Function    : bg77_get_iccid
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
const char *bg77_get_iccid(void)
{
    return bg77.iccid;
}

/*
********************************************************************************
*Function    : bg77_get_imei
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
const char *bg77_get_imei(void)
{
    return bg77.imei;
}
/*
********************************************************************************
*Function    : bg77_get_model
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
const char *bg77_get_model(void)
{
    return bg77.model;
}

/*
********************************************************************************
*Function    : bg77_get_rssi
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
char bg77_get_rssi(void)
{
    return bg77.rssi;
}

/*
********************************************************************************
*Function    : bg77_is_connected
*Description :
*Input       :
*Output      :
*Return      : 0 if not, 1 if connected
*Others      :
********************************************************************************
*/
int bg77_is_connected(int sd)
{
    socket_t *socket;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return 0;
    }

    socket = &bg77.sockets[sd];

    //socket not used
    if (atomic_read(&socket->used) != 1) {
        return 0;
    }

    if (socket->sd < 0) {
        return 0;
    }

    return 1;
}
/*
********************************************************************************
*Function    : bg77_socket_recv
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
static int bg77_socket_recv(socket_t *socket)
{
    at_addr_t addr;
    char *buf = bg77.tmpbuf;
    int len = sizeof(bg77.tmpbuf);

    len = bg77_socket_receive(socket->sd, &addr, buf, len);
    if (len <= 0) {
        return -1;
    }

    __bg77_socket_recv(socket, &addr, buf, len);

    return 0;
}

/*
********************************************************************************
*Function    : bg77_receive
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_receive(socket_t *socket)
{
    if (atomic_read(&socket->used) != 1) {
        return;
    }

    switch (socket->type) {
        case SOCK_DGRAM:
        case SOCK_STREAM:
            while (bg77_socket_recv(socket) == 0);
            break;
        case SOCK_MQTT:
            break;
        default:
            break;
    }
}

/*
********************************************************************************
*Function    : bg77_receive_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_receive_process(at_client_t client)
{
    int i;

    //接收套接字信息
    at_client_recv(client);

    for (i = 0; i < BG77_MAX_SOCKET; i++) {
        bg77_receive(&bg77.sockets[i]);
    }

    //清空信号量
    rt_sem_control(client->rx_notice, RT_IPC_CMD_RESET, 0);

    //再次接收套接字信息，以清空缓冲
    at_client_recv(client);
}

/*
********************************************************************************
*Function    : bg77_socket_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int bg77_socket_process(at_client_t client)
{
    (void)client;

    //socket 状态检测
    bg77_socket_detect();

    //socket 消息处理
    return bg77_socket_msg_accept();
}

/*
********************************************************************************
*Function    : bg77_thread_notify
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_thread_notify(int evtype)
{
   (void)evtype;

   rt_sem_release(bg77.client->rx_notice);
}
/*
********************************************************************************
*Function    : bg77_cli_process
*Description : AT命令行 处理
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_atcli_process(at_client_t client)
{
    if (atomic_cmpxchg(&bg77.atcli.enable, 1, 2) == 1) {
        rt_device_t console = rt_console_get_device();
        char ch;

		while (atomic_read(&bg77.atcli.enable) == 2) {
            //socket 消息清除
            bg77_socket_msg_clean();

            rt_sem_take(client->rx_notice, RT_WAITING_FOREVER);

            while( rt_device_read(client->device, 0, &ch, 1) == 1){
               rt_device_write(console, 0, &ch, 1);
            }
        }

        //exit CLI mode, restart module
        bg77.monitor.restart = 1;
    }
}

/*
******************************************************************************** 
*Function    : at
*Description : AT命令
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
static rt_err_t console_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(bg77.atcli.rx_sem);

    return RT_EOK;
}

static void at(int argc, char **argv)
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
    bg77.atcli.rx_sem = &console_rx_notice;

    //backup console configure
    int_lvl = rt_hw_interrupt_disable();
    console = rt_console_get_device();//获取控制台设备的句柄
    if (console) {
        old_open_flag = console->open_flag;
        console->open_flag &= (~RT_DEVICE_FLAG_STREAM);//流模式位清零
        /* backup RX indicate */
        old_rx_ind = console->rx_indicate;
        rt_device_set_rx_indicate(console, console_rx_ind);
    }
    rt_hw_interrupt_enable(int_lvl);

    //entry to at mode
    atomic_set(&bg77.atcli.enable, 1);
    bg77_thread_notify(EV_TX);

    rt_kprintf("======== Welcome to using AT command client cli, press 'ESC' to exit========\n");
    rt_kprintf("Entry AT client cli mode  ");
    i = 0;
    do {
        rt_kprintf("%c%c",AT_CLI_BACKSPACE_KEY, disp[(i++) & 0x03]);
        rt_thread_mdelay(300);
        if (rt_device_read(console, 0, &ch, 1) == 0) {
            ch = AT_CLI_NOP_KEY;
        }
    } while (ch != AT_CLI_ESC_KEY && atomic_read(&bg77.atcli.enable) != 2);
    rt_kprintf("\n");
    if (ch != AT_CLI_ESC_KEY) {
        rt_device_write(bg77.client->device, 0, "ATE1\r", 5);
        while (1) {
            while (rt_device_read(console, 0, &ch, 1) == 0) {
                rt_sem_take(&console_rx_notice, RT_WAITING_FOREVER);
            }
            if (ch == AT_CLI_ESC_KEY) {
                break;
            }
            rt_device_write(bg77.client->device, 0, &ch, 1);
        }
    }
    rt_kprintf("======== Exit AT client cli mode========\n");

    //exit at mode
    atomic_set(&bg77.atcli.enable, 0);

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

    bg77.atcli.rx_sem = RT_NULL;
    rt_sem_detach(&console_rx_notice);
}

FINSH_FUNCTION_EXPORT(at, at <client>)
MSH_CMD_EXPORT(at, RT-Thread AT component cli: at <client>);

/*
********************************************************************************
*Function    : bg77_thread_entry
*Description : bg77 线程入口
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void bg77_thread_entry(at_client_t client)
{
    rt_tick_t timeout_tick;

    bg77_module_init();

    while (1) {
        timeout_tick = bg77_status_monitor();

        //处理套接字
        if (bg77_socket_process(client) == 0) {
            //等待信号量
            if (timeout_tick > bg77_tick_from_ms(2000)) {
                timeout_tick = bg77_tick_from_ms(2000);
            }
            rt_sem_take(client->rx_notice, timeout_tick);
        }

        //数据接收处理
        bg77_receive_process(client);
        
        //AT命令行处理
        bg77_atcli_process(client);
    }
}

/*
********************************************************************************
*Function    : __bg77_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int __bg77_init(const char *dev_name)
{
    at_client_t client;
    int i;

    //create at client
    client = at_client_create(dev_name, urc_table, sizeof(urc_table)/sizeof(urc_table[0]));
    if (client == RT_NULL) {
        LOG_E("AT client on device %s initialize failed.", dev_name);
        return -RT_ERROR;
    }
    bg77.client = client;

    //message queue init
    at_mq_init(&bg77.mq);

    //socket RX message queue init
    for (i = 0; i < BG77_MAX_SOCKET; i++) {
        bg77.sockets[i].connect_id = i;
        bg77.sockets[i].local_port = BG77_BASE_PORT + i;
        at_mq_init(&bg77.sockets[i].rx_mq);
    }
 
    //disable AT CLI
    atomic_set(&bg77.atcli.enable, 0);

    //create thread
    bg77.thread = rt_thread_create("bg77",
                                   (void (*)(void *parameter))bg77_thread_entry,
                                    client,
                                    1024 + 512,
                                    RT_THREAD_PRIORITY_MAX / 3 - 1,
                                    5);
    if (bg77.thread != RT_NULL){
        rt_thread_startup(bg77.thread);
    }

    return RT_EOK;
}

/*
********************************************************************************
*Function    : bg77_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int bg77_init(void)
{
    at_memset(&bg77, 0, sizeof(bg77));
    __bg77_init("uart4");
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(bg77_init);

