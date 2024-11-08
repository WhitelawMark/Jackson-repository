/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */                                                                    
       
#include  "pit.h"

/*
********************************************************************************
********************************************************************************
*/ 
#define  PIT_ENTER_CRITICAL  //OS_ENTER_CRITICAL         
#define  PIT_EXIT_CRITICAL  // OS_EXIT_CRITICAL 

/*
********************************************************************************
********************************************************************************
*/                 
static   PIT_TIMER *pPitTimerList;                                     /* PIT 定时器链的头指针 */
unsigned int PitTimeTick=0;                                              /* PIT 时钟节拍  */
/*
********************************************************************************
********************************************************************************
*/ 
/*                                                                        
********************************************************************************
*Function    : PITTimeTick                                      
*Description : PIT 时钟节拍处理                                                           
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
void PITTimeTick(void)
{
    PIT_TIMER *pList;

    PitTimeTick++;

    for(pList = pPitTimerList; pList != (PIT_TIMER *)0; pList = pList->Next) {
        if(pList->Timer > 0) {
            pList->Timer--;
            if(( pList->Timer == 0 )&&( pList->pTimerCb != 0 )){
                pList->pTimerCb();
            }
        }
    }
}
/*                                                                        
********************************************************************************
*Function    : PITRegTimer                                      
*Description : 向PIT时间管理器注册一个节拍定时器并设置定时时间                                                           
*Input       : timer         -- 指向定时器的指针
**             timelen       -- 要设置定时的时长(单位：ms)                                                           
*Output      : none                                                           
*Return      : PIT_DONE_OK   -- 定时器注册或设定成功
**             PIT_DONE_FAIL --定时器注册失败                                                            
*Others      : none                                                           
********************************************************************************
*/
 
unsigned int PITRegTimer(PIT_TIMER *timer,unsigned int timelen,void (*pTimerCb)(void))
{
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR cpu_sr = 0;
#endif   

    PIT_TIMER *pList;
    
    if(timer == (PIT_TIMER *)0) {
        return PIT_DONE_FAIL; 
    }

    //PIT_ENTER_CRITICAL();
    for(pList = pPitTimerList; pList != (PIT_TIMER *)0; pList = pList->Next) {
        if(pList == timer) {
            //
            //该时间块已经注册过
            //
            timer->Timer = timelen;
            timer->pTimerCb = pTimerCb;
            //PIT_EXIT_CRITICAL();
            return PIT_DONE_OK; 
        }
    }
    timer->pTimerCb = pTimerCb;
    timer->Next = pPitTimerList;
    pPitTimerList = timer;
    
    pPitTimerList->Timer = timelen;

    //PIT_EXIT_CRITICAL();

    return PIT_DONE_OK;
}
/*                                                                        
********************************************************************************
*Function    : PIT_CancelTimer                                      
*Description : 向PIT时间管理器注销一个定时器                                                           
*Input       : timer -- 指向定时器的指针                                                          
*Output      : none                                                           
*Return      : none                                                           
*Others      : none                                                           
********************************************************************************
*/
void PIT_CancelTimer(PIT_TIMER *timer)
{
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR cpu_sr = 0;
#endif

    PIT_TIMER *pCurTimer;

    if(timer == (PIT_TIMER *)0) {
        return;
    }

   // PIT_ENTER_CRITICAL();
    if(pPitTimerList == timer) {
        pPitTimerList = pPitTimerList->Next; 
        timer->Next = (PIT_TIMER *)0;
        timer->Timer = 0;
       // PIT_EXIT_CRITICAL();
        return;      
    }

    for(pCurTimer = pPitTimerList; pCurTimer != (PIT_TIMER *)0; pCurTimer = pCurTimer->Next) { 
        if(pCurTimer->Next == timer) {
            pCurTimer->Next = timer->Next;
            timer->Next = (PIT_TIMER *)0; 
            timer->Timer = 0;  
            //PIT_EXIT_CRITICAL();  
            return;   
        }    
    }
    
    timer->Next = (PIT_TIMER *)0;
    timer->Timer = 0; 
   // PIT_EXIT_CRITICAL();
    return;        
} 