/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __QUEUE16_H_
#define __QUEUE16_H_


/*
********************************************************************************
********************************************************************************
*/ 

typedef struct
{
    u16_t *base;
    u16_t front;
    u16_t rear;
    u16_t maxsize;
}sqqueue16;

/*
********************************************************************************
********************************************************************************
*/ 
void InitQueue16(sqqueue16* q, u16_t *buff,u16_t maxsize);
u8_t EnQueue16(sqqueue16* q,u16_t e);
u8_t DeQueue16(sqqueue16* q,u16_t* e);
u8_t IsQueue16(sqqueue16* q);
#endif
/*__QUEUE16_H_*/

