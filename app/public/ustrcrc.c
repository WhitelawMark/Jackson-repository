/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */
#include "type.h" 
#include <stdio.h>
#include <string.h>
/*
********************************************************************************
********************************************************************************
*/
//CRC余式表
static const unsigned int UstrCrcTable[16] =
{    
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
    0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF
};
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
**  函数名称:  ustrcalcrc
**  功能描述:  计算CRC
**  输入参数:  pSrc    -- 指向源字符串的首地址
**             len     --    待计算校验值字符串的长度
**  输出参数:  crcval  -- 计算出的CRC值
**  返回参数:  无
********************************************************************************
*/
unsigned short ustrcalcrc(unsigned char *pStr,unsigned int len) 
{    
    unsigned short crc;
    unsigned char temp;


    crc = 0;
    while(len--) {
        // 暂存CRC的高4位
        temp = ((unsigned char)(crc >> 8)) >> 4;
        // 取CRC的低12位                 
        crc <<= 4;                            
        
        // CRC高4位和本字节的高4位相加后查表计算CRC,再加前次CRC的余数
        crc ^= UstrCrcTable[temp^(*pStr >> 4)];                

        // 暂存CRC的高4位
        temp = ((unsigned char)(crc >> 8)) >> 4;
        // 取CRC的低12位                 
        crc <<= 4;                                
 
        // CRC高4位和本字节的低4位相加后查表计算CRC,再加前次CRC的余数
        crc ^= UstrCrcTable[temp^(*pStr & 0x0f)]; 
 
        pStr++;
    }

    return    crc;
} 
/*
********************************************************************************
**  函数名称:  ustrcrc16
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
**  其    他:   
********************************************************************************
*/
u16_t ustrcrc16(u8_t *ptr, u16_t len) 
{
	const u16_t crctalbeabs[] = { 
		0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 
		0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400 
	};
    u16_t crc = 0xffff; 
    u16_t i;
    u8_t ch;
 
    for (i = 0; i < len; i++) {
        ch = *ptr++;
        crc = crctalbeabs[(ch ^ crc) & 15] ^ (crc >> 4);
        crc = crctalbeabs[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
    } 
    
    return crc;
}
/*
********************************************************************************
**  函数名称:  check_sum
**  功能描述:   
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
u8_t check_sum(const u8_t *msg, u32_t len)
{
    u32_t i;
    u8_t sum = 0;

    for (i = 0; i < len; i++) {
        sum += msg[i];
    }

    return sum;
}
/*
********************************************************************************
**  函数名称:  check_sum
**  功能描述:   
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
u16_t ucheck_sum(const u8_t *msg, u32_t len)
{
    u32_t i;
    u16_t sum = 0;

    for (i = 0; i < len; i++) {
        sum += msg[i];
    }

    return sum;
}
/*
********************************************************************************
**  函数名称:  parity_crc
**  功能描述:  生成奇偶校验  如果1的个数为奇数个，则返回1，如果1的个数为偶数个，则返回0。
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char parity_crc(unsigned char data)
{
	int val = 0;
	while(data){
		val ^= data;	//val 和x进行异或运算
		data >>= 1;//data右移一位
	}	
	return (unsigned char)(val & 0x1);			//取末位运算. val的二进制形式最后一位位1则返回1，为0则返回0.			
}
/*
********************************************************************************
**  函数名称:  sum_create
**  功能描述:   
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char sum_create(unsigned char *data,unsigned short ulen,unsigned char head)
{
	unsigned short sum = 0;  
 	unsigned short i = 0;	

	for (i = head;i < ulen;i++){
		sum += data[i];
		if(sum > 0xff){
			sum = ~sum;						
			sum += 1;						 
		}
	}
	sum = sum & 0xff;

	return (unsigned char)(~sum); 
}