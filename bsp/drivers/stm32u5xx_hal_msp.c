/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32u5xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "board.h"

#include <drv_common.h>



#define USER_BUTTON_Pin         GPIO_PIN_13
#define USER_BUTTON_GPIO_Port   GPIOC

#define VBUS_SENSE_Pin          GPIO_PIN_2
#define VBUS_SENSE_GPIO_Port    GPIOC

#define UCPD_FLT_Pin            GPIO_PIN_14
#define UCPD_FLT_GPIO_Port      GPIOB

#define UCPD1_CC2_Pin           GPIO_PIN_15
#define UCPD1_CC2_GPIO_Port     GPIOB

#define LED_GREEN_Pin           GPIO_PIN_7
#define LED_GREEN_GPIO_Port     GPIOC

#define USART1_TX_Pin           GPIO_PIN_9
#define USART1_TX_GPIO_Port     GPIOA

#define USART1_RX_Pin           GPIO_PIN_10
#define USART1_RX_GPIO_Port     GPIOA

#define USB_OTG_FS_DM_Pin       GPIO_PIN_11
#define USB_OTG_FS_DM_GPIO_Port GPIOA

#define USB_OTG_FS_DP_Pin       GPIO_PIN_12
#define USB_OTG_FS_DP_GPIO_Port GPIOA

#define UCPD1_CC1_Pin           GPIO_PIN_15
#define UCPD1_CC1_GPIO_Port     GPIOA

#define UCPD_DBn_Pin            GPIO_PIN_5
#define UCPD_DBn_GPIO_Port      GPIOB

#define LED_BLUE_Pin            GPIO_PIN_7
#define LED_BLUE_GPIO_Port      GPIOB



void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);

  /* System interrupt init*/
}
void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    if(rtcHandle->Instance==RTC){
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
  
        }
        __HAL_RCC_RTC_ENABLE();
        __HAL_RCC_RTCAPB_CLK_ENABLE();
    }
}
/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    
    if(hadc->Instance==ADC1){
        /** Initializes the peripherals clock
        */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADCDAC;
        PeriphClkInit.AdcDacClockSelection = RCC_ADCDACCLKSOURCE_HSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}
/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance==ADC1){
        /* Peripheral clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();

    }
}
/**
* @brief RNG MSP Initialization
* This function configures the hardware resources used in this example
* @param hrng: RNG handle pointer
* @retval None
*/
void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hrng->Instance==RNG)
  {
  /* USER CODE BEGIN RNG_MspInit 0 */

  /* USER CODE END RNG_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RNG;
    PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_RNG_CLK_ENABLE();
  /* USER CODE BEGIN RNG_MspInit 1 */

  /* USER CODE END RNG_MspInit 1 */
  }

}

/**
* @brief RNG MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrng: RNG handle pointer
* @retval None
*/
void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng)
{
  if(hrng->Instance==RNG)
  {
  /* USER CODE BEGIN RNG_MspDeInit 0 */

  /* USER CODE END RNG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();
  /* USER CODE BEGIN RNG_MspDeInit 1 */

    /* Enable RNG reset state */
    __HAL_RCC_RNG_FORCE_RESET();

    /* Release RNG from reset state */
    __HAL_RCC_RNG_RELEASE_RESET();

  /* USER CODE END RNG_MspDeInit 1 */
  }
}
/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
#if 0
    if(huart->Instance==LPUART1)
    {
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
        PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK3;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
          Error_Handler();
        }
        
        __HAL_RCC_LPUART1_CLK_ENABLE();
        
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**LPUART1 GPIO Configuration
        PC0     ------> LPUART1_RX
        PC1     ------> LPUART1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);        
#endif
#if 1
    if(huart->Instance==LPUART1)
    {
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
        PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK3;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
          Error_Handler();
        }
        
        __HAL_RCC_LPUART1_CLK_ENABLE();
        
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**LPUART1 GPIO Configuration
        PC0     ------> LPUART1_RX
        PC1     ------> LPUART1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);        
#endif
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
        GPIO_InitStruct.Pin = USART1_TX_Pin|USART1_RX_Pin;
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
    else if(huart->Instance==UART4){
        /** Initializes the peripherals clock
        */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UART4;
        PeriphClkInit.Usart2ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_UART4_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if(huart->Instance==LPUART1){
        /* Peripheral clock disable */
        __HAL_RCC_LPUART1_CLK_DISABLE();
        /**LPUART1 GPIO Configuration
        PG7     ------> LPUART1_TX
        PG8     ------> LPUART1_RX
        */
        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_7|GPIO_PIN_8);
    }else if(huart->Instance==USART1){
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOA, USART1_TX_Pin|USART1_RX_Pin);
    }else if(huart->Instance==USART2){
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
    }
}


/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
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
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**SPI1 GPIO Configuration
        PA4     ------> SPI1_NSS
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
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);   
    }else if(hspi->Instance==SPI2){
        /* Peripheral clock enable */
        __HAL_RCC_SPI2_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();

        /**SPI3 GPIO Configuration
        PB3     ------> SPI2_SCK
        PB4     ------> SPI2_MISO
        PB5     ------> SPI2_MOSI
        */
        GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /* SPI3 interrupt Init */
    }else if(hspi->Instance==SPI3){
        /* Peripheral clock enable */
        __HAL_RCC_SPI3_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();


    }
}
/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    if(htim_base->Instance==TIM1){
        __HAL_RCC_TIM1_CLK_ENABLE();
    }else if(htim_base->Instance==TIM4){
        __HAL_RCC_TIM4_CLK_ENABLE();
    }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(htim->Instance==TIM1){
        __HAL_RCC_GPIOE_CLK_ENABLE();
        /**TIM1 GPIO Configuration
        PE9     ------> TIM1_CH1
        PE11     ------> TIM1_CH2
        PE13     ------> TIM1_CH3
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_13;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    }else if(htim->Instance==TIM4){
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**TIM4 GPIO Configuration
        PD15     ------> TIM4_CH4
        */
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    }
}
/**
* @brief TIM_Base MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
    if(htim_base->Instance==TIM1){
        /* Peripheral clock disable */
        __HAL_RCC_TIM1_CLK_DISABLE();
    }else if(htim_base->Instance==TIM4){
        /* Peripheral clock disable */
        __HAL_RCC_TIM4_CLK_DISABLE();
    }
}
