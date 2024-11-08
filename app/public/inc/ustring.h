/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */
#ifndef __USTRING_H
#define __USTRING_H    


double get_distance(double base_lati, double base_lng, double pos_lati, double pos_lng);
/*
********************************************************************************
********************************************************************************
*/ 
unsigned short ustrcrc16(unsigned char *ptr, unsigned short len);
unsigned char check_sum(const unsigned char *msg, unsigned int len);
unsigned short ustrcalcrc(unsigned char *pStr,unsigned int len) ;


signed char strseq_bitget(unsigned char *pcmd,int len ,int index);
signed char strseq_bitclr(unsigned char *pcmd,int len ,int index);
signed char strseq_bitset(unsigned char *pcmd,int len ,int index);
/*
********************************************************************************
********************************************************************************
*/ 
unsigned int get_le_dword(unsigned char *msg);
unsigned char ustr2hex(unsigned char *str);
unsigned char *ustrfind(unsigned char *pSrc,unsigned int srclen,unsigned char *pDest,unsigned int destlen) ;
int ustrcmpstrf(unsigned char *pSrc, const unsigned char *pDest, unsigned int count);
int ustrcmpstr(unsigned char *pSrc, unsigned char *pDest, unsigned int count) ;
unsigned int ustrlen(unsigned char *pStr);
unsigned int ustrflen(const unsigned char *pStr);
void ustrncpy(unsigned char *pDest,unsigned char *pSrc,unsigned int len);

unsigned int ustrdectostr(unsigned char *pDest,unsigned int maxlen,unsigned short dec);
unsigned int ustrhextodec(unsigned char *pHexStr,unsigned int len,unsigned short *pDec);
unsigned int ustrdectodec(unsigned char *pDecStr,unsigned int len,unsigned short *pDec); 

unsigned int ustrclr( unsigned char *pStr,unsigned short length);
int ustrtohex(char *buf, const char *str, int len);
int ustrhextostr(char *buf, const char *str, int len);

unsigned int str_average(unsigned int *pstr,u8_t strnum);
u16_t str_averageu16(u16_t *pstr,u8_t strnum);
u16_t ustraver(u16_t *pstr,u16_t len);
float ustraverf(float *pstr,u16_t len);

#endif
/*__USTRING_H*/
