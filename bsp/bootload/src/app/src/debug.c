 /*
 * debug.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */
#include "app_lib.h"  
/*
********************************************************************************
********************************************************************************
*/ 
#define LOG_EMERG_KEY         "emerg"    /* system is unusable */
#define LOG_ALERT_KEY         "alert"    /* action must be taken immediately */
#define LOG_CRIT_KEY          "crit"     /* critical conditions */
#define LOG_ERR_KEY           "err"      /* error conditions */
#define LOG_ERROR_KEY         "error"    /* error conditions */
#define LOG_WARN_KEY          "warn"     /* warning conditions */
#define LOG_WARNING_KEY       "warning"  /* warning conditions */
#define LOG_NOTICE_KEY        "notice"   /* normal but signification condition */
#define LOG_INFO_KEY          "info"     /* informational */
#define LOG_DEBUG_KEY         "debug"    /* debug-level messages */
/*
********************************************************************************
********************************************************************************
*/ 
static u8_t Loglevel = LG_DMP;

/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  syslogsetlevel
**  功能描述:  设置调试输出等级
**  输入参数:  level：调试等级
**  输出参数:  无
**  返回参数:  无
**  备    注:  无
********************************************************************************
*/
void syslogsetlevel(u8_t level)
{
    Loglevel = level;
}

/*
********************************************************************************
**  函数名称:  sysloggetinfo
**  功能描述:  等级转换函数
**  输入参数:  level：调试等级
**  输出参数:  无
**  返回参数:  无
**  备    注:  无
********************************************************************************
*/
static u8_t *sysloggetinfo(u8_t level)
{
    u8_t *pinfo;

    switch(level){
        case LG_FTL:
             pinfo =LOG_EMERG_KEY;
             break;
        case LG_ERR:
             pinfo =LOG_ERROR_KEY;
             break;
        case LG_WRN:
             pinfo =LOG_WARN_KEY;
             break;
        case LG_INF:
             pinfo =LOG_INFO_KEY;
             break;
        case LG_DBG:
             pinfo =LOG_DEBUG_KEY; 
             break;
        case LG_DMP:
             pinfo =LOG_DEBUG_KEY;
             break;
        case LG_TIM:
             pinfo =LOG_DEBUG_KEY;
             break;
    }

    return pinfo;
}
/*
********************************************************************************
**  函数名称:  kprintf
**  功能描述:  替代printf使用（IAR 9.2版本无法使用printf）
**  输入参数:  level  ：调试等级
**             pmodule: 模块
**             info   : 输出信息
**  输出参数:  无
**  返回参数:  无
**  备    注:  无
********************************************************************************
*/
void kprintf(const char* fmt,...)
{  
    uint32_t ulen,i;
    uint8_t tmpbuf[256];
    
    va_list ap; 
    va_start(ap,fmt);
    vsprintf((char*)tmpbuf,fmt,ap);
    va_end(ap);
    
    ulen =strlen((const char*)tmpbuf);		//此次发送数据的长度
    for(i=0;i<ulen;i++){			//循环发送数据
        SerialPutChar(tmpbuf[i]);
    }
}
/*
********************************************************************************
**  函数名称:  system_log_entry
**  功能描述:  设置调试输出等级
**  输入参数:  level  ：调试等级
**             pmodule: 模块
**             info   : 输出信息
**  输出参数:  无
**  返回参数:  无
**  备    注:  无
********************************************************************************
*/
void system_log_entry(int level, u8_t *pmodule, char *fmt, ...)
{
    va_list ap;
    char info[256] = {0};

    if (level > Loglevel) {
        return;
    }

    va_start(ap, fmt);
    vsnprintf(info, sizeof(info), fmt, ap);
    va_end(ap);

    kprintf("[%s-%s]: %s\r\n", pmodule, sysloggetinfo(level), info);
}

