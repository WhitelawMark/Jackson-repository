/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-7      SummerGift   first version
 */

#include "drv_common.h"
#include "board.h"

#ifdef RT_USING_SERIAL
#ifdef RT_USING_SERIAL_V2
#include "drv_usart_v2.h"
#else
#include "drv_usart.h"
#endif /* RT_USING_SERIAL */
#endif /* RT_USING_SERIAL_V2 */

#define DBG_TAG    "drv_common"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>



extern __IO uint32_t uwTick;
static uint32_t _systick_ms = 1;
#if 0
/*
********************************************************************************
*Function    :   MX_ICACHE_Init 
*Description :   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void MX_ICACHE_Init(void)
{
    if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK){
        Error_Handler();
    }
    if (HAL_ICACHE_Enable() != HAL_OK){
        Error_Handler();
    }
}
#endif
/*
********************************************************************************
*Function    :   reboot 
*Description :   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
#ifndef PKG_USING_SYSWATCH
#ifdef RT_USING_FINSH
#include <finsh.h>
static void reboot(uint8_t argc, char **argv)
{
    rt_hw_cpu_reset();
}
MSH_CMD_EXPORT(reboot, Reboot System);
#endif /* RT_USING_FINSH */
#endif /* PKG_USING_SYSWATCH */
/*
********************************************************************************
*Function    :   rt_hw_systick_init 
*Description :   SysTick configuration
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void rt_hw_systick_init(void)
{
    HAL_SYSTICK_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    NVIC_SetPriority(SysTick_IRQn, 0xFF);

    _systick_ms = 1000u / RT_TICK_PER_SECOND;
    if(_systick_ms == 0){
        _systick_ms = 1;
    }
}
/*
********************************************************************************
*Function    :   SysTick_Handler 
*Description :  This is the timer interrupt service routine.
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){
        HAL_IncTick();
    }

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}
/*
********************************************************************************
*Function    :  HAL_GetTick 
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
uint32_t HAL_GetTick(void)
{
    if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){
        HAL_IncTick();
    }

    return uwTick;
}
/*
********************************************************************************
*Function    :  HAL_IncTick 
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void HAL_IncTick(void)
{
    uwTick += _systick_ms;
}
/*
********************************************************************************
*Function    :  HAL_SuspendTick 
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void HAL_SuspendTick(void)
{
}
/*
********************************************************************************
*Function    :  HAL_ResumeTick 
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void HAL_ResumeTick(void)
{
}
/*
********************************************************************************
*Function    :  HAL_Delay 
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void HAL_Delay(__IO uint32_t Delay)
{
    if (rt_thread_self()){
        rt_thread_mdelay(Delay);
    }else{
        for (rt_uint32_t count = 0; count < Delay; count++){
            rt_hw_us_delay(1000);
        }
    }
}
/*
********************************************************************************
*Function    :  HAL_InitTick 
*Description :   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    rt_hw_systick_init();

    /* Return function status */
    return HAL_OK;
}
/*
********************************************************************************
*Function    :  _Error_Handler 
*Description : This function is executed in case of error occurrence.  
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
void _Error_Handler(char *s, int num)
{

    /* User can add his own implementation to report the HAL error return state */
    LOG_E("Error_Handler at file:%s num:%d", s, num);
    while (1)
    {
    }

}
/*
********************************************************************************
*Function    :  rt_hw_us_delay 
*Description : This function will delay for some us.
*Input       : @param us the delay time of us
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint64_t ticks;
    rt_uint32_t told, tnow, tcnt = 0;
    rt_uint32_t reload = SysTick->LOAD;

    ticks = us * (reload / (1000000 / RT_TICK_PER_SECOND));
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}
/*
********************************************************************************
*Function    : rt_hw_vect_init 
*Description : This function will initial STM32 board.
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
rt_weak void rt_hw_vect_init(void)
{  
#define NVIC_VTOR_MASK              0x3FFFFF80
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x10000000 */
    SCB->VTOR  = (0x10000000 & NVIC_VTOR_MASK);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x0800A200  0x08010200*/
    SCB->VTOR  = (0x08020200 & NVIC_VTOR_MASK);
#endif
}
/*
********************************************************************************
*Function    : rt_hw_board_init 
*Description : This function will initial STM32 board.
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
rt_weak void rt_hw_board_init(void)
{
  rt_hw_vect_init();
#ifdef BSP_SCB_ENABLE_I_CACHE
    SCB_EnableICache();
#endif

#ifdef BSP_SCB_ENABLE_D_CACHE
    SCB_EnableDCache();
#endif
    /* HAL_Init() function is called at the beginning of the program */
    HAL_Init();
    
//    rt_hw_vect_init();
    
   // MX_ICACHE_Init();
    
    /* System clock initialization */
    SystemClock_Config();
       
    /* Heap initialization */
#if defined(RT_USING_HEAP)
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif

    /* Pin driver initialization is open by default */
#ifdef RT_USING_PIN
    rt_hw_pin_init();
#endif

    /* USART driver initialization is open by default */
#ifdef RT_USING_SERIAL
    rt_hw_usart_init();/*串口初始化默认打开*/
#endif

    /* Set the shell console output device */
#if defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}

