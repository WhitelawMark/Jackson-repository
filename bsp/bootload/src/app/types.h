 /*
 * types.h
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */

#ifndef __TYPES__H
#define __TYPES__H

typedef unsigned char        u8_t;             // 无符号1个字节
typedef signed   char        s8_t;             // 有符号1个字节
typedef unsigned short       u16_t;            // 无符号2个字节
typedef signed   short       s16_t;            // 有符号2个字节
typedef unsigned int         u32_t;            // 无符号4个字节
typedef signed   int         s32_t;            // 有符号4个字节

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE    0
#endif
#ifndef NULL
#define NULL    (void *)0
#endif

#endif /* __TYPES__H */

