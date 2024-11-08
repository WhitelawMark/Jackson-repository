/*******************************************************************************
**                                                                            **
**  文件名称:  pit.h                                                           **
**  版权所有:  CopyRight @ AcTec(XiaMen) Medical Technology Co.,Ltd 2015       **
**  文件描述:  PIT驱动头文件                                                    **
**  ========================================================================  **
**  创建信息:  | 2021-9-20 | lwp | 创建本模块                                   **
**  ========================================================================  **
**  修改信息:  单击此处添加....                                                 **
*******************************************************************************/
#ifndef __PIT_H_
#define __PIT_H_
/******************************************************************************/
/*                           结构体声明                                       */
/******************************************************************************/


/*
 *PIT 定时器结构体             
 */
typedef struct _PIT_TIMER 
{
    struct _PIT_TIMER *Next;
    unsigned long Timer;
    void (*pTimerCb)(void);
}PIT_TIMER;
/*
********************************************************************************
********************************************************************************
*/
#define PIT_TimeOutSet(end_time,timelen)       ((end_time) = (PitTimeTick+(timelen)))
#define PIT_TimeOutDet(end_time,timelen)       ((unsigned int)(end_time-PitTimeTick) > (timelen))

#define PIT_TimerSet(timer,timelen,ptimecb)    {(timer)->Timer = timelen;(timer)->pTimerCb = ptimecb; }
#define PIT_TimerDet(timer)                    ((timer)->Timer == 0)
#define PIT_TimerDetCb(timer)                  ((timer)->pTimerCb == NULL)
/*
********************************************************************************
********************************************************************************
*/
#define PIT_TIME_OUT        1
#define PIT_TIME_NOTOUT     0

#define PIT_DONE_OK            1
#define PIT_DONE_FAIL          0
/*
********************************************************************************
********************************************************************************
*/
extern unsigned int PitTimeTick;
/*
********************************************************************************
********************************************************************************
*/
void PITTimeTick(void);
void PIT_CancelTimer(PIT_TIMER *timer);
unsigned int PITRegTimer(PIT_TIMER *timer,unsigned int timelen,void (*pTimerCb)(void));

#endif
/*__PIT_H_*/

