/*
 *  show.c
 *
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
#include "app_lib.h"
#define DBG_TAG "misc"
#define DBG_LVL DBG_LOG   
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
*Function    : show
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void show_board(void)
{
    u8_t tempbuf[64];
    u8_t value,size;
    
    /*厂家标识*/

    size = PPItemSize(PP_PRD_FACTURER);
    PPItemRead(PP_PRD_FACTURER,tempbuf,size);
    rt_kprintf("PRD FACTURER  : %s\n",tempbuf);
    /*产品序列号 */
    size = PPItemSize(PP_PRD_PSN);
    PPItemRead(PP_PRD_PSN,tempbuf,size);
    rt_kprintf("PRD SN: %s\n",tempbuf);
    /*产品型号 20*/
    size = PPItemSize(PP_PRD_MODEL);
    PPItemRead(PP_PRD_MODEL,tempbuf,size);
    rt_kprintf("PRD MODEL  : %s \n",tempbuf);
    /*主板類別 1*/
    size = PPItemSize(PP_PRD_BOARD);
    PPItemRead(PP_PRD_BOARD,&value,size);
    rt_kprintf("PRD BOARD  : %d \n",value);

}
/*
********************************************************************************
*Function    : show_network
*Description : 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void show_network(void)
{
    u8_t tempbuf[128];
    u16_t value;
    
    /*MQT HOST*/
    PPItemRead(PP_MQT_URL,tempbuf,PPItemSize(PP_MQT_URL));
    rt_kprintf("MQT HOST  : %s \n",tempbuf);
    
    /*MQT PORT*/
    PPItemRead(PP_MQT_PORT,&value,PPItemSize(PP_MQT_PORT));
    rt_kprintf("MQT PORT  : %d \n",value);
    
    /*MQT TICK*/
    PPItemRead(PP_MQT_TICK,&value,PPItemSize(PP_MQT_TICK));
    rt_kprintf("MQT TICK  : %d \n",value);
    
    
    /*网卡  蜂窝BG77*/
    rt_kprintf("model  : BG77 \n");
    
    rt_kprintf("RSSI  : %d \n",0);
    rt_kprintf("IMEI  : %s \n","0");
    PPItemRead(PP_ESIM_CCID0,tempbuf,PPItemSize(PP_ESIM_CCID0));
    rt_kprintf("ICCID0  : %s \n",tempbuf);
    PPItemRead(PP_ESIM_CCID1,tempbuf,PPItemSize(PP_ESIM_CCID1));
    rt_kprintf("ICCID1  : %s \n",tempbuf);
    PPItemRead(PP_ESIM_CCID2,tempbuf,PPItemSize(PP_ESIM_CCID2));
    rt_kprintf("ICCID2  : %s \n",tempbuf);
    
    
    /*网卡  WIFI*/
    PPItemRead(PP_WIFI_NAME,tempbuf,PPItemSize(PP_WIFI_NAME));
    rt_kprintf("WIFI_NAME  : %s \n",tempbuf);
    
    PPItemRead(PP_WIFI_PWD,tempbuf,PPItemSize(PP_WIFI_PWD));
    rt_kprintf("WIFI_PWD  : %s \n",tempbuf);
    
    /*网卡  BLE*/
    PPItemRead(PP_BLE_NAME,tempbuf,PPItemSize(PP_BLE_NAME));
    rt_kprintf("BLE_NAME  : %s \n",tempbuf);
    
    PPItemRead(PP_BLE_PIN,tempbuf,PPItemSize(PP_BLE_PIN));
    rt_kprintf("BLE_PIN  : %s \n",tempbuf);
}
/*
********************************************************************************
*Function    : show_system
*Description : 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/ 
static void show_system(void)
{
    u8_t tempbuf[64];
    u16_t value;
    
    /* 飞行模式 */
    PPItemRead(PP_AIRPLANE_MODE,&value,PPItemSize(PP_AIRPLANE_MODE));
    rt_kprintf("AIRPLANE_MODE  : %d \n",value);
    
    /* 工作模式 */
    PPItemRead(PP_WORK_MODE,&value,PPItemSize(PP_WORK_MODE));
    rt_kprintf("WORK_MODE  : %d \n",value);
    
    
    /* GPS运行模式 */
    PPItemRead(PP_GPS_MODE,&value,PPItemSize(PP_GPS_MODE));
    rt_kprintf("GPS_MODE  : %d \n",value);
    
    /* GPS循环类型 */
    PPItemRead(PP_GPS_CYCLE,&value,PPItemSize(PP_GPS_CYCLE));
    rt_kprintf("GPS_CYCLE  : %d \n",value);
    
    /* GPS间隔时间 */
    PPItemRead(PP_GPS_PERIOD,&value,PPItemSize(PP_GPS_PERIOD));
    rt_kprintf("GPS_PERIOD  : %d \n",value);
    
    
    /* GYR模式 */
    PPItemRead(PP_GYR_MODE,&value,PPItemSize(PP_GYR_MODE));
    rt_kprintf("GYR_MODE  : %d \n",value);
    /* GYR间隔时间 */
    PPItemRead(PP_GYR_PERIOD,&value,PPItemSize(PP_GYR_PERIOD));
    rt_kprintf("GYR_PERIOD  : %d \n",value);
    
    
    /* 电子围栏 开关 */
    PPItemRead(PP_ELEFEN_OFFON,&value,PPItemSize(PP_ELEFEN_OFFON));
    rt_kprintf("ELEFEN_OFFON  : %d \n",value);
    
    /* 电子围栏 范围 阈值*/
    PPItemRead(PP_ELEFEN_DISTANCE,&value,PPItemSize(PP_ELEFEN_DISTANCE));
    rt_kprintf("ELEFEN_DISTANCE  : %d \n",value);
    
    /* 电子围栏 经度*/
   // PPItemRead(PP_ELEFEN_LONG,&value,PPItemSize(PP_ELEFEN_LONG));
   // rt_kprintf("ELEFEN_LONG  : %d \n",value);
    
    /* 电子围栏 维度*/
  //  PPItemRead(PP_ELEFEN_LATI,&value,PPItemSize(PP_ELEFEN_LATI));
  //  rt_kprintf("ELEFEN_LATI  : %d \n",value);
    
    
}
/*
********************************************************************************
*Function    : show_app
*Description : 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void show_all(void)
{
     rt_kprintf("===========================\n");
     show_board(); 
     rt_kprintf("===========================\n");
     show_network(); 
     rt_kprintf("===========================\n");
     show_system();
     rt_kprintf("===========================\n");
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
    rt_kprintf("Please input '<help>' \n"); 
}
/*
********************************************************************************
*Function    : show
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void show(int argc, char** argv)
{
    if (argc < 2 ) {
        show_usage();
        return;
    }
  
    if(strcmp(argv[1], "help") == 0) {
        show_usage();
    }else if(strcmp(argv[1], "all") == 0) {    
        show_all();
    }else{
        show_usage();
    }
} 
MSH_CMD_EXPORT(show,Display system information <help|all|...> );