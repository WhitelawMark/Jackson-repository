/*
 * oam_thread.h
 *
 *  Created on: 2023Äê7ÔÂ17ÈÕ
 *      Author: lwp
 */

#ifndef OAM_THREAD_H_
#define OAM_THREAD_H_

#include "rtthread.h"
#include "task_timer.h"

/*
********************************************************************************
********************************************************************************
*/
enum oam_protype {
    OAM_PRO_UDP   = 0,
    OAM_PRO_MQT   = 1,
};
/*
********************************************************************************
********************************************************************************
*/
void oam_thread_timer_set(struct task_timer *timer, void (*func)(void *arg), void *arg);

int oam_thread_post(void (*process)(void *data, int data_len), void *data, int data_len);

#endif /* OAM_THREAD_H_ */
