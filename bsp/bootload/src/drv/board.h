 /*
 * board.h
 *
 *  Created on: 2022Äê10ÔÂ19ÈÕ
 *      Author: lwp edit
 */
#ifndef _BOARD_H
#define _BOARD_H

#include <stdint.h>
#include "stm32u5xx_hal.h"
/*
********************************************************************************
********************************************************************************
*/
void Error_Handler(void);

void SystemClock_Config(void);

void SystemPower_Config(void);

void Icache_Init(void);

void system_reset(void);

void delay_1ms(uint32_t count);
 
void delay_decrement(void);

#endif 
/* _BOARD_H */
