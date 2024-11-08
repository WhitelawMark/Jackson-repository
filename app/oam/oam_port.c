/*
 * oam_port.c
 *
 *  Created on: 2023年7月17日
 *      Author: lwp
 */
#include <string.h>
#include "app_lib.h"
#include "oam_thread.h"
#include "oam_port.h"
#include "task_timer.h"
#include "app_led.h"
#include "pit.h"
 
#define DBG_TAG "oam"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/

static  struct {
     u8_t  initflg;
     u8_t  connect;
     u32_t runtime; 
     u8_t  airmode;
     u8_t  readyflg;
     u8_t  devtype;
     u16_t heart;
} oam;

struct oam_evt_msg{
     u8_t msgtype;
     u8_t subtype;
     int  arg;
     void (*process)(int,int);
} oam_evt_msg_t;

static PIT_TIMER oam_timer;
static PIT_TIMER ota_timer;
static DECLARE_TASK_TIMER(oam_poll_timer);

/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    : oam_get_le_word
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u16_t oam_get_le_word(u8_t *msg)
{
    return (((((u16_t)(msg[1])) << 8 ) & 0xFF00) | msg[0]);
}
/*
********************************************************************************
*Function    : oam_get_le_dword
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u32_t oam_get_le_dword(u8_t *msg)
{
    return (((((u32_t)(msg[3])) << 24) & 0xFF000000) |
            ((((u32_t)(msg[2])) << 16) & 0x00FF0000) |
            ((((u32_t)(msg[1])) << 8 ) & 0x0000FF00) | msg[0]);
}
 
/*
********************************************************************************
*Function    : lan_set_le_word
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void oam_set_le_word(u8_t *msg, u16_t value)
{
    msg[0] = (u8_t)(value);
    msg[1] = (u8_t)((value) >> 8);
}
/*
********************************************************************************
*Function    : oam_set_le_dword
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void oam_set_le_dword(u8_t *msg, u32_t value)
{
    msg[0] = (u8_t)(value);
    msg[1] = (u8_t)((value) >> 8);
    msg[2] = (u8_t)((value) >> 16);
    msg[3] = (u8_t)((value) >> 24);
} 
/*
********************************************************************************
*Function    : oam_event_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void oam_event_process(struct oam_evt_msg *msg, int len)
{

}   
/*
********************************************************************************
*Function    : oam_evt_trgger
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
rt_err_t oam_evt_trgger(int msgtype,int subtype,int arg, void (*process)(int,int))
{
    struct oam_evt_msg msg;
    
    msg.msgtype = msgtype ;
    
    msg.subtype = subtype ;
    
    msg.arg = arg ;
    
    msg.process = process ;
      
    if (oam_thread_post((void(*)(void *, int))oam_event_process, &msg, sizeof(struct oam_evt_msg)) != 0) {
        LOG_E("oam message post fail");
    }
    return RT_EOK;
} 

/*
********************************************************************************
*Function    : oam_get_runtime
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
u32_t oam_get_runtime(void)
{

    return oam.runtime/10;
} 
/*
********************************************************************************
*Function    : oam_timer_setcb
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void oam_timer_setcb( void (*func)(void *arg))
{
    oam_thread_timer_set(&oam_poll_timer, func, 0);
}
/*
********************************************************************************
*Function    : oam_timer_start
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void oam_timer_start(int timeout)
{
    task_timer_start(&oam_poll_timer, timeout);
}
/*   
********************************************************************************
*Function    : oam_timer_stop
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void oam_timer_stop(void)
{
    task_timer_stop(&oam_poll_timer);
}
/*
********************************************************************************
*Function    : oam_business_process
*Description : 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void oam_business_process(void *arg)
{
//    business_process();
}
/*
********************************************************************************
*Function    : oam_business_tick
*Description : 
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void oam_business_tick(void *arg)
{
    oam.runtime++;
    
    PITTimeTick();
    
    oam_business_process(0);
    
    oam_timer_start(100);
}
/*
********************************************************************************
*Function    : oam_is_connect
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int oam_is_connect(void)
{ 
    return oam.connect;
}
/*
********************************************************************************
*Function    : oam_start_ota
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void oam_start_ota(void)
{ 
    PITRegTimer(&ota_timer,5*10,ota_start);
} 
/*
********************************************************************************
*Function    : oam_heart_report
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void oam_heart_report(void)
{ 
    if( oam.airmode == 0 ){
        smart_report_message(SMART_RPT_HEARTBEAT);
    }

    PIT_TimerSet(&oam_timer,oam.heart*10,oam_heart_report);
}
/*
********************************************************************************
*Function    : oam_business_start
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void oam_business_start(void)
{
   // ota_init();
    


    PIT_TimerSet(&oam_timer, 10*2,oam_heart_report);
}
/*
********************************************************************************
*Function    : oam_parm_load
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void oam_parm_load(void)
{ 
    PPItemRead(PP_AIRPLANE_MODE, &oam.airmode,  PPItemSize(PP_AIRPLANE_MODE));

    PPItemRead(PP_MQT_TICK, &oam.heart,  PPItemSize(PP_MQT_TICK));
}
/*
********************************************************************************
*Function    : oam_start
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/ 
void oam_start(void)
{          
    oam.readyflg = FALSE;
    
    oam.runtime = 0; 
    
    oam.initflg = 0;
    
    oam.connect = 0;

    oam_parm_load();
    
    oam_timer_setcb(oam_business_tick);
    
    oam_timer_start(100);
    
    quectel_mqtt_init();
    
    esp_btser_init();

    PITRegTimer(&oam_timer,30*10,oam_business_start);
}