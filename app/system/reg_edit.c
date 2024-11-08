/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */                                                                         
#include "app_lib.h"                                                          
                                                                              
/*
********************************************************************************
********************************************************************************
*/
#define  REG_EDIT_MAX_NUM     5                                               
                                                                              
/*
********************************************************************************
********************************************************************************
*/

static REG_EDIT *pRegListHead;                    /*REG 定时器链的头指针    */ 


#define REG_THREAD_PRIORITY       29
#define REG_THREAD_STACK_SIZE     1024
#define REG_THREAD_TIMESLICE      5

static u8_t reg_thread_stack[REG_THREAD_STACK_SIZE];
static struct  rt_thread   reg_thread;

REG_EDIT REG_EDITPoll[REG_EDIT_MAX_NUM];
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
**  函数名称:  REGTimeTick
**  功能描述:  REG注册任务
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static void REGTimeTick(void)
{
    REG_EDIT *pList;

    for(pList = pRegListHead; pList != (REG_EDIT *)0; pList = pList->Next) {
        if(pList->Timer > 0) {
            pList->Timer--;
            if(( pList->Timer == 0 )&&( pList->pTimerCb != 0 )){
                pList->Timer = pList->TimerLen;
                pList->pTimerCb();
            }
        }
    }
} 
/*
********************************************************************************
**  函数名称:  REGRegTimer
**  功能描述:  REG 时钟节拍处理
**  输入参数:  向REG时间管理器注册一个节拍定时器并设置定时时间
**             timer         -- 指向定时器的指针
**             timelen       -- 要设置定时的时长(单位：ms)
**  输出参数:  REG_DONE_OK   -- 定时器注册或设定成功
**             REG_DONE_FAIL --定时器注册失败
**  返回参数:  无
********************************************************************************
*/
unsigned int REGRegTimer(REG_EDIT *timer,unsigned int timelen,void (*pfuncCb)(void))
{
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR cpu_sr = 0;
#endif   

    REG_EDIT *pList;
    
    if(timer == (REG_EDIT *)0) {
        return REG_DONE_FAIL; 
    }
    for(pList = pRegListHead; pList != (REG_EDIT *)0; pList = pList->Next) {
        if(pList == timer) {
            //
            //该时间块已经注册过
            //
            timer->Timer = timelen;
            return REG_DONE_OK; 
        }
    }
    timer->Next = pRegListHead;
    pRegListHead = timer;
    pRegListHead->Timer = timelen;
    pRegListHead->TimerLen = timelen;
    pRegListHead->pTimerCb = pfuncCb;

    return REG_DONE_OK;
} 
/*
********************************************************************************
**  函数名称:  REG_CancelTimer
**  功能描述:  向PIT时间管理器注销一个定时器
**  输入参数:  timer -- 指向定时器的指针
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void REG_CancelTimer(REG_EDIT *timer)
{
#if OS_CRITICAL_METHOD == 3u
    OS_CPU_SR cpu_sr = 0;
#endif

    REG_EDIT *pCurTimer;

    if(timer == (REG_EDIT *)0) {
        return;
    }

    if(pRegListHead == timer) {
        pRegListHead = pRegListHead->Next; 
        timer->Next = (REG_EDIT *)0;
        timer->Timer = 0;
        return;      
    }

    for(pCurTimer = pRegListHead; pCurTimer != (REG_EDIT *)0; pCurTimer = pCurTimer->Next) { 
        if(pCurTimer->Next == timer) {
            pCurTimer->Next = timer->Next;
            timer->Next = (REG_EDIT *)0; 
            timer->Timer = 0;  
            return;   
        }    
    }
    
    timer->Next = (REG_EDIT *)0;
    timer->Timer = 0; 
    return;        
} 

/*
********************************************************************************
**  函数名称:  POLLEventReg
**  功能描述:  填充定时处理任务
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void POLLEventReg(unsigned int timelen,void (*pfuncCb)(void))
{
    for(u8_t i=0;i< REG_EDIT_MAX_NUM;i++){
       if( REG_EDITPoll[i].pTimerCb==pfuncCb){
           return;
       }
    }
    for(u8_t i=0;i< REG_EDIT_MAX_NUM;i++){
        if( REG_EDITPoll[i].pTimerCb==NULL){
            REG_EDITPoll[i].pTimerCb = pfuncCb;
            REG_EDITPoll[i].Timer = timelen;
            REG_EDITPoll[i].TimerLen = timelen;
            break;
        }
    }  
}
/*
********************************************************************************
**  函数名称:  POLLEventCancel
**  功能描述:  填充定时处理任务
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void POLLEventCancel(void (*pfuncCb)(void))
{
    for(u8_t i=0;i< REG_EDIT_MAX_NUM;i++){
        if( REG_EDITPoll[i].pTimerCb==pfuncCb){
            REG_EDITPoll[i].pTimerCb=NULL;
            break;
        }
    }  

}

/*
********************************************************************************
**  函数名称:  reg_thread_entry   
**  功能描述:  注册任务 
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void reg_thread_entry(void *parameter)
{
    while(1){
        REGTimeTick();
        rt_thread_mdelay(10);
   }
}
/*
********************************************************************************
**  函数名称:  reg_thread_init   
**  功能描述:  注册任务初始化
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
int reg_thread_init(void)
{
    rt_err_t result = RT_EOK;
    for(u8_t i=0;i< REG_EDIT_MAX_NUM;i++)
    {
        REGRegTimer(&REG_EDITPoll[i], 0, NULL);
    }
    result = rt_thread_init(&reg_thread, "RegThread",
                          reg_thread_entry, RT_NULL,
                          &reg_thread_stack[0], REG_THREAD_STACK_SIZE,
                          REG_THREAD_PRIORITY, REG_THREAD_TIMESLICE);

    if(result == RT_EOK){
        rt_thread_startup(&reg_thread);
    }else{
        return -1;
    }
    return 0;
}
INIT_COMPONENT_EXPORT(reg_thread_init);
