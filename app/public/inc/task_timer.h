/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef TASK_TIMER_H_
#define TASK_TIMER_H_

#include "rtthread.h"
#include "rtdef.h"
#include "type.h"

/*
********************************************************************************
********************************************************************************
*/ 
#define DECLARE_TASK_TIMER(timer)   struct task_timer timer = {.entry = RT_LIST_OBJECT_INIT(timer.entry)}

struct task_timer_list {
    rt_list_t timer_list;
    rt_list_t timerout_list;
};

struct task_timer {
    rt_list_t entry;
    rt_tick_t will_tick;
    struct task_timer_list *list;
    void (*timeout_func)(void *arg);
    void *arg;
};

/*
********************************************************************************
********************************************************************************
*/  

rt_tick_t task_timer_process(struct task_timer_list *list);

void task_timer_stop(struct task_timer *timer);

int task_timer_start(struct task_timer *timer, u32_t ms);

void task_timer_set(struct task_timer *timer, struct task_timer_list *list, void (*func)(void *arg), void *arg);

void task_timer_assign(struct task_timer *timer, struct task_timer_list *list, void (*func)(void *arg), void *arg);

void task_timer_list_init(struct task_timer_list *list);

#endif   /*TASK_TIMER_H_*/




