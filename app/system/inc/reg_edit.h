/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */                                                                        
#ifndef __REG_EDIT_H                                                      
#define __REG_EDIT_H                                                      
                                                                          
/*
********************************************************************************
********************************************************************************
*/                                                                      
/*                                                                        
 *REG 定时器结构体                                                        
 */                                                                       
typedef struct _REG_EDIT                                                  
{                                                                         
    struct _REG_EDIT *Next;                                               
    unsigned long Timer;                                                  
    unsigned long TimerLen;                                               
    void (*pTimerCb)(void);                                               
}REG_EDIT;                                                                
/*
********************************************************************************
********************************************************************************
*/                                                                         
#define REG_EDITSet(timer,timelen)        ((timer)->Timer = timelen)      
#define REG_EDITDet(timer)                ((timer)->Timer == 0)           

#define REG_TIME_OUT        1                                         
#define REG_TIME_NOTOUT     0                                         

#define REG_DONE_OK         1                                         
#define REG_DONE_FAIL       0                                         
/*
********************************************************************************
********************************************************************************
*/
void POLLEventCancel(void (*pfuncCb)(void));
void POLLEventReg(unsigned int timelen,void (*pfuncCb)(void));
void REG_CancelTimer(REG_EDIT *timer);
unsigned int REGRegTimer(REG_EDIT *timer,unsigned int timelen,void (*pfuncCb)(void));

int reg_thread_init(void);
#endif 
/* __REG_EDIT_H */
