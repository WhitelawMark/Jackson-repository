/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __QUEUE32_H_
#define __QUEUE32_H_


/*
********************************************************************************
********************************************************************************
*/ 
typedef struct
{
    u32_t *base;
    u32_t front;
    u32_t rear;
    u32_t maxsize;
}sqqueue32;

/*
********************************************************************************
********************************************************************************
*/ 
void InitQueue32(sqqueue32* q, u32_t *buff,u32_t maxsize);
u8_t EnQueue32(sqqueue32* q,u32_t e);
u8_t DeQueue32(sqqueue32* q,u32_t* e);
u8_t IsQueue32(sqqueue32* q);
#endif
/*__QUEUE32_H_*/

