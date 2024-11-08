/*
 *  business.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include <board.h>                                                     
#include <rtthread.h>                                                  
#include "app_lib.h"                                                 
                                         
#include "app_led.h"
#include "adc_thread.h"


#define DBG_TAG "busi"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>       


/*
********************************************************************************
********************************************************************************
*/
#define PI                      3.1415926
#define EARTH_RADIUS            6378.137        //地球近似半径
 
/*
********************************************************************************
********************************************************************************
*/

#if 0
/* 管理参数 */
static struct {
     /* 实时参数 */
     int systemp;
     int cputemp;


} busi;
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
Function      : gpio_cmd
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static int business_reload(void)
{

    

    return 0;
}
#endif
/*
********************************************************************************
*Function    : gps_set_baudrate
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static double radian(double d)
{
    return d * PI / 180.0;   //角度1? = π / 180
}
/*
********************************************************************************
*Function    : get_distance
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
double get_distance(double base_lati, double base_lng, double pos_lati, double pos_lng)
{
    double radLat1 = radian(base_lati);
    double radLat2 = radian(pos_lati);
    
    double a = base_lati - pos_lati;
    double b = radian(base_lng) - radian(pos_lng);
    
    double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2) )));
    
    dst = dst * EARTH_RADIUS;
    dst= round(dst * 10000) / 10000;
    
    
    return dst;
}
/*
********************************************************************************
Function      : business_process
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void business_process(void)
{ 

}