/*
 *  show.c
 *
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
 
#include "app_lib.h"

/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
Function      : stm32_flash_sample
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static void stm32_flash_sample(void)
{
     u8_t tmpbuf[128];
     rt_uint32_t addr;
   
     memset(tmpbuf,0xaa,128);
     addr=0x080FE000;
     stm32_flash_erase(addr,  4096*2);
     stm32_flash_write(addr,tmpbuf,128);
}
/*
********************************************************************************
*Function    : rtc_sample
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
#define RTC_NAME "rtc"
static int rtc_sample(int argc, char *argv[])
{
    rt_err_t ret = RT_EOK;
    time_t now;
    rt_device_t device = RT_NULL;
 
    device = rt_device_find(RTC_NAME);
    if (!device){
        rt_kprintf("find %s failed!", RTC_NAME);
        return RT_ERROR;
    }
 
    if(rt_device_open(device, 0) != RT_EOK){
        rt_kprintf("open %s failed!", RTC_NAME);
        return RT_ERROR;
    }
 
    /* 设置日期 */
    ret = set_date(2022, 12, 1);
    if (ret != RT_EOK){
        rt_kprintf("set RTC date failed\n");
        return ret;
    }
 
    /* 设置时间 */
    ret = set_time(18, 24, 00);
    if (ret != RT_EOK){
        rt_kprintf("set RTC time failed\n");
        return ret;
    }
 
    /* 延时3秒 */
    rt_thread_mdelay(3000);
 
    /* 获取时间 */
    now = time(RT_NULL);
    rt_kprintf("%s\n", ctime(&now));
 
    return ret;
}

/*
********************************************************************************
*Function    : user_alarm_callback
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void user_alarm_callback(rt_alarm_t alarm, time_t timestamp)
{
    rt_kprintf("user alarm callback function.\n");
}
/*
********************************************************************************
*Function    : alarm_sample
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void alarm_sample(void)
{
    rt_device_t dev = rt_device_find("rtc");
    struct rt_alarm_setup setup;
    struct rt_alarm * alarm = RT_NULL;
    static time_t now;
    struct tm p_tm;
 
    if (alarm != RT_NULL)
        return;
 
    /* 获取当前时间戳，并把下3秒时间设置为闹钟时间 */
    now = time(NULL) + 3;
    gmtime_r(&now,&p_tm);
 
    setup.flag = RT_ALARM_SECOND;
    setup.wktime.tm_year = p_tm.tm_year;
    setup.wktime.tm_mon = p_tm.tm_mon;
    setup.wktime.tm_mday = p_tm.tm_mday;
    setup.wktime.tm_wday = p_tm.tm_wday;
    setup.wktime.tm_hour = p_tm.tm_hour;
    setup.wktime.tm_min = p_tm.tm_min;
    setup.wktime.tm_sec = p_tm.tm_sec;
 
    alarm = rt_alarm_create(user_alarm_callback, &setup);
    if(RT_NULL != alarm){
        rt_alarm_start(alarm);
    }
}
/*
********************************************************************************
Function      : show_usage
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static void show_usage(void)
{
    rt_kprintf("Please input '<flash <erase> <chip> >' \n"); 
}
/*
********************************************************************************
Function      : board_cmd
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void board_cmd(int argc, char** argv)
{
    if (argc < 2 ) {
        show_usage();
        return;
    }
    if(strcmp(argv[1], "rtc") == 0) {
        rtc_sample(argc,argv);
    }else if(strcmp(argv[1], "alarm") == 0) {
        alarm_sample();
    }else if(strcmp(argv[1], "flash") == 0) {
        if(strcmp(argv[1], "w25") == 0) {
            W25QXXEraseChip();
        }else if(strcmp(argv[1], "stm") == 0) {
            stm32_flash_sample();
        }
    }else{
        show_usage();
    }
}
MSH_CMD_EXPORT(board_cmd,board_cmd <mode|tick|time|...> );

  


  