/*
 * types.h
 *
 *  Created on: 2023Äê7ÔÂ17ÈÕ
 *      Author: lwp
 */
#ifndef _TYPE_H
#define _TYPE_H  

/*
********************************************************************************
********************************************************************************
*/ 
typedef unsigned long           uint32;
typedef unsigned short          uint16;
typedef unsigned char           uint8;
typedef signed long             int32;
typedef signed short            int16;
typedef signed char             int8;

typedef unsigned char           u8_t;              
typedef signed   char           s8_t;              
typedef unsigned short          u16_t;             
typedef signed   short          s16_t;             
typedef unsigned int            u32_t;             
typedef signed   int            s32_t;             

typedef volatile unsigned long  vuint32;
typedef volatile unsigned short vuint16;
typedef volatile unsigned char  vuint8;


typedef unsigned int             sys_tick_t;

typedef signed char  		s8;
typedef signed short 		s16;
typedef signed long  		s32;
typedef signed long long  	s64;

typedef unsigned char  		u8;
typedef unsigned short 		u16;
typedef unsigned long  		u32;
typedef unsigned long long  u64;

typedef float				f32;
typedef double				f64;

typedef enum {FALSE = 0, TRUE = !FALSE} BoolStatus;

/*
********************************************************************************
********************************************************************************
*/ 
#define OS_COUNTOF(a)     (sizeof(a) / sizeof(a[0]))

/*
********************************************************************************
********************************************************************************
*/ 
typedef struct
{
    u8_t  Hour;
    u8_t  Minute;
    u8_t  Second;
    u8_t  Month;
    u8_t  Day;
    u8_t  Week;
    u16_t Year;
}RTC_TIME_DEF;  

typedef void (*IntNoteCallBack_t)(void);
#endif 
/* _TYPE_H */
