 /*
 * common.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */
#include "app_lib.h"


/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  Int2Str
**  功能描述:  Convert an Integer to a string
**  输入参数:  str: The string
**             intnum: The integer to be converted
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void Int2Str(uint8_t* str, int32_t intnum)
{
    uint32_t i, Div = 1000000000, j = 0, Status = 0;

    for (i = 0; i < 10; i++){
        str[j++] = (intnum / Div) + 48;

        intnum = intnum % Div;
        Div /= 10;
        if ((str[j-1] == '0') & (Status == 0)){
            j = 0;
        }else{
            Status++;
        }
    }
}
/*
********************************************************************************
**  函数名称:  Str2Int
**  功能描述:  Convert a string to an integer
**  输入参数:  inputstr: The string to be converted
**             intnum: The integer value
**  输出参数:  无
**  返回参数:  无
*********************************************************************************
*/
uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
    uint32_t i = 0, res = 0;
    uint32_t val = 0;

    if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X')){
        if (inputstr[2] == '\0'){
            return 0;
        }
        for (i = 2; i < 11; i++){
            if (inputstr[i] == '\0'){
                *intnum = val;
                /* return 1; */
                res = 1;
                break;
            }
            if (ISVALIDHEX(inputstr[i])){
                val = (val << 4) + CONVERTHEX(inputstr[i]);
            }else{
                /* Return 0, Invalid input */
                res = 0;
                break;
            }
        }
        /* Over 8 digit hex --invalid */
        if (i >= 11){
            res = 0;
        }
    }else{                                                                         /* max 10-digit decimal input */
    
        for (i = 0;i < 11;i++){
            if (inputstr[i] == '\0'){
                *intnum = val; 
                res = 1;                                                               /*  return 1 */
                break;
            }else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0)){
                val = val << 10;
                *intnum = val;
                res = 1;
                break;
            }else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0)){
                val = val << 20;
                *intnum = val;
                res = 1;
                break;
            }else if (ISVALIDDEC(inputstr[i])){
                val = val * 10 + CONVERTDEC(inputstr[i]);
            }else{
                /* return 0, Invalid input */
                res = 0;
                break;
            }
        }
        /* Over 10 digit decimal --invalid */
        if (i >= 11){
            res = 0;
        }
    }

    return res;
}
/*
********************************************************************************
**  函数名称:  GetIntegerInput
**  功能描述:  从终端获取一个整型的数据
**  输入参数:  无
**  输出参数:  无
**  返回参数:  1 正确
**             0 错误
********************************************************************************
*/
uint32_t GetIntegerInput(int32_t * num)
{
    uint8_t inputstr[16];

    while (1){
        GetInputString(inputstr);
        if (inputstr[0] == '\0') continue;
        if ((inputstr[0] == 'a' || inputstr[0] == 'A') && inputstr[1] == '\0'){
          SerialPutString("User Cancelled \r\n");
          return 0;
        }

        if (Str2Int(inputstr, num) == 0){
            SerialPutString("Error, Input again: \r\n");
        }else{
            return 1;
        }
    }
}
/*
********************************************************************************
**  函数名称:  GetKey
**  功能描述:  获取终端输入值
**  输入参数:  无
**  输出参数:  无
**  返回参数:  输入的值
********************************************************************************
*/
uint8_t GetKey(void)
{
    uint8_t key = 0;
    uint32_t count=0;
    /* Waiting for user input */
#if 1
    while (1){
        if (DrvUartGetChar((uint8_t*)&key)) {
            break;
        }
        delay_1ms(20);
        if(count++>1000){
            break;
        }
    }
#else
    while (1){
        if (DrvUartGetChar((uint8_t*)&key)) {
            break;
        }
        if(count++>20){
            break;
        }
    }
    
#endif
    return key;
}
/*
********************************************************************************
**  函数名称:  GetInputString
**  功能描述:  获取一串字符，从终端
**  输入参数:  buff: The input string
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void GetInputString (uint8_t * buffP)
{
    uint32_t bytes_read = 0;
    uint8_t c = 0;
    do
    {
        c = GetKey();
        if (c == '\r')
            break;
        if (c == '\b'){ /* Backspace */
            if (bytes_read > 0){
                SerialPutString("\b \b");
                bytes_read --;
            }
            continue;
        }
        if (bytes_read >= CMD_STRING_SIZE ){
            SerialPutString("Command string size overflow\r\n");
            bytes_read = 0;
            continue;
        }
        if (c >= 0x20 && c <= 0x7E){
            buffP[bytes_read++] = c;
            SerialPutChar(c);
        }
    }
    while (1);
    SerialPutString(("\n\r"));
    buffP[bytes_read] = '\0';
}

