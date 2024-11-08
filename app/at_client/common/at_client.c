/*
 * at_client.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include <string.h>
#include <rtdbg.h>
#include "at_client.h"


#define DBG_TAG "at"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>

#define AT_TIME_NOTOUT                  0
#define at_tick_from_ms(ms)             rt_tick_from_millisecond((rt_int32_t)(ms))
#define at_timeout_set(endtime, tick)   ((endtime) = (rt_tick_get() + (tick)))
#define at_timeout_det(endtime, tick)   ((unsigned long)(endtime-rt_tick_get()) > (tick))
#define at_wait_time(waittime, endtime) (waittime = (unsigned long)(endtime-rt_tick_get()))

#ifdef AT_CLIENT_NUM_MAX
#undef AT_CLIENT_NUM_MAX
#endif
#define AT_CLIENT_NUM_MAX               2

static struct at_client at_client_table[AT_CLIENT_NUM_MAX] = { 0 };

static void at_urc_detect(at_client_t client, unsigned char data);

extern void ec600_sleep(void);
extern void ec600_wakeup(void);
/*
********************************************************************************
*Function    : at_sendstr                                                       
*Description :                                                                  
*Input       :                                                                  
*Output      :                                                                  
*Return      : 0 if success, -1 if fail.                                        
*Others      :                                                                  
********************************************************************************
*/
int at_sendstr(at_client_t client, const unsigned char *str, unsigned int len)
{
    
    if (rt_device_write(client->device, 0, str, len) == len) {
        
        return 0;
    }
    
    return -1;
}

/*
********************************************************************************
*Function    : at_getchar
*Description :
*Input       :
               @tick:  > 0 wait tick, <= 0 no wait
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int at_getchar(at_client_t client, unsigned char *ch, long tick, int detect)
{
    if (rt_device_read(client->device, 0, ch, 1) == 1) {
        
        if (detect) {
            at_urc_detect(client, *ch);
        }
        return 0;
    }

    if (tick > 0) {
        rt_sem_take(client->rx_notice, tick);
        if (rt_device_read(client->device, 0, ch, 1) == 1) {
            if (detect) {
                at_urc_detect(client, *ch);
            }
            return 0;
        }
    }

    return -1;
}

/*
********************************************************************************
*Function    : at_rx_ind
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static rt_err_t at_rx_ind(rt_device_t dev, rt_size_t size)
{
    int idx;

    for (idx = 0; idx < AT_CLIENT_NUM_MAX; idx++) {
        if (at_client_table[idx].device == dev && size > 0) {
            rt_sem_release(at_client_table[idx].rx_notice);
            break;
        }
    }
    return RT_EOK;
}

/*
********************************************************************************
*Function    : at_reset_detector
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void at_reset_detector(at_detector_t *detector)
{
    detector->curr_urc = (at_urc_t *)0;
    detector->len = 0;
}

/*
********************************************************************************
*Function    : at_detector_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void at_detector_init(at_detector_t *detector, const at_urc_t *urc_table, int table_size)
{
    at_reset_detector(detector);

    detector->nesting = 0;

    detector->urc_table = (at_urc_t *)urc_table;

    detector->urc_table_size = table_size;
}

/*
********************************************************************************
*Function    : at_urc_detect
*Description : unsolicited result code detect
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void at_urc_detect(at_client_t client, unsigned char data)
{
    at_detector_t *det = &client->detector;
    at_urc_t *urc;
    const char *suffix;
    int i, len;

    if (det->nesting) {
        return;
    }

    //缓冲区满则进行调整
    if (det->len >= sizeof(det->data)) {
        if (det->curr_urc) {
            det->curr_urc = (at_urc_t *)0;
            det->len = 0;
        } else {
            memmove(det->data, det->data+(sizeof(det->data)-AT_CMD_ACK_MAX_SIZE), AT_CMD_ACK_MAX_SIZE);
            det->len = AT_CMD_ACK_MAX_SIZE;
        }
    }
    det->data[det->len++] = data;


    //侦测并接收URC
    if (det->curr_urc) {
        urc = det->curr_urc;
        len = urc->suffix ? strlen(urc->suffix) : 0;
        if (len > 0) {
            suffix = urc->suffix;
        } else {
            suffix = "\r\n";
            len = 2;
        }
        if (det->len >= len && memcmp(&det->data[det->len - len], suffix, len) == 0) {
            det->len -= len;
            det->data[det->len] = '\0';
            det->nesting = 1;
            urc->func((char *)det->data, det->len);
            det->nesting = 0;
            det->curr_urc = (at_urc_t *)0;
            det->len = 0;
        } else {
            //接收超时,重新侦测
            if (at_timeout_det(det->endtime, det->waittime_tick) != AT_TIME_NOTOUT) {
                det->curr_urc = (at_urc_t *)0;
                det->len = 0;
            }
        }
    } else {
        for (i = 0; i < det->urc_table_size; i++) {
            urc = &det->urc_table[i];
            len = (urc->prefix ? strlen(urc->prefix) : 0);
            if (len > 0 && det->len >= len && memcmp(&det->data[det->len - len], urc->prefix, len) == 0) {
                det->len = 0;
                det->curr_urc = urc;
                det->waittime_tick = (rt_time_t)at_tick_from_ms(1000);
                at_timeout_set(det->endtime, det->waittime_tick);
                break;
            }
        }
        //line end detect
        if (data == '\n' && det->len >= 2 && det->data[det->len-2] == '\r') {
            det->len = 0;
        }
    }
}

/*
********************************************************************************
*Function    : at_clear_rxbuf
*Description : 清除接收缓冲区
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void at_clear_rxbuf(at_client_t client)
{
    at_time_t endtime;
    int tick;
    unsigned char data;

    tick = at_tick_from_ms(1000);
    at_timeout_set(endtime, tick);
    while(at_timeout_det(endtime, tick) == AT_TIME_NOTOUT) {
        if(at_getchar(client, &data, 0, 1) != 0) {
            return;
        }
    }
}

/*
********************************************************************************
*Function    : at_client_recv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void at_client_recv(at_client_t client)
{
    unsigned char data;

    while (at_getchar(client, &data, 0, 1) == 0) {
        ;
    }
}

/*
********************************************************************************
*Function    : at_client_getchar
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
char at_client_getchar(at_client_t client)
{
    unsigned char data;

    while (at_getchar(client, &data, 0, 1) == 0) {
        
    }
    return data;
}

/*
********************************************************************************
*Function    : at_client_destory
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void at_client_destory(at_client_t client)
{
    if (client && client->device) {

        rt_device_set_rx_indicate(client->device, RT_NULL);

        client->detector.urc_table_size = 0;

        rt_device_close(client->device);

        rt_sem_delete(client->rx_notice);
        client->rx_notice = RT_NULL;

        client->device = RT_NULL;
    }
}

/*
********************************************************************************
*Function    : at_client_create
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
at_client_t at_client_create(const char *dev_name, const at_urc_t *urc_table, int table_size)
{
    at_client_t client;
    rt_err_t result;
    int idx;
    char name[8];

    for (idx = 0; idx < AT_CLIENT_NUM_MAX && at_client_table[idx].device; idx++);

    /*can not find*/
    if (idx == AT_CLIENT_NUM_MAX) {
        return RT_NULL;
    }
    client = &at_client_table[idx];

    client->device = rt_device_find(dev_name);
    if (client->device) {

        RT_ASSERT(client->device->type == RT_Device_Class_Char);

        //create notice semaphore
        strncpy(name, "at", sizeof(name));
        name[sizeof(name)-1] = '\0';
        client->rx_notice = rt_sem_create(name, 0, RT_IPC_FLAG_FIFO);
        if (client->rx_notice == RT_NULL) {
            LOG_E("AT client initialize failed! at_client_notice semaphore create failed!");
            client->device = RT_NULL;
            return RT_NULL;
        }

        /* using DMA mode first */
        result = rt_device_open(client->device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_RX);
        /* using interrupt mode when DMA mode not supported */
        if (result == -RT_EIO) {
            result = rt_device_open(client->device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
        }

        RT_ASSERT(result == RT_EOK);

        at_detector_init(&client->detector, urc_table, urc_table ? table_size : 0);

        rt_device_set_rx_indicate(client->device, at_rx_ind);
    } else {
        client = RT_NULL;
    }

    return client;
}

/*
********************************************************************************
*Function    : at_cmd_line_get
*Description :
*Input       :
*Output      :
*Return      : length of data
*Others      :
********************************************************************************
*/
int at_cmd_line_get(at_client_t client, char *suffix, unsigned char *xbuf, int xbuf_size, unsigned int timeout_ms)
{
    at_time_t endtime;
    unsigned long waittime;
    unsigned int cmdlen;
    int timeout_tick;
    int suffix_len;
    unsigned char data;

    if (suffix) {
        suffix_len = strlen(suffix);
    } else {
        suffix_len = 0;
    }

    cmdlen = 0;
    if (xbuf && xbuf_size > 0) {
        timeout_tick = at_tick_from_ms(timeout_ms);
        at_timeout_set(endtime, timeout_tick);
        while(at_wait_time(waittime, endtime) <= timeout_tick) {
            if(at_getchar(client, &data, (long)waittime, 0) == 0) {
                //rt_kprintf("%c",data);
                xbuf[cmdlen++] = data;
                if (suffix_len > 0 && cmdlen >= suffix_len
                        && memcmp(&xbuf[cmdlen-suffix_len], suffix, suffix_len) == 0) {
                    cmdlen -= suffix_len;
                    xbuf[cmdlen] = '\0';
                    *suffix = '\0';
                    break;
                }

                if (cmdlen >= xbuf_size) {
                    break;
                }
            }
        }
    }

    return cmdlen;
}

/*
********************************************************************************
*Function    : at_cmd_line
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int at_cmd_line(at_client_t client,
                const unsigned char *atcmd, int atcmdlen,
                const unsigned char *ack,
                unsigned char *xbuf, int xbuf_size,
                unsigned int timeout_ms, int det_err)
{
    at_time_t endtime;
    unsigned long waittime;
    unsigned int cmdlen,acklen;
    unsigned int  i,temp,step;
    int timeout_tick;
    unsigned char data;
    unsigned char tmpbuf[AT_CMD_ACK_MAX_SIZE+1];
    unsigned char bakbuf[AT_CMD_ACK_MAX_SIZE+1];

    //ACK 的合法性检验
    if(ack == 0) {
        acklen = 0;
    } else {
        acklen = strlen((char *)ack);
        if(acklen > AT_CMD_ACK_MAX_SIZE) {
            return AT_CMD_WRONG_ACKFMT;
        }
    }

    //清除接收缓冲区
    if (acklen > 0) {
        at_clear_rxbuf(client);
    }

    //发送AT指令
    if(atcmdlen == 0) {
        cmdlen = strlen((char *)atcmd);
    }
    else {
        cmdlen = atcmdlen;
    }
    if(at_sendstr(client, atcmd, cmdlen) != 0) {
        return AT_CMD_SEND_FAIL;
    }

    if (acklen == 0) {
        return AT_CMD_ACK_OK;
    }

    //获取MODEM AT指令应答命令并处理
    cmdlen = 0;
    step = 0;
    timeout_tick = at_tick_from_ms(timeout_ms);
    at_timeout_set(endtime, timeout_tick);
    
    
    while(at_wait_time(waittime, endtime) <= timeout_tick) {
        if(at_getchar(client, &data, (long)waittime, 1) == 0) {
            rt_kprintf("%c",data);
            switch(step) {
                //step 0: 查找ACK
                case 0:
                    //缓冲区满
                    if(cmdlen > AT_CMD_ACK_MAX_SIZE) {
                        temp = acklen - 1;
                        if (temp < 4) temp = 4;
                        memmove(tmpbuf, tmpbuf + (cmdlen - temp), temp);
                        cmdlen = temp;
                    }
                    tmpbuf[cmdlen++] = data;

                    if(det_err) {
                        if(cmdlen >= 5 && memcmp(&tmpbuf[cmdlen-5], "ERROR", 5) == 0) {
                            return AT_CMD_ACK_ERROR;
                        }
                    }

                    //捕获AT ACK
                    if(cmdlen >= acklen && memcmp(&tmpbuf[cmdlen-acklen], ack, acklen) == 0) {
                        if(xbuf == (unsigned char *)0 || xbuf_size == 0 || xbuf[0] == '\0') {
                            //找到ACK
                            return AT_CMD_ACK_OK;
                        }

                        //拷贝特殊符号标志并检测是否有错,如果正确继续进行下一步,否则返回错误结束标志
                        if(xbuf_size > AT_CMD_ACK_MAX_SIZE) {
                            i = AT_CMD_ACK_MAX_SIZE;
                        } else {
                            i = xbuf_size;
                        }
                        for(cmdlen = 0; cmdlen < i; cmdlen++) {
                            tmpbuf[cmdlen] = xbuf[cmdlen];
                            if(tmpbuf[cmdlen] == '\0') {
                                break;
                            }
                        }
                        if(cmdlen == i) {
                            //错误的特殊结束符号标志
                            return AT_CMD_WRONG_ENDFLG;
                        }

                        //将MODEM数据接收模式设为侦测模式
                        at_reset_detector(&client->detector);
                        acklen = 0;
                        step = 1;
                    }
                    break;

                //step 1: 接收数据
                case 1:
                    if(acklen >= xbuf_size) {
                        if(acklen > AT_CMD_ACK_MAX_SIZE) {
                            i = acklen-AT_CMD_ACK_MAX_SIZE;
                        } else {
                            i = 0;
                        }
                        //拷贝最新的那部分数据
                        for(temp = 0; i < acklen; i++) {
                            bakbuf[temp++] = xbuf[i];
                        }
                        acklen = temp;

                        bakbuf[acklen++] = data;
                        if(acklen >= cmdlen && memcmp(&bakbuf[acklen-cmdlen],tmpbuf,cmdlen) == 0) {
                            //数据长度太长
                            return AT_CMD_ACK_OVER_MAXLEN;
                        }
                        //继续下一步操作
                        step = 2;
                    } else {
                        xbuf[acklen++] = data;
                        if(acklen >= cmdlen && memcmp(&xbuf[acklen-cmdlen], tmpbuf, cmdlen) == 0) {
                            xbuf[acklen-cmdlen] = '\0';
                            return AT_CMD_DATA_GET_OK;
                        }
                    }
                    break;

                //step 2: 接收的数据超过缓冲区的大小,继续检测结束ACK标识
                case 2:
                    //缓冲区满
                    if(acklen > AT_CMD_ACK_MAX_SIZE) {
                        temp = acklen-cmdlen+1;
                        for(i = 0; i < cmdlen-1; i++) {
                            bakbuf[i] = bakbuf[i+temp];
                        }
                        acklen = cmdlen-1;
                    }
                    bakbuf[acklen++] = data;
                    if(acklen >= cmdlen && memcmp(&bakbuf[acklen-cmdlen],tmpbuf,cmdlen) == 0) {
                        //数据包长度太长
                        return AT_CMD_ACK_OVER_MAXLEN;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    return AT_CMD_ACK_TIMEOUT;
}
/*
********************************************************************************
*Function    : at_cmd_line_ex
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int at_cmd_line_ex(at_client_t client,
                const unsigned char *atcmd, int atcmdlen,
                const unsigned char *ack,
                unsigned char *xbuf, int xbuf_size,
                unsigned int timeout_ms, int det_err)
{
    at_time_t endtime;
    unsigned long waittime;
    unsigned int cmdlen,acklen;
    unsigned int  i,temp,step;
    int timeout_tick;
    unsigned char data;
    unsigned char tmpbuf[AT_CMD_ACK_MAX_SIZE+1];
    unsigned char bakbuf[AT_CMD_ACK_MAX_SIZE+1];

    //ACK 的合法性检验
    if(ack == 0) {
        acklen = 0;
    } else {
        acklen = strlen((char *)ack);
        if(acklen > AT_CMD_ACK_MAX_SIZE) {
            return AT_CMD_WRONG_ACKFMT;
        }
    }

    //清除接收缓冲区
    if (acklen > 0) {
        at_clear_rxbuf(client);
    }

    //发送AT指令
    if(atcmdlen == 0) {
        cmdlen = strlen((char *)atcmd);
    }
    else {
        cmdlen = atcmdlen;
    }
    if(at_sendstr(client, atcmd, cmdlen) != 0) {
        return AT_CMD_SEND_FAIL;
    }

    if (acklen == 0) {
        return AT_CMD_ACK_OK;
    }

    //获取MODEM AT指令应答命令并处理
    cmdlen = 0;
    step = 0;
    timeout_tick = at_tick_from_ms(timeout_ms);
    at_timeout_set(endtime, timeout_tick);
    while(at_wait_time(waittime, endtime) <= timeout_tick) {
        if(at_getchar(client, &data, (long)waittime, 1) == 0) {
   
            switch(step) {
                //step 0: 查找ACK
                case 0:
                    //缓冲区满
                    if(cmdlen > AT_CMD_ACK_MAX_SIZE) {
                        temp = acklen - 1;
                        if (temp < 4) temp = 4;
                        memmove(tmpbuf, tmpbuf + (cmdlen - temp), temp);
                        cmdlen = temp;
                    }
                    tmpbuf[cmdlen++] = data;

                    if(det_err) {
                        if(cmdlen >= 5 && memcmp(&tmpbuf[cmdlen-5], "ERROR", 5) == 0) {
                            return AT_CMD_ACK_ERROR;
                        }
                    }

                    //捕获AT ACK
                    if(cmdlen >= acklen && memcmp(&tmpbuf[cmdlen-acklen], ack, acklen) == 0) {
                        if(xbuf == (unsigned char *)0 || xbuf_size == 0 || xbuf[0] == '\0') {
                            //找到ACK
                            return AT_CMD_ACK_OK;
                        }

                        //拷贝特殊符号标志并检测是否有错,如果正确继续进行下一步,否则返回错误结束标志
                        if(xbuf_size > AT_CMD_ACK_MAX_SIZE) {
                            i = AT_CMD_ACK_MAX_SIZE;
                        } else {
                            i = xbuf_size;
                        }
                        for(cmdlen = 0; cmdlen < i; cmdlen++) {
                            tmpbuf[cmdlen] = xbuf[cmdlen];
                            if(tmpbuf[cmdlen] == '\0') {
                                break;
                            }
                        }
                        if(cmdlen == i) {
                            //错误的特殊结束符号标志
                            return AT_CMD_WRONG_ENDFLG;
                        }

                        //将MODEM数据接收模式设为侦测模式
                        at_reset_detector(&client->detector);
                        acklen = 0;
                        step = 1;
                    }
                    break;

                //step 1: 接收数据
                case 1:
                    if(acklen >= xbuf_size) {
                        if(acklen > AT_CMD_ACK_MAX_SIZE) {
                            i = acklen-AT_CMD_ACK_MAX_SIZE;
                        } else {
                            i = 0;
                        }
                        //拷贝最新的那部分数据
                        for(temp = 0; i < acklen; i++) {
                            bakbuf[temp++] = xbuf[i];
                        }
                        acklen = temp;

                        bakbuf[acklen++] = data;
                        if(acklen >= cmdlen && memcmp(&bakbuf[acklen-cmdlen],tmpbuf,cmdlen) == 0) {
                            //数据长度太长
                            return AT_CMD_ACK_OVER_MAXLEN;
                        }
                        //继续下一步操作
                        step = 2;
                    } else {
                        xbuf[acklen++] = data;
                        if(acklen >= cmdlen && memcmp(&xbuf[acklen-cmdlen], tmpbuf, cmdlen) == 0) {
                            xbuf[acklen-cmdlen] = '\0';
                            return AT_CMD_DATA_GET_OK;
                        }
                    }
                    break;

                //step 2: 接收的数据超过缓冲区的大小,继续检测结束ACK标识
                case 2:
                    //缓冲区满
                    if(acklen > AT_CMD_ACK_MAX_SIZE) {
                        temp = acklen-cmdlen+1;
                        for(i = 0; i < cmdlen-1; i++) {
                            bakbuf[i] = bakbuf[i+temp];
                        }
                        acklen = cmdlen-1;
                    }
                    bakbuf[acklen++] = data;
                    if(acklen >= cmdlen && memcmp(&bakbuf[acklen-cmdlen],tmpbuf,cmdlen) == 0) {
                        //数据包长度太长
                        return AT_CMD_ACK_OVER_MAXLEN;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    return AT_CMD_ACK_TIMEOUT;
}

/*
********************************************************************************
*Function    : at_cmd_send
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int at_cmd_send(at_client_t client,const unsigned char *atcmd, int atcmdlen)
{
    if(atcmdlen == 0) {
        atcmdlen = strlen((char *)atcmd);
    }
    
    if(at_sendstr(client, atcmd, atcmdlen) != 0) {
        return AT_CMD_SEND_FAIL;
    }
    
    return AT_CMD_ACK_OK;
}