/*
 *  colle_thread.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include <stdio.h>
#include <string.h>
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <drivers/pin.h>

#include "type.h"
#include "Queue16.h"
#include "ustring.h"
#include "app_lib.h"

#include "board.h"



/*
********************************************************************************
********************************************************************************
*/
#define DBG_TAG "adc"
#define DBG_LVL DBG_LOG   //
#include <rtdbg.h>



// #define REFER_VOLTAGE       3300         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define REFER_VOLTAGE       1800         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 12)   /* 转换位数为12位 */

/*
********************************************************************************
********************************************************************************
*/
#define COLLE_MAX_NUM                4
#define COLLE_NOTE_EVENT           0x01    
 
#define COLLE_THREAD_PRIORITY        10
#define COLLE_THREAD_STACK_SIZE      1024
#define COLLE_THREAD_TIMESLICE       5
/*
********************************************************************************
********************************************************************************
*/
 
static struct rt_thread  g_colle ;
 
static struct rt_event event;                
 

static rt_uint8_t colle_thread_stack[COLLE_THREAD_STACK_SIZE];
/*
********************************************************************************
********************************************************************************
*/
static u16_t CapBuf[COLLE_MAX_NUM][10];

static s32_t g_ColleVaule[COLLE_MAX_NUM];
static s32_t g_ColleAdcVaule[COLLE_MAX_NUM];
static struct _SENSOR_UNIT
{    
    u32_t     Count;  
    u8_t      errcnt;
    sqqueue16 Sql;                                                    
} g_colle_sql[COLLE_MAX_NUM];

static rt_adc_device_t dev;
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
Function      : colle_get_value
Description   : 采集数据过滤处理 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
signed short colle_get_value(int  eid)
{
    int chx=0;
    
    switch(eid){
        case RTSBatVol:     /* 电池电压  */ 
             chx = ADC_CHX_VBAT;
             break;  
        default:
             break;
    }
             
    return g_ColleVaule[chx];
}
/*
********************************************************************************
Function      : colle_get_adc_value
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
signed short colle_get_adc_value(int  eid)
{
    int chx=0;
    
    switch(eid){
        case RTSCPUTemp:    /* 处理器温度 */
             chx = 1;
             break;  
        case RTSBatVol:     /* 电池电压  */ 
             chx = 0;
             break;  
        default:
             break;
    }
             
    return g_ColleAdcVaule[chx];
}
/*
********************************************************************************
Function      : colle_filter
Description   : 采集数据过滤处理 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static u32_t colle_filter(u8_t chx,u16_t rtdata)
{
    u16_t data;
     
 
    while(EnQueue16(&g_colle_sql[chx].Sql,(u16_t )rtdata)==0){
        DeQueue16(&g_colle_sql[chx].Sql,&data);
    }
    if(!IsQueue16(&g_colle_sql[chx].Sql)){
        rtdata = str_averageu16(&CapBuf[chx][0],10);
    }
                    
     return (u32_t)rtdata;
}
/*
********************************************************************************
Function      : colle_noti_event
Description   : 回调函数 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void colle_noti_event(void)
{
    rt_event_send(&event, COLLE_NOTE_EVENT);    
}
/*
********************************************************************************
Function      : colle_data_convert
Description   : 采集数据过滤处理 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static signed short colle_data_convert(s32_t chx,int adcval)
{
    signed int value=0;
    float temperature;
    float scale;
    float vbat;
    float R1 = 470;
    float R2 = 1220;
    scale=R1/R2;
    // LOG_D("scaleis :%f ", scale);
    switch(chx){
        case 9:  
          {
               value = adcval * REFER_VOLTAGE / CONVERT_BITS;
               LOG_D("channel=%d the voltage is :%d ", chx,value);
               vbat = value/scale;
               LOG_D("VBAT voltage is :%f ", vbat);
          }
          break;
        case RT_ADC_INTERN_CH_TEMPER:   
          {  
               // temperature = (1.43 - 3.3/4095 * adcval)/0.0043 + 25;
               // temperature = (1.43 - 1.8/4096 * adcval)/0.0043 + 25;
                  temperature = 100/1347 *(adcval-4117)+ 30;
               LOG_D("channel=TEMPER  is :%f ", temperature);
          }
          break;
        case RT_ADC_INTERN_CH_VREF:  
          {
               value = adcval * REFER_VOLTAGE / CONVERT_BITS;
               LOG_D("channel=CH_VREF the voltage is :%d", value);
          }
          break;
        case RT_ADC_INTERN_CH_VBAT:   
          {
               value =4* adcval * REFER_VOLTAGE / CONVERT_BITS;
               LOG_D("channel=STM32_VBAT the voltage is :%d ", value);
          }
          break;
        default :
          break; 
    } 
    LOG_D("ch[%d] adcval=%d  \n\n",chx,adcval);
    return value;
}
/*
********************************************************************************
Function      : colle_thread
Description   : 数据采集线程 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static void colle_thread(void *pdata)
{
    rt_err_t ret;
    unsigned int voutadc;
    unsigned int averdata;
    s32_t channel[4]={9,RT_ADC_INTERN_CH_TEMPER,RT_ADC_INTERN_CH_VREF,RT_ADC_INTERN_CH_VBAT};
    u8_t index;
    
    while(1){

        index = 0;

        do{
            ret = rt_adc_enable(dev, channel[index]);//
            if(ret<0){
                LOG_E("rt_adc_enable adc1 failed!");
            }
            voutadc = rt_adc_read(dev, channel[index]);
            LOG_D("rt_adc_read value is :%d ", voutadc);
      
            // averdata = colle_filter(index,voutadc);  
            // LOG_D("averdata value is :%d ", averdata);
            
            // g_ColleAdcVaule[index] = averdata;
            /*数据计算根据ADC值算出最后的温度 电压等值*/
            // g_ColleVaule[index] = colle_data_convert( channel[index], averdata);  
             g_ColleVaule[index] = colle_data_convert( channel[index], voutadc);  

            ret = rt_adc_disable(dev, channel[index]);
            if(ret<0){
                LOG_E("rt_adc_disable adc1 failed!");
            }
            index++; 
            if(index>3){
              index = 0;
              break;
            }
            // if( ++index == ARRAYNUM(channel)){
            //      LOG_E("ARRAYNUM");
            //      break;
            // } 
        }while(index);
            
        rt_thread_mdelay(2000);

    }
}
/*
#define RT_ADC_INTERN_CH_TEMPER     (-1)
#define RT_ADC_INTERN_CH_VREF       (-2)
#define RT_ADC_INTERN_CH_VBAT       (-3)
********************************************************************************
Function      : colle_thread_init
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int colle_adc_open(void)
{

    dev = (rt_adc_device_t)rt_device_find("adc1");
    if(RT_NULL==dev){
        LOG_E("rt_device_find adc1 failed!");
        return RT_ERROR;
    }
    

    return 0;
}
/*
********************************************************************************
Function      : colle_thread_init
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int colle_thread_init(void)
{ 
    rt_err_t result;
    
    colle_adc_open();
    
/*    for(u32_t chn=0;chn<COLLE_MAX_NUM;chn++){
        InitQueue16(&g_colle_sql[chn].Sql, &CapBuf[chn][0],10); 
    } */
    
    result = rt_event_init(&event, "event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK){
        LOG_E("init static event failed.");
        return -1;
    } 
    result = rt_thread_init(&g_colle,
                            "coll",
                            colle_thread, 
                            (void*)1, 
                            &colle_thread_stack[0], 
                            COLLE_THREAD_STACK_SIZE, 
                            COLLE_THREAD_PRIORITY, 
                            COLLE_THREAD_TIMESLICE);
    if (result == RT_EOK){
        rt_thread_startup(&g_colle);
    }else{
        return -1;
    }
    return 0;
}
INIT_COMPONENT_EXPORT(colle_thread_init);
