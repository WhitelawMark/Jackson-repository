/*
 * oam_thread.c
 *
 *  Created on: 2023Äê7ÔÂ17ÈÕ
 *      Author: lwp
 */
#include <string.h>
#include "oam_thread.h"
#include "task_timer.h"
#include "oam_port.h"

#define DBG_TAG "oam"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
struct oam_thread_msg {
    void (*process)(void *data, int data_len);
    void *data;
    int data_len;
};

/*
********************************************************************************
********************************************************************************
*/
#define OAM_THREAD_PRIORITY      19

#define OAM_THREAD_TIMESLICE      5

static struct rt_thread  oam_thread;

static struct rt_messagequeue oam_thread_mq;

static char oam_thread_msg_pool[80*sizeof(void*)];

static rt_uint8_t oam_thread_stack[4096*2];

static struct task_timer_list oam_thread_timer_list = {
    .timer_list = RT_LIST_OBJECT_INIT(oam_thread_timer_list.timer_list),
    .timerout_list = RT_LIST_OBJECT_INIT(oam_thread_timer_list.timerout_list),
};
/*
********************************************************************************
*Function    : oam_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void oam_init(void)
{
    oam_start();
    
    rt_thread_mdelay(4000);
}
/*
********************************************************************************
*Function    : oam_thread_timer_set
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void oam_thread_timer_set(struct task_timer *timer, void (*func)(void *arg), void *arg)
{
    task_timer_set(timer, &oam_thread_timer_list, func, arg);
}

/*
********************************************************************************
*Function    : oam_thread_entry
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void oam_thread_entry(void *arg)
{
    struct oam_thread_msg *thread_msg;
    rt_tick_t wait_tick;
    (void)arg;
    
    
    oam_init();

    while (1) {
        wait_tick = task_timer_process(&oam_thread_timer_list);
  
        if (rt_mq_recv(&oam_thread_mq, &thread_msg, sizeof(struct oam_thread_msg *), wait_tick) > 0){
            if (thread_msg->process) {
                thread_msg->process(thread_msg->data, thread_msg->data_len);
            }
            rt_free(thread_msg);
        }
    }
}
/*
********************************************************************************
*Function    : oam_thread_post
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int oam_thread_post(void (*process)(void *data, int data_len), void *data, int data_len)
{
    struct oam_thread_msg *thread_msg;

    if (rt_thread_self() == &oam_thread) {
        if (process) {
            process(data, data_len);
        }
        return 0;
    }

    thread_msg = rt_malloc(sizeof(struct oam_thread_msg) + data_len);
    if (thread_msg == RT_NULL) {
        return -1;
    }
    thread_msg->data = (void *)(thread_msg + 1);

    thread_msg->process = process;
    memcpy(thread_msg->data, data, data_len);
    thread_msg->data_len = data_len;

    if (rt_mq_send(&oam_thread_mq, &thread_msg, sizeof(struct oam_thread_msg *)) != RT_EOK) {
        rt_free(thread_msg);
        return -1;
    }

    return 0;
}

/*
********************************************************************************
*Function    : oam_thread_Init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int oam_thread_Init(void)
{
    rt_err_t result;


    result = rt_mq_init(&oam_thread_mq, "oam_mq", &oam_thread_msg_pool[0], sizeof(void *), sizeof(oam_thread_msg_pool), RT_IPC_FLAG_FIFO);
    if (result != RT_EOK){
        return result;
    }

    
    result = rt_thread_init(&oam_thread,
                             "oam",
                             oam_thread_entry,
                             (void*)0,
                             &oam_thread_stack[0],
                             sizeof(oam_thread_stack),
                             OAM_THREAD_PRIORITY,
                             OAM_THREAD_TIMESLICE);
    if (result == RT_EOK){
        rt_thread_startup(&oam_thread);
    }

    return result;
}
INIT_APP_EXPORT(oam_thread_Init);
