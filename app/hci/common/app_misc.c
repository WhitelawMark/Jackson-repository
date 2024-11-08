/*
 * app_init.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include "app_lib.h"  

#define DBG_TAG "misc"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/ 
static struct {
     u8_t     bootvs[4];
     char     CompileTime[6];
     uint32_t UDID[3];
     char     ota_stat;
} Board;

#ifdef RT_USING_RTC
static u8_t g_sys_time=0;
#endif

static PIT_TIMER rtc_timer;

/*
********************************************************************************
********************************************************************************
*/ 
#define BOOT_VS_MSG       (__IO uint8_t*)(0x08001000 ) 
/*
********************************************************************************
Function      : app_version
Description   :   
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void app_version(void)
{
    rt_kprintf("****************************************************************\r\n");
    rt_kprintf("%s \r\n",SYS_CMPY_NAME);
    rt_kprintf("UBOOT    Version : V%c.%c.%c \r\n",Board.bootvs[0],Board.bootvs[1],Board.bootvs[2]);    
    rt_kprintf("Hardware Type    : %s %s \r\n",SYS_BOARD_NAME,SYS_BOARD_TYPE); 
    rt_kprintf("Hardware Version : V%s \r\n",SYS_BOARD_VERSION);
    rt_kprintf("Software Version : V%s \r\n",SYS_SOFE_VERSION);
  
    rt_kprintf("Compile     Time : %02d/%02d/%02d %02d/%02d/%02d \n",
               Board.CompileTime[0],Board.CompileTime[1],Board.CompileTime[2],
               Board.CompileTime[3],Board.CompileTime[4],Board.CompileTime[5]);
    
    rt_kprintf("Cpuid            : %X%X%X \r\n", Board.UDID[0],Board.UDID[1],Board.UDID[2]);
    rt_kprintf("****************************************************************\r\n");
} 
/*
********************************************************************************
Function      : app_get_softvs
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
char *app_get_softvs(void)
{
    return Board.CompileTime;
}
/*
********************************************************************************
Function      : app_get_softvs
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
void app_set_otastat(u8_t stat)
{
    Board.ota_stat = stat;
}
/*
********************************************************************************
Function      : app_softvs_init
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
void app_softvs_init(void)
{
    const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    u8_t date[12]=__DATE__;  // Aug 28 2016
    u8_t time[9]=__TIME__;// 17:17:12
    u8_t smonth[4];
    
    for(u8_t i=0;i<3;i++){
        smonth[i]=date[i];
    }
    smonth[3]=0;
    Board.CompileTime[1] = (strstr(month_names,(const char*) smonth)-month_names)/3+1;/*1月*/
    Board.CompileTime[2] = ustr2hex(&date[4]);  /*9号*/      
    Board.CompileTime[0] = ustr2hex(&date[9]);   /*24年*/
    Board.CompileTime[3] = ustr2hex(&time[0]);   /*16时*/
    Board.CompileTime[4] = ustr2hex(&time[3]);   /*13分*/
    Board.CompileTime[5] = ustr2hex(&time[6]);  /*06秒*/
    
    memcpy( Board.bootvs,(uint8_t*)BOOT_VS_MSG,4);/*访问flash 某个位置的值给它*/
}
/*
********************************************************************************
Function      : app_get_chip_id
Description   :  
Input         :
Output        :
Return        :
Others        : 会死机
********************************************************************************
*/ 

void app_get_chip_id(void)
{
    Board.UDID[0] = *(__IO uint32_t*)(0x0BFA0700);
    Board.UDID[1] = *(__IO uint32_t*)(0x0BFA0700+0x04);
    Board.UDID[2] = *(__IO uint32_t*)(0x0BFA0700+0x08);
}
/*
********************************************************************************
Function      : app_get_chip_id
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 

u32_t *board_get_cpuid(void)
{
    return Board.UDID;
}
/*
********************************************************************************
Function      : app_parm_init
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
void app_parm_init(void)
{
    u8_t tmpval_u8;

    /*1、飞行模式 */
    PPItemRead(PP_AIRPLANE_MODE, &tmpval_u8,  PPItemSize(PP_AIRPLANE_MODE));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_AIRPLANE_MODE, &tmpval_u8,  PPItemSize(PP_AIRPLANE_MODE));
    }
    LOG_D("AIRPLANE_MOD:  %d", tmpval_u8);  
    
    /*2、工作模式 */
    PPItemRead(PP_WORK_MODE, &tmpval_u8,  PPItemSize(PP_WORK_MODE));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_WORK_MODE, &tmpval_u8,  PPItemSize(PP_WORK_MODE));
    }
    LOG_D("WORK_MODE:  %d", tmpval_u8);  
    
    
    /*3、GPS运行模式 */
    PPItemRead(PP_GPS_MODE, &tmpval_u8,  PPItemSize(PP_GPS_MODE));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_GPS_MODE, &tmpval_u8,  PPItemSize(PP_GPS_MODE));
    }
    LOG_D("GPS_MODE:  %d", tmpval_u8);  
    
    
    /*4、GPS循环类型 */
    PPItemRead(PP_GPS_CYCLE, &tmpval_u8,  PPItemSize(PP_GPS_CYCLE));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_GPS_CYCLE, &tmpval_u8,  PPItemSize(PP_GPS_CYCLE));
    }
    LOG_D("GPS_CYCLE:  %d", tmpval_u8);  
    
    /*5、GPS间隔时间 */
    PPItemRead(PP_GPS_PERIOD, &tmpval_u8,  PPItemSize(PP_GPS_PERIOD));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_GPS_PERIOD, &tmpval_u8,  PPItemSize(PP_GPS_PERIOD));
    }
    LOG_D("GPS_PERIOD:  %d", tmpval_u8);  
    
    /*6、GYR模式 */
    PPItemRead(PP_GYR_MODE, &tmpval_u8,  PPItemSize(PP_GYR_MODE));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_GYR_MODE, &tmpval_u8,  PPItemSize(PP_GYR_MODE));
    }
    LOG_D("GYR_MODE:  %d", tmpval_u8);  
    
    /*7、GYR间隔时间 */
    PPItemRead(PP_GYR_PERIOD, &tmpval_u8,  PPItemSize(PP_GYR_PERIOD));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_GYR_PERIOD, &tmpval_u8,  PPItemSize(PP_GYR_PERIOD));
    }
    LOG_D("GYR_PERIOD:  %d", tmpval_u8);  
   
    /*8、电子围栏 开关 */
    PPItemRead(PP_ELEFEN_OFFON, &tmpval_u8,  PPItemSize(PP_ELEFEN_OFFON));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_ELEFEN_OFFON, &tmpval_u8,  PPItemSize(PP_ELEFEN_OFFON));
    }
    LOG_D("ELEFEN_OFFON:  %d", tmpval_u8);  
   

    /*9、电子围栏 范围 阈值 */
    PPItemRead(PP_ELEFEN_DISTANCE, &tmpval_u8,  PPItemSize(PP_ELEFEN_DISTANCE));
    if(tmpval_u8 == 0 || tmpval_u8 == 1){
    
    }else{
        tmpval_u8=0;
        PPItemWrite(PP_ELEFEN_DISTANCE, &tmpval_u8,  PPItemSize(PP_ELEFEN_DISTANCE));
    }
    LOG_D("ELEFEN_OFFON:  %d", tmpval_u8);  
        
}

/*
********************************************************************************
Function      : sys_rtc_sync
Description   :   
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
static void sys_rtc_sync(void)
{
 
}
/*
********************************************************************************
*Function    : sys_set_time
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
#ifdef RT_USING_RTC
int sys_set_time(int year, int mon,int day,int hour,int min,int sec)
{
    if(g_sys_time == 1){ 
        return -1;
    }
    LOG_D("%d%d%d-%d%d%d\n",year,  mon, day, hour, min, sec);
    if (year > 2099 || year < 2000){
        LOG_E("year is out of range [2000-2099]\n");
        return -1;
    }
    if (mon == 0 || mon > 12){
        LOG_E("month is out of range [1-12]\n");
        return -1;
    }
    if (day == 0 || day > 31){
        LOG_E("day is out of range [1-31]\n");
        return -1;
    }
    if (hour > 23){
        LOG_E("hour is out of range [0-23]\n");
        return -1;
    }
    if (min > 59){
        LOG_E("minute is out of range [0-59]\n");
        return -1;
    }
    if (sec > 59){
        LOG_E("second is out of range [0-59]\n");
        return -1;
    }
   
    set_date(year, mon, day);
    set_time(hour, min, sec);
    g_sys_time = 1;
    
    PITRegTimer(&rtc_timer,10*60*60*24,sys_rtc_sync);
    return 1;  
}
#endif