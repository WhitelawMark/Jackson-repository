
/*
 * lan_port.c
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
Function      : gps_cacl
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static void gps_cacl(void)
{
    int length;
    double lat1 = 39.90744;
    double lng1 = 116.41615;//经度,纬度1
    double lat2 = 39.90744;
    double lng2 = 116.30746;//经度,纬度2
    // insert code here...

    double dst = get_distance(lat1, lng1, lat2, lng2);
    length = (int )dst*1000;
    LOG_E("dst = %dkm\n", length);  //dst = 9.281km
}

/*
********************************************************************************
Function      : show_gnss
Description   :  
Input         :
Output        :
Return        :
Others        :+QGPSLOC: <UTC>,<latitude>,<longitude>,<HDOP>,<altitude>,<fix>,<COG>,<spkm>,<spkn>,<date>,<nsat>
********************************************************************************

 latitude :2432.2185N 
 longitude:11756.4691E 
 UTC      :31245 
 HDOP     :1 
 altitude :-16 
 fix      :3 
 COG      :0 
 spkm     :0 
 spkn     :0 
 date     :250723 
 nsat     :3 
*/
static void show_gnss(void)
{
    char tmpbuf[128]="+QGPSLOC: 031245.000,2432.2185N,11756.4691E,1.4,-16.4,3,0.00,0.0,0.0,250723,03";
    char *str=tmpbuf;
    gpsloc_t gnss;
    char *latitude; 
    char *longitude;
    
    while (*str && *str != ':') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.UTC = at_atoi(str);
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    latitude = str ;
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    longitude = str ;
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.HDOP = at_atoi(str);
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.altitude = at_atoi(str);
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.fix = at_atoi(str);
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.COG = at_atoi(str);
   
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.spkm = at_atoi(str);
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.spkn = at_atoi(str);
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.date = at_atoi(str);
    
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    gnss.nsat = at_atoi(str);
    
    strcpy(gnss.latitude,latitude);
    strcpy(gnss.longitude,longitude);
    
    rt_kprintf(" latitude :%s \n", latitude );
    rt_kprintf(" longitude:%s \n", longitude);
    rt_kprintf(" UTC      :%d \n", gnss.UTC      );
    rt_kprintf(" HDOP     :%d \n", gnss.HDOP     );
    rt_kprintf(" altitude :%d \n", gnss.altitude );
    rt_kprintf(" fix      :%d \n", gnss.fix      );
    rt_kprintf(" COG      :%d \n", gnss.COG      );
    rt_kprintf(" spkm     :%d \n", gnss.spkm     );
    rt_kprintf(" spkn     :%d \n", gnss.spkn     );
    rt_kprintf(" date     :%d \n", gnss.date     );
    rt_kprintf(" nsat     :%d \n", gnss.nsat     );

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
static void show_float(void)
{   
    double lat1 = 39.90744;
     
    double lat2 = 0;
    
    PPItemWrite(PP_ELEFEN_LONG, &lat1,  PPItemSize(PP_ELEFEN_LONG));
 
    PPItemRead(PP_ELEFEN_LONG, &lat2,  PPItemSize(PP_ELEFEN_LONG));
    
    
    rt_kprintf("lat1 = %lf lat2 = %lf \n",lat1, lat2);
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
    rt_kprintf("Please input '<led >' \n"); 
}
/*<HDOP>,<altitude>,<fix>,<COG>,<spkm>,<spkn>,<date>,<nsat>
********************************************************************************
Function      : bg77_cmd
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void bg77_cmd(int argc, char** argv)
{
    if (argc < 2 ) {
        show_usage();
        return;
    }
    if(strcmp(argv[1], "gnss") == 0) {
        if(strcmp(argv[2], "elec") == 0) {
            gps_cacl();
        }else if(strcmp(argv[2], "show") == 0) {
            show_gnss();
        }else if(strcmp(argv[2], "float") == 0) {
            show_float();
        }
    }else if(strcmp(argv[1], "mqtt") == 0) {
        if(strcmp(argv[2], "report") == 0) {
            quectel_mqtt_send("hellword",strlen("hellword"));
        }
    }else if(strcmp(argv[1], "file") == 0) {
        if(strcmp(argv[2], "upload") == 0) {
            quectel_file_upload(0,0);
        }else if(strcmp(argv[2], "download") == 0) {
            quectel_file_download(0,0);
        }
    }else if(strcmp(argv[1], "http") == 0) {
        if(strcmp(argv[2], "upload") == 0) {
            quectel_https_upload(0);
        }else if(strcmp(argv[2], "download") == 0) {
            quectel_https_download(0);
        }
    }else if(strcmp(argv[1], "dwl") == 0) {
      

    }
}
MSH_CMD_EXPORT(bg77_cmd,bg77_cmd);