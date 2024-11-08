/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */
#include <stdio.h>
#include <string.h>
#include "type.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ch_isupper(c)    (((c)>='A')&&((c)<='Z'))
#define ch_islower(c)    (((c)>='a')&&((c)<='z'))
#define ch_isdigit(c)    (((c)>='0')&&((c)<='9'))
#define ch_isspace(c)    ((c)==' ' || (c)=='\t'  || (c)=='\r' || (c)=='\n')
/*
******************************************************************************** 
*Function    : ustr2hex
*Description : 字符串转ask码
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
u8_t ustr2hex(u8_t *str)
{
    u8_t ihex;
    if(str[0]!=' '){
        ihex = str[0]-0x30;
        ihex *= 10;
        ihex += str[1]-0x30;
    }else{
        ihex = str[1]-0x30;
    }
    return ihex;
}
/*
******************************************************************************** 
*Function    : get_le_dword
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
u32_t get_le_dword(u8_t *msg)
{
    return (((((u32_t)(msg[3])) << 24) & 0xFF000000) |
            ((((u32_t)(msg[2])) << 16) & 0x00FF0000) |
            ((((u32_t)(msg[1])) << 8 ) & 0x0000FF00) | msg[0]);
}
/*
******************************************************************************** 
*Function    : ustrflen
*Description : 计算字符串的长度
*Input       : pStr    -- 指向字符串的首地址
*Output      :
*Return      : 返回字符串的长度
*Others      : 字符串必须以'\0'为结束标志
******************************************************************************** 
*/
unsigned int ustrflen(const unsigned char *pStr)
{
    unsigned int length = 0;

    while(*pStr++ != 0) {
        length++;  
    }

    return length;
} 
/*
******************************************************************************** 
*Function    : ustrclr
*Description : 清除缓存中的数据
*Input       : pStr    -- 指向字符串的首地址
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
unsigned int ustrclr( unsigned char *pStr,unsigned short length)
{    
    unsigned short index;
    
    for(index=0;index<length;index++){
        *(pStr+index) = 0; 
    }

    return 0;
} 
/*
******************************************************************************** 
*Function    : ustrlen
*Description : 计算字符串的长度
*Input       : pStr    -- 指向字符串的首地址
*Output      : 返回字符串的长度
*Return      :
*Others      : 字符串必须以'\0'为结束标志
******************************************************************************** 
*/
unsigned int ustrlen(unsigned char *pStr)
{
    unsigned int length = 0;

    while(*pStr++ != 0) {
        length++;  
    }

    return length;
}
/*
******************************************************************************** 
*Function    : ustrcmpstr
*Description : 目标字符串与源字符串进行比较
*Input       : pSrc    -- 指向源字符串的首地址
*              pDest   -- 指向目标字符串的首地址
*              count   -- 比较的长度
*Output      :
*Return      : return 0  if pDest = pSrc
*              return > 0 if pDest > pSrc
*              return < 0 if pDest < pSrc    or count=0
*Others      :
******************************************************************************** 
*/
int ustrcmpstr(unsigned char *pSrc, unsigned char *pDest, unsigned int count)
{
    int r;

    if(count == 0) {
        return -1;
    }

    while(count) {
        r = *pDest - *pSrc;
        if(r != 0) {
            return r;
        }
        pDest++;
        pSrc++;
        count--;
    }

    return 0;
}
/*
******************************************************************************** 
*Function    : ustrfind
*Description : 源字符串中查找目标字符串
*Input       : pSrc     -- 指向源字符串的首地址
*              srclen   -- 源字符串的长度
*              pDest    -- 指向目标字符串的首地址
*              destlen  -- 目标字符串的长度
*Output      :
*Return      : 找到则返回目标字符串在源字符串中的首地址,未找到返回NULL
*Others      :
******************************************************************************** 
*/
unsigned char *ustrfind(unsigned char *pSrc,unsigned int srclen,unsigned char *pDest,unsigned int destlen)
{
    unsigned int matchlen,curlen;

    if((destlen > srclen) || (destlen == 0)) {
        return (unsigned char *)0;
    }

    curlen = 0;
    matchlen = 0;
    while(curlen < srclen) {
        if(pSrc[curlen] != pDest[matchlen]) {
            curlen -= matchlen;
            matchlen = 0;
        }
        else {
            matchlen++;
        }
        curlen++;

        if(matchlen >= destlen) {
            return (unsigned char *)(pSrc+curlen-matchlen);
        }
    }
    
    return (unsigned char *)0;    
}
/*
******************************************************************************** 
*Function    : ustrcmpstrf
*Description : 目标字符串与源字符串进行比较
*Input       : pSrc    -- 指向源字符串的首地址
*              pDest   -- 指向目标字符串的首地址
*              count   -- 比较的长度
*Output      :
*Return      : return 0  if pDest = pSrc
*              return > 0 if pDest > pSrc
*              return < 0 if pDest < pSrc    or count=0
*Others      :
******************************************************************************** 
*/
/*******************************************************************************
* 函数名称 : ustrcmpstrf
* 函数功能 : 目标字符串与源字符串进行比较
* 入口参数 : pSrc    -- 指向源字符串的首地址
             pDest   -- 指向目标字符串的首地址
             count   -- 比较的长度
* 出口参数 : return 0  if pDest = pSrc
             return > 0 if pDest > pSrc
             return < 0 if pDest < pSrc    or count=0
* 注意事项 : 
*******************************************************************************/

int ustrcmpstrf(unsigned char *pSrc, const unsigned char *pDest, unsigned int count)
{
    int r;

    if(count == 0) {
        return -1;
    }

    while(count) {
        r = *pDest - *pSrc;
        if(r != 0) {
            return r;
        }
        pDest++;
        pSrc++;
        count--;
    }

    return 0;
}
/*
******************************************************************************** 
*Function    : ustrncpy
*Description : 将源字符串拷贝到目标字符串中
*Input       : pSrc    -- 指向源字符串的首地址
*              pDest   -- 指向目标字符串的首地址
*              len     -- 要拷贝字符串的长度
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
void ustrncpy(unsigned char *pDest,unsigned char *pSrc,unsigned int len)
{
    if((pDest == (unsigned char*)0) || (pSrc == (unsigned char *)0)) {
        return;
    }

    while(len) {
        *pDest++ = *pSrc++;
        len--;
    }
}
/*
******************************************************************************** 
*Function    : ustrdectostr
*Description : 将十进制转化成字符串
*Input       : pDest          -- 转化后字符串存放的首地址
*              destmaxlen     -- 存放BUF的最大长度
*              dec            -- 待转化的数据
*Output      : 转换后产生的字符串的长度
*Return      :
*Others      :如果BUF最大长度小于字符串转化后的实际字符串的长度,后面的
*             数据的将被丢弃
******************************************************************************** 
*/
unsigned int ustrdectostr(unsigned char *pDest,unsigned int maxlen,unsigned short dec)
{
    unsigned int i,cnt;
    unsigned char tmpbuf[5];
    
    if(maxlen == 0) {
        return 0;
    }
    
    //65535 => 5
    for(cnt = 0; cnt < 5; cnt++) {
        tmpbuf[cnt] = dec%10 + '0';
        dec /= 10;
        if(dec == 0) {
            cnt++;
            break;
        }
    }
    
    if(maxlen > cnt) {
        maxlen = cnt;
    }
    
    for(i = 0; i < maxlen; i++) {
        cnt--;
        pDest[i] = tmpbuf[cnt];
    }
    
    return i;
}
/*
******************************************************************************** 
*Function    : ustrhextodec
*Description : 将十六进制的字符串转化成十进制
*Input       : pDecStr  -- 指向十六进制形式的字符串首地址
*              len      -- 字符串的长度
*              pDest    -- 指向目标的首地址(大小unsigned int 类型的大小)
*Output      : 0        --- 转换成功
*            : !0       --- 转换失败
*Return      :
*Others      : 最多只能支持unsigned int 数据长度
******************************************************************************** 
*/
unsigned int ustrhextodec(unsigned char *pHexStr,unsigned int len,unsigned short *pDec)
{
    unsigned int i,temp;
    
    if(len == 0){
        return 1;
    } 
    
    temp = 0;
    for(i = 0; i < len; i++) {
        
        if(temp > 4095) {
            return 1;
        }
        
        //temp = temp*16 = temp << 4;
        temp <<= 4;
        
        if((pHexStr[i] >= '0') && (pHexStr[i] <= '9')) {
            temp += pHexStr[i]-'0';
        }
        else if((pHexStr[i] >= 'a') && (pHexStr[i] <= 'f')) {
            temp += (10+(pHexStr[i]-'a'));
        }
        else if((pHexStr[i] >= 'A') && (pHexStr[i] <= 'F')) {
            temp += (10+(pHexStr[i]-'A'));
        }
        else {
            return 1;
        }
    }
    
    if(temp > 0xffff) {
        return 1;
    }
    
    //结果
    *pDec = (unsigned short)temp;
     
    return 0;
}
/*
********************************************************************************
*Function    : ustrdectodec
*Description : 将十进制的字符串转化成十进制
*Input       : pDecStr  -- 指向十进制形式的字符串首地址
*              len      -- 字符串的长度
*              pDest    -- 指向目标的首地址(大小unsigned int 类型的大小)
*Output      : 0        -- 转换成功  
*            : !0       -- 转换失败
*Return      :
*Others      : 最多只能支持unsigned int 数据长度  
********************************************************************************
*/
unsigned int ustrdectodec(unsigned char *pDecStr,unsigned int len,unsigned short *pDec)
{
    unsigned int i,temp;
    
    if(len == 0) {
        return 1;
    }
    
    temp = 0;
    for(i = 0; i < len; i++) {
        if(temp > 6553) {
            return 1;
        }
        //temp = temp*10 = temp*8+temp*2 = (temp << 3) + (temp << 1);
        temp = ((temp << 3) + (temp << 1));    
        
        if((pDecStr[i] >= '0') && (pDecStr[i] <= '9')) {                
            temp += (pDecStr[i]-'0');    
        }
        else {
            return 1;
        }
    
    }
    
    if(temp > 0xffff) {
        return 1;
    }
    
    //结果
    *pDec = (unsigned short)temp;
    
    return 0;        
}
/*
********************************************************************************
*Function    : strseq_bitset
*Description :
*Input       :
*Output      :
*Return      :
*Others      :  
********************************************************************************
*/
#define BYTE_SIZE  8

signed char strseq_bitset(char *pcmd,int len ,int index)
{
    int byte_index;
    int bit_index;
 
    if(pcmd==NULL || len == 0){
        return -1;
    }
    
    byte_index = index/BYTE_SIZE;
    bit_index = index%BYTE_SIZE ;
  
    if(byte_index > len) {
        return -2;
    }
    
    pcmd[byte_index] |=(unsigned int)(1<<bit_index);
    
    return 0;
}
/*
********************************************************************************
*Function    : strseq_bitclr
*Description :
*Input       :
*Output      :
*Return      :
*Others      :  
********************************************************************************
*/
signed char strseq_bitclr(char *pcmd,int len ,int index)
{
    int byte_index;
    int bit_index;
       
    if(pcmd==NULL || len == 0){
        return -1;
    }
    
    byte_index = index/BYTE_SIZE;
    bit_index = index%BYTE_SIZE ;
  
    if(byte_index > len) {
        return -2;
    }
    
    pcmd[byte_index] &= ~(unsigned int)(1<<bit_index);
    
    return 0;
}
/*
********************************************************************************
*Function    : strseq_bitget
*Description :
*Input       :
*Output      :
*Return      :
*Others      :   
********************************************************************************
*/ 
signed char strseq_bitget(char *pcmd,int len ,int index)
{
    static int byte_index;
    static int bit_index;
       
    if(pcmd==NULL || len == 0){
        return -1;
    }
     
    byte_index = index/BYTE_SIZE;
    bit_index = index%BYTE_SIZE ;
  
    if(byte_index > len) {
        return -2;
    }

    if(pcmd[byte_index]&(1<<bit_index)){
        return 1;
    }
    return  0;
}
/*
********************************************************************************
*Function    : ustrtohex
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 目标大小可以是原来/2
********************************************************************************
*/
int ustrtohex(char *buf, const char *str, int len)
{
    char *dst = buf;
    char ch, hex;

    if (len & 0x01) {
        return -1;
    }

    while (len-- > 0) {
        ch = *str++;
        if (ch_isdigit(ch)) {
            ch -= '0';
        } else if (ch_isupper(ch)) {
            ch -= 'A';
            ch += 10;
        } else if (ch_islower(ch)) {
            ch -= 'a';
            ch += 10;
        } else {
            return -1;
        }

        if (ch > 0x0F) {
            return -1;
        }

        if (len & 0x01) {
            hex = (ch << 4) & 0xF0;
        } else {
            hex |= (ch & 0x0F);
            *dst++ = hex;
        }
    }

    return (int)(dst - buf);
}

/*
********************************************************************************
*Function    : ustrhextostr
*Description :
*Input       :
*Output      :
*Return      :
*Others      : 目标大小必须是原来大小的两倍+1
********************************************************************************
*/
int ustrhextostr(char *buf, const char *str, int len)
{
    static const char digits[]= "0123456789ABCDEF";
    char *dst = buf;
    char ch;

    while (len-- > 0) {
        ch = *str++;
        *dst++ = digits[(ch >> 4) & 0x0F];
        *dst++ = digits[ch & 0x0F];
    }
    *dst = '\0';

    return (int)(dst-buf);
}

