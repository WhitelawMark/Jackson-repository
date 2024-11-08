 /*
 * Common.h
 *
 *  Created on: 2022Äê10ÔÂ19ÈÕ
 *      Author: lwp edit
 */

#ifndef __COMMON_H__
#define __COMMON_H__
/*
********************************************************************************
********************************************************************************
*/
/* Constants used by Serial Command Line Mode */
#define CMD_STRING_SIZE       128

/* Common routines */
#define IS_AF(c)  ((c >= 'A') && (c <= 'F'))
#define IS_af(c)  ((c >= 'a') && (c <= 'f'))
#define IS_09(c)  ((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  IS_AF(c) || IS_af(c) || IS_09(c)
#define ISVALIDDEC(c)  IS_09(c)
#define CONVERTDEC(c)  (c - '0')

#define CONVERTHEX_alpha(c)  (IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)   (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))

#define SerialPutString(x) Serial_PutString((uint8_t*)(x))
/*
********************************************************************************
********************************************************************************
*/
void Int2Str(uint8_t* str,int32_t intnum);
uint32_t Str2Int(uint8_t *inputstr,int32_t *intnum);
uint32_t GetIntegerInput(int32_t * num);
uint32_t SerialKeyPressed(uint8_t *key);
uint8_t GetKey(void);
void SerialPutChar(uint8_t c);
void Serial_PutString(uint8_t *s);
void GetInputString(uint8_t * buffP);
 
#endif /* __COMMON_H__ */
