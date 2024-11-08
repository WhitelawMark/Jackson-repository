 /*
 * drv_uart.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */

#include "app_lib.h"
#include "board.h"


#define DRV_PPH_UART     USART0


static UART_HandleTypeDef huart;

/*
********************************************************************************
**  函数名称:  uart_init
**  功能描述:  无  
**  输入参数:  无  
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
    Error_Handler();
}
/*
********************************************************************************
**  函数名称:  uart_init
**  功能描述:  
**  输入参数:  无  
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
#if 1
void drv_uart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  
  
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
        Error_Handler();
    }
    /* USART2 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**
    modify rong
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
 
	huart.Instance = USART1;
	huart.Init.BaudRate = 115200;
	huart.Init.WordLength = UART_WORDLENGTH_8B;
	huart.Init.StopBits = UART_STOPBITS_1;
	huart.Init.Parity = UART_PARITY_NONE;
	huart.Init.Mode = UART_MODE_TX_RX;
	huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart.Init.OverSampling = UART_OVERSAMPLING_16;
	huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart) != HAL_OK){
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK){
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK){
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart) != HAL_OK){
		Error_Handler();
	}
}
#endif

#if 0
void drv_uart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  
  
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart1ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK){
        Error_Handler();
    }
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**
    modify The small package chip does not
    USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
 
	huart.Instance = USART1;
	huart.Init.BaudRate = 115200;
	huart.Init.WordLength = UART_WORDLENGTH_8B;
	huart.Init.StopBits = UART_STOPBITS_1;
	huart.Init.Parity = UART_PARITY_NONE;
	huart.Init.Mode = UART_MODE_TX_RX;
	huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart.Init.OverSampling = UART_OVERSAMPLING_16;
	huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart) != HAL_OK){
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK){
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK){
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart) != HAL_OK){
		Error_Handler();
	}
}
#endif
/*
********************************************************************************
**  函数名称:  SerialKeyPressed
**  功能描述:  判断终端是否有值输入
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
uint32_t SerialKeyPressed(uint8_t *key)
{
    int res;
    
    res = HAL_UART_Receive(&huart, key, 1, 500);
    if ( res == HAL_OK){
        return 1;
    }else{
        return 0;
    }
} 
/*
********************************************************************************
*Function    : DrvUartGetChar
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
s8_t DrvUartGetChar(u8_t *ch)
{    
    int res;
#if 0  
    res = HAL_UART_Receive(&huart, ch, 1, 500);
    if(HAL_OK == res){
        return 1;
    }
#else  
    if (__HAL_UART_GET_FLAG(&(huart), UART_FLAG_RXNE) != RESET)/*串口接收中断标志非空*/
    {
        *ch = huart.Instance->RDR & 0xff;/*还回接收的数据*/
        return 1;
    }else{
        if (__HAL_UART_GET_FLAG(&(huart), UART_FLAG_ORE) != RESET)
        {
            __HAL_UART_CLEAR_OREFLAG(&huart);
        }
        if (__HAL_UART_GET_FLAG(&(huart), UART_FLAG_NE) != RESET)
        {
            __HAL_UART_CLEAR_NEFLAG(&huart);
        }
        if (__HAL_UART_GET_FLAG(&(huart), UART_FLAG_FE) != RESET)
        {
            __HAL_UART_CLEAR_FEFLAG(&huart);
        }
        if (__HAL_UART_GET_FLAG(&(huart), UART_FLAG_PE) != RESET)
        {
            __HAL_UART_CLEAR_PEFLAG(&huart);
        }   
    }
#endif
    return 0;
}
/*
********************************************************************************
**  函数名称:  SerialPutChar
**  功能描述:  输出一个字符
**  输入参数:  要输出的字符
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void SerialPutChar(uint8_t ch)
{
    __HAL_UART_CLEAR_FLAG(&(huart), UART_FLAG_TC);
    
    huart.Instance->TDR = (ch& 0xFFU);  

    while (__HAL_UART_GET_FLAG(&(huart), UART_FLAG_TC) == RESET);
    
} 
/*
********************************************************************************
**  函数名称:  Serial_PutString
**  功能描述:  输出一个字符
**  输入参数:  要输出的字符
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void Serial_PutString(uint8_t *p_string)
{
    uint16_t length = 0;

    while (p_string[length] != '\0')
    {
        SerialPutChar(p_string[length]);
        length++;
    }
}


