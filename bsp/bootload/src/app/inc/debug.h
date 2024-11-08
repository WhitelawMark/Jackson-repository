 /*
 * debug.h
 *
 *  Created on: 2022Äê10ÔÂ19ÈÕ
 *      Author: lwp edit
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__ 
/*
********************************************************************************
********************************************************************************
*/
#define LG_TIM                       7
#define LG_DMP                       6
#define LG_DBG                       5
#define LG_INF                       4
#define LG_WRN                       3
#define LG_ERR                       2
#define LG_FTL                       1
/*
********************************************************************************
********************************************************************************
*/
void system_log_entry(int level,u8_t *pmodule,char *fmt, ...);
void syslogsetlevel(u8_t level);

#define  bl_log(level, args...)  system_log_entry(level,"uboot",args); 
//#define  bl_log(level, args...)

void kprintf(const char* fmt,...);

#endif  /* __DEBUG_H__ */

