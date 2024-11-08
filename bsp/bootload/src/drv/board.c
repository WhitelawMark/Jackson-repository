 /*
 * board.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */


#include "board.h"

#include "stm32u5xx_hal.h"
 


/*
********************************************************************************
********************************************************************************
*/ 
#define SCB_CFSR        (*(volatile const unsigned *)0xE000ED28) /* Configurable Fault Status Register */
#define SCB_HFSR        (*(volatile const unsigned *)0xE000ED2C) /* HardFault Status Register */
#define SCB_MMAR        (*(volatile const unsigned *)0xE000ED34) /* MemManage Fault Address register */
#define SCB_BFAR        (*(volatile const unsigned *)0xE000ED38) /* Bus Fault Address Register */
#define SCB_AIRCR       (*(volatile unsigned long *)0xE000ED0C)  /* Reset control Address Register */
#define SCB_RESET_VALUE 0x05FA0004                               /* Reset value, write to SCB_AIRCR can reset cpu */

#define SCB_CFSR_MFSR   (*(volatile const unsigned char*)0xE000ED28)  /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR   (*(volatile const unsigned char*)0xE000ED29)  /* Bus Fault Status Register */
#define SCB_CFSR_UFSR   (*(volatile const unsigned short*)0xE000ED2A) /* Usage Fault Status Register */

/*
********************************************************************************
********************************************************************************
*/ 

volatile static uint32_t delay;
/*
********************************************************************************
**  函数名称:  system_reset
**  功能描述:  无  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void system_reset(void)
{
    SCB_AIRCR = SCB_RESET_VALUE;
}
/*
********************************************************************************
**  函数名称:  SysTick_Handler
**  功能描述:  无  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void SysTick_Handler(void)
{
    HAL_IncTick();
}
/*
********************************************************************************
**  函数名称:  Error_Handler
**  功能描述:  无  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void Error_Handler(void)
{

    __disable_irq();
    while (1)
    {
    }
}
/*
********************************************************************************
**  函数名称:  HAL_UART_MspInit
**  功能描述:  无  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    
    if(huart->Instance==LPUART1){
        /** Initializes the peripherals clock
        */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
        PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK3;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_LPUART1_CLK_ENABLE();

        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**LPUART1 GPIO Configuration
        PG7     ------> LPUART1_TX
        PG8     ------> LPUART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    }else if(huart->Instance==USART1){
        /** Initializes the peripherals clock
        */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
            Error_Handler();
        }
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }else if(huart->Instance==USART2){
        /** Initializes the peripherals clock
        */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
        PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
            Error_Handler();
        }
 
        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    }
}

/*
********************************************************************************
**  函数名称:  HAL_SPI_MspInit
**  功能描述:  无  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    if(hspi->Instance==SPI1){
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
        PeriphClkInit.Spi1ClockSelection = RCC_SPI1CLKSOURCE_SYSCLK;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
           // Error_Handler();
        }
    
        /* Peripheral clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
      //  __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**SPI1 GPIO Configuration
        //PA4     ------> SPI1_NSS
        PB0     ------> SPI1_NSS
        PA5     ------> SPI1_SCK
        PA6     ------> SPI1_MISO
        PA7     ------> SPI1_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  //PD14 开发板  PB0
    }else if(hspi->Instance==SPI2){

    /* SPI3 interrupt Init */
    }else if(hspi->Instance==SPI3){



    }
}
/*
********************************************************************************
**  函数名称:  SystemClock_Config
**  功能描述:  无  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Enable voltage range 1 for frequency above 100 Mhz */
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Switch to SMPS regulator instead of LDO */
    HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY);

    __HAL_RCC_PWR_CLK_DISABLE();

    /* MSI Oscillator enabled at reset (4Mhz), activate PLL with MSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 80;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLFRACN= 0;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
        /* Initialization Error */
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK){

    }
}

/*
********************************************************************************
**  函数名称:  Icache_Init
**  功能描述:  Enable instruction cache in 1-way (direct mapped cache)
**  输入参数:  count
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void Icache_Init(void)
{
    if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK){
        Error_Handler();
    }
    if (HAL_ICACHE_Disable() != HAL_OK){
        Error_Handler();
    }
}
/*
********************************************************************************
**  函数名称:  delay_1ms
**  功能描述:  无  
**  输入参数:  count
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void delay_1ms(uint32_t count)
{
    HAL_Delay(count);
}
