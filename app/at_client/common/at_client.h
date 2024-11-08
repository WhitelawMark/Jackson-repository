/*
 * at_client.h
 *
 *  Created on: 2018年11月12日
 *      Author: root
 */

#ifndef AT_CLIENT_H_
#define AT_CLIENT_H_

#include <rtthread.h>

#define AT_CMD_ACK_OK             0x00    /*获取到 AT指令应该返回的ACK*/
#define AT_CMD_ACK_ERROR          0xFF    /*获取到的 ACK为"ERROR"*/
#define AT_CMD_ACK_TIMEOUT        0x01    /*请求ACK或获取返回的数据超时*/
#define AT_CMD_SEND_FAIL          0x02    /*AT命令发送失败*/
#define AT_CMD_WRONG_ACKFMT       0x03    /*请求ACK的格式出错 */
#define AT_CMD_WRONG_ENDFLG       0x0A    /*特殊的结束符号标志的格式出错*/
#define AT_CMD_ACK_OVER_MAXLEN    0x0B    /*返回数据的长度超过最大长度 */
#define AT_CMD_DATA_GET_OK        0x80    /*获取到返回的数据*/


#define AT_CMD_ACK_MAX_SIZE       128

typedef rt_tick_t at_time_t;

typedef struct at_urc {
    const char *prefix;
    const char *suffix;
    void (*func)(const char *data, int len);
} at_urc_t;

typedef struct at_detector {
    at_time_t endtime;
    at_time_t waittime_tick;
    at_urc_t *curr_urc;
    at_urc_t *urc_table;
    int urc_table_size;
    int nesting;
    int len;
    unsigned char data[AT_CMD_ACK_MAX_SIZE+1+256];
} at_detector_t;

struct at_client {
    rt_device_t device;
    rt_sem_t rx_notice;
    at_detector_t detector;
};

typedef struct at_client* at_client_t;

int at_getchar(at_client_t client, unsigned char *ch, long tick, int detect);

int at_sendstr(at_client_t client, const unsigned char *str, unsigned int len);

at_client_t at_client_create(const char *dev_name, const at_urc_t *urc_table, int table_size);

void at_client_destory(at_client_t client);

void at_client_recv(at_client_t client);

int at_cmd_line_get(at_client_t client, char *suffix, unsigned char *xbuf, int xbuf_size, unsigned int timeout_ms);

int at_cmd_line(at_client_t client,
                const unsigned char *atcmd, int atcmdlen,
                const unsigned char *ack,
                unsigned char *xbuf, int xbuf_size,
                unsigned int timeout_ms, int det_err);

int at_cmd_send(at_client_t client,const unsigned char *atcmd, int atcmdlen);

#endif /* AT_CLIENT_H_ */
