/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */
#include "type.h" 
#include "Queue32.h" 

/*
********************************************************************************
********************************************************************************
*/ 
#ifdef MAXQSIZE
#undef MAXQSIZE
#endif
#define MAXQSIZE         5

/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  InitQueue
**  功能描述:  初始化队列
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void InitQueue32(sqqueue32* q, u32_t *buff, u32_t maxsize)
{    
    q->base = buff;
    q->front = q->rear = 0;
    q->maxsize =maxsize;
} 
/*
********************************************************************************
**  函数名称:  EnQueue
**  功能描述:  入队
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
u8_t EnQueue32(sqqueue32* q,u32_t e)
{
    if (((q->rear+1)%q->maxsize) == q->front){
        return 0;                                                 /* 队列满 */
    }

    if (q->rear == q->maxsize) {
        q->rear = q->maxsize;
    }
    q->base[q->rear]=e;
    q->rear=(q->rear+1)%q->maxsize;
    return 1;
} 
/*
********************************************************************************
**  函数名称:  DeQueue
**  功能描述:  出队
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
u8_t DeQueue32(sqqueue32* q,u32_t* e)
{
    if(q->front == q->rear){
        return 0;
    }

    *e = q->base[q->front];
    q->front = (q->front+1)% q->maxsize;

    return 1;
}
/*
********************************************************************************
**  函数名称:  DeQueue
**  功能描述:  出队
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
u8_t IsQueue32(sqqueue32* q)
{
    if (((q->rear+1)% q->maxsize) == q->front){
        return 0;                                                                    /* 队列满 */
    }
    return 1;
}