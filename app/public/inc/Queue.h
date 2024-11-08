/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */


/* Data types redefintion headers. */
#ifndef __QUEUE_H_
#define __QUEUE_H_

/*
********************************************************************************
********************************************************************************
*/ 
#define TXBUF_SIZE              1024    //CC1000发送缓冲区大小
#define RXBUF_SIZE              1024    //CC1000接收缓冲区大小 
#define MAXQSIZE RXBUF_SIZE


typedef struct
{
    u8_t *base;
    u32_t front;
    u32_t rear;
}sqqueue;

/*
********************************************************************************
********************************************************************************
*/ 
void InitQueue(sqqueue* q, u8_t *buff);
u8_t EnQueue(sqqueue* q,u8_t e);
u8_t DeQueue(sqqueue* q,u8_t* e);
#endif
/*__QUEUE_H_*/

