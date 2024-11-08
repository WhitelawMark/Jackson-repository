 /*
 * drv_uart.h
 *
 *  Created on: 2022Äê10ÔÂ19ÈÕ
 *      Author: lwp edit
 */
#ifndef _DRV_UART_H
#define _DRV_UART_H
/*
********************************************************************************
********************************************************************************
*/

#define SerialPutChar  DrvUartPutChar
#define DrvUartSendStr DrvUartPutStr


extern UART_HandleTypeDef UartHandle;
/*
********************************************************************************
********************************************************************************
*/
void drv_uart_init(void);
void SerialPutChar(uint8_t c);
void SerialBufClear(void);
s8_t DrvUartGetChar(u8_t *c_ptr);
void DrvUartPutStr(u8_t *str, u16_t len);

HAL_StatusTypeDef DrvUART_Receive( uint8_t *pData, uint16_t Size, uint32_t timeout);
#endif 
/* _DRV_UART_H */

