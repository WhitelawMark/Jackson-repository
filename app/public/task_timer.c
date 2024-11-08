/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */
#include <stdio.h>
#include <string.h>
#include <board.h>
#include <rtthread.h>
#include <drivers/pin.h>
#include "task_timer.h"

/*
********************************************************************************
********************************************************************************
*/  

#ifdef IS_TIMEOUT
#undef IS_TIMEOUT
#endif
#define IS_TIMEOUT(current_tick, will_tick)  ((current_tick) - (will_tick) < RT_TICK_MAX / 2)
 
#define rt_list_for_each_time(pos, head, type, member) \
    for (pos = rt_list_entry((head)->next, type, member); \
         &pos->member != (head); \
         pos = rt_list_entry(pos->member.next, type, member))

 
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : task_timer_check
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/  
static rt_tick_t task_timer_check(struct task_timer_list *list, rt_tick_t current_tick)
{
    struct task_timer *t;
    rt_tick_t wait_tick = (rt_tick_t)(RT_WAITING_FOREVER);

    while (!rt_list_isempty(&list->timer_list)) {
        t = rt_container_of(list->timer_list.next, struct task_timer, entry);
        if (IS_TIMEOUT(current_tick, t->will_tick)) {
            rt_list_remove(&t->entry);
            rt_list_insert_before(&list->timerout_list, &t->entry);
        } else {
            wait_tick = (t->will_tick - current_tick);
            break;
        }
    }

    if (!rt_list_isempty(&list->timerout_list)) {
        wait_tick = RT_WAITING_NO;
    }

    return wait_tick;
}

/*
********************************************************************************
*Function    : task_timer_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/   
rt_tick_t task_timer_process(struct task_timer_list *list)
{
    struct task_timer *t;
    rt_tick_t current_tick;

    current_tick = rt_tick_get();

    //check timer
    task_timer_check(list, current_tick);

    //process
    while (!rt_list_isempty(&list->timerout_list)) {
        t = rt_container_of(list->timerout_list.next, struct task_timer, entry);
        rt_list_remove(&t->entry);
        t->timeout_func(t->arg);
    }

    current_tick = rt_tick_get();

    return task_timer_check(list, current_tick);
}


/*
********************************************************************************
*Function    : task_timer_stop
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/  
void task_timer_stop(struct task_timer *timer)
{
    rt_list_remove(&timer->entry);
}

/*
********************************************************************************
*Function    : task_timer_start
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/   
int task_timer_start(struct task_timer *timer, u32_t ms)
{
    struct task_timer *t;
    rt_tick_t current_tick;
    
    if (timer->list == RT_NULL) {
        return -1;
    }

    current_tick = rt_tick_get();

    //check timer
    task_timer_check(timer->list, current_tick);

    //stop timer
    task_timer_stop(timer);
 
    timer->will_tick = current_tick + rt_tick_from_millisecond(ms);

    if (IS_TIMEOUT(current_tick, timer->will_tick)) {
        rt_list_insert_before(&timer->list->timerout_list, &timer->entry);
    } else {
        rt_list_for_each_time(t, &timer->list->timer_list, struct task_timer, entry) {
            if ((t->will_tick - current_tick) > (timer->will_tick - current_tick)) {
                break;
            }
        }
        rt_list_insert_before(&t->entry, &timer->entry);
    }
    
    return 0;
}

/*
********************************************************************************
*Function    : task_timer_set
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/   
void task_timer_set(struct task_timer *timer, struct task_timer_list *list, void (*func)(void *arg), void *arg)
{
    timer->list = list;
    timer->timeout_func = func;
    timer->arg = arg;  
}

/*
********************************************************************************
*Function    : task_timer_assign
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/   
void task_timer_assign(struct task_timer *timer, struct task_timer_list *list, void (*func)(void *arg), void *arg)
{
    rt_list_init(&timer->entry);

    task_timer_set(timer, list, func, arg);
} 

/*
********************************************************************************
*Function    : task_timer_list_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/  
void task_timer_list_init(struct task_timer_list *list)
{
    rt_list_init(&list->timer_list);
    rt_list_init(&list->timerout_list);
}

