/*
 *  bg77_gnss.c
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
#include "app_lib.h"  
#include "at_client.h"
#include "bg77.h"
#include "bg77_cmd.h"


#define DBG_TAG "bg77"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/

extern at_manage_t bg77;
/*
********************************************************************************
********************************************************************************
*/


/*
********************************************************************************
*Function    : bg77_file_size
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int bg77_file_size(const char *file)
{
    int  fd;
    int ulen; 

    fd = open( file , O_RDONLY);
    if( fd == -1 ){ 
        LOG_E("Unable to find file %s ",file);
        return 0;
    }
    lseek(fd, 0, SEEK_CUR);
    ulen = lseek(fd, 0, SEEK_END);
    
    close(fd);
    
    return ulen;
}
/*
********************************************************************************
*Function    : bg77_cmd_line_with_len
*Description :    
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int bg77_cmd_line_with_len(const char *atcmd, int atcmdlen,const char *ack,
                      char *xbuf, int xbuf_size,unsigned int timeout_ms, int det_err)
{

    return at_cmd_line(bg77.client, (unsigned char*)atcmd, atcmdlen, (unsigned char *)ack,(unsigned char *)xbuf, xbuf_size, timeout_ms, det_err);
}
/*
********************************************************************************
*Function    : bg77_cmd_line
*Description :    
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int bg77_cmd_line( const char *atcmd,const char *ack,char *xbuf, int xbuf_size,unsigned int timeout_ms, int det_err)
{
    return bg77_cmd_line_with_len(atcmd, 0, ack, xbuf, xbuf_size, timeout_ms, det_err);
}
/*
********************************************************************************
*Function    : bg77_bg77_cmd_line_get
*Description :    
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int bg77_cmd_line_get(char *suffix,char *xbuf, int xbuf_size, unsigned int timeout_ms)
{
    return at_cmd_line_get(bg77.client, suffix, (unsigned char *)xbuf, xbuf_size, timeout_ms);
}
/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
*Function    : bg77_at_resp_detect
*Description : 检测是否回应AT指令
*Input       :
*Output      :
*Return      :  0   -- AT指令回应正常
               -1   -- AT指令响应异常
*Others      :
********************************************************************************
*/
int bg77_at_resp_detect(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (bg77_cmd_line("AT\r", "OK", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {
            return 0;
        }
    }
    return -1;
}

/*
********************************************************************************
*Function    : bg77_imei_detect
*Description : 检测 BG77 IMEI
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_imei_detect(void)
{
    char *str, *ptr;
    char tmpbuf[40] = {'\r', '\n', 'O', 'K', '\0'};

    if (bg77_cmd_line("AT+GSN\r", "\r\n", tmpbuf, sizeof(tmpbuf), 2000, 1) == AT_CMD_DATA_GET_OK) {
        str = tmpbuf;

        //skip left white
        while (at_isspace(*str)) str++;

        //skip right white
        ptr = str + strlen(str);
        while (ptr > str) {
            --ptr;
            if (at_isspace(*ptr)) {
                *ptr = '\0';
            } else {
                break;
            }
        }

        //copy data
        strncpy(bg77.imei, str, sizeof(bg77.imei)-1);

        return 0;
    }

    return -1;
}
/*
********************************************************************************
*Function    : bg77_model_detect
*Description : 检测 BG77 Model ID
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_model_detect(void)
{
    char *str, *ptr;
    char tmpbuf[40] = {'\r', '\n', 'O', 'K', '\0'};

    if (bg77_cmd_line("AT+GMM\r", "\r\n", tmpbuf, sizeof(tmpbuf), 2000, 1) == AT_CMD_DATA_GET_OK) {
        str = tmpbuf;

        //skip left white
        while (at_isspace(*str)) str++;

        //skip right white
        ptr = str + strlen(str);
        while (ptr > str) {
            --ptr;
            if (at_isspace(*ptr)) {
                *ptr = '\0';
            } else {
                break;
            }
        }

        //copy data
        strncpy(bg77.model, str, sizeof(bg77.model)-1);

        return 0;
    }

    return -1;
}
/*
********************************************************************************
*Function    : bg77_detect
*Description : 检测BG77 ICCID
*Input       :
*Output      :
*Return      : 0   -- 识别到SIM卡
               -1  -- 未识别到SIM卡
*Others      :
********************************************************************************
*/
int bg77_iccid_detect(void)
{
    char *str, *ptr;
    char tmpbuf[40] = {'\r', '\n', 'O', 'K', '\0'};

    if (bg77_cmd_line("AT+QCCID\r", "+QCCID:", tmpbuf, sizeof(tmpbuf), 3000, 1) == AT_CMD_DATA_GET_OK) {
        str = tmpbuf;

        //skip left white
        while (at_isspace(*str)) str++;

        //skip right white
        ptr = str + strlen(str);
        while (ptr > str) {
            --ptr;
            if (at_isspace(*ptr)) {
                *ptr = '\0';
            } else {
                break;
            }
        }

        //copy data
        strncpy(bg77.iccid, str, sizeof(bg77.iccid)-1);

        return 0;
    }

    return -1;
}
/*
********************************************************************************
*Function    : bg77_sim_card_detect
*Description : 检测BG77 是否识别到SIM卡
*Input       :
*Output      :
*Return      : 0   -- 识别到SIM卡
               -1  -- 未识别到SIM卡
*Others      :
********************************************************************************
*/
int bg77_sim_card_detect(void)
{
    int res = 0;

    if (bg77_iccid_detect() != 0) {
        res = -1;
    }

    bg77_imei_detect();

    bg77_model_detect();

    return res;
}

/*
********************************************************************************
*Function    : bg77_csq_detect
*Description :
*Input       :
*Output      :
*Return      :  0 -- detect OK, -1 -- Not known or not detectable
*Others      : AT+CSQ\r
               +CSQ: 29,99
********************************************************************************
*/
int bg77_csq_detect(void)
{
    char *ptr;
    int rssi;
    char tmpbuf[30];

    at_strncpy(tmpbuf, "\r\nOK", sizeof(tmpbuf));
    tmpbuf[sizeof(tmpbuf)-1] = '\0';

    if (bg77_cmd_line("AT+CSQ\r", "+CSQ:", tmpbuf, sizeof(tmpbuf), 2000, 0) == AT_CMD_DATA_GET_OK) {
        ptr = strchr(tmpbuf, ',');
        if (ptr) {
            *ptr = '\0';
            rssi = at_atoi(tmpbuf);
            bg77.rssi = rssi;
            if (rssi != 99) {
                return 0;
            }
            
        }
    }
    return -1;
}
/*
********************************************************************************
*Function    : bg77_ps_attach_detect
*Description :
*Input       :
*Output      :
*Return      : 0 if attach, -1 if detach
*Others      :
********************************************************************************
*/
int bg77_ps_attach_detect(void)
{
    int attach;
    char tmpbuf[30];

    at_strncpy(tmpbuf, "\r\nOK", sizeof(tmpbuf));
    tmpbuf[sizeof(tmpbuf)-1] = '\0';

    if (bg77_cmd_line("AT+CGATT?\r", "+CGATT:", tmpbuf, sizeof(tmpbuf), 2000, 0) == AT_CMD_DATA_GET_OK) {
        attach = at_atoi(tmpbuf);
        if (attach == 1) {
            return 0;
        }
    }
    return -1;
}
/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
Function      : bg77_gnss_on
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void bg77_gnss_on(void)
{
    if (bg77_cmd_line("AT+QGPS=1\r", "\r\nOK:", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {

    }
}

/*
********************************************************************************
Function      : bg77_gnss_off
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void bg77_gnss_off(void)
{
    if (bg77_cmd_line("AT+QGPSEND\r", "\r\nOK:", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {

    }
}
/*
********************************************************************************
*Function    : bg77_detect
*Description : 检测BG77 ICCID
*Input       :
*Output      :
*Return      : 0   -- 识别到SIM卡
               -1  -- 未识别到SIM卡
*Others      :
********************************************************************************
*/
int bg77_gnss_config(int mode)
{
    char atcmd[128];

    if( mode != GPS_GLONASS && mode != GPS_BeiDou && mode != GPS_Galileo && mode != GPS_QZSS ){
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QGPSCFG=\"gnssconfig\",%d\r", mode);
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    return -1;
}
/*
********************************************************************************
Function      : bg77_gnss_detect
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_gnss_detect(void)
{
    char tmpbuf[128];
    char *str=tmpbuf;
    gpsloc_t gnss;
    char *latitude; 
    char *longitude;

    at_strncpy(tmpbuf, "\r\nOK", sizeof(tmpbuf));
    tmpbuf[sizeof(tmpbuf)-1] = '\0';

    if (bg77_cmd_line("AT+QGPSLOC?\r", "+QGPSLOC:", tmpbuf, sizeof(tmpbuf), 2000, 0) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
   
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
    
    return 0;
}

