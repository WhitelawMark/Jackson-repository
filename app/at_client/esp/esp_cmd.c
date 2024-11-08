/*
 *  esp_cmd.c
 *
 *  Created on: 2023年08月24日
 *
 *      Author: lwp
 *      
 *    Document: https://docs.espressif.com/projects/esp-at/zh_CN/release-v3.2.0.0/
 *              esp32c3/AT_Command_Set/Basic_AT_Commands.html
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <drivers/pin.h>
#include "type.h"
#include "ustring.h"
#include "at.h"
#include "at_client.h"
#include "esp.h"

#define DBG_TAG "esp"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
extern esp_at_t esp;

 /*
********************************************************************************
********************************************************************************
*/ 

#define get_version_ptr()   (&esp.vs)    
#define get_lap_ptr()       (&esp.lap)    
#define get_sta_ptr()       (&esp.wifista) 
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : esp_cmd_line_with_len
*Description : 通用at命令接口     
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int esp_cmd_line_with_len(const char *atcmd, int atcmdlen,const char *ack,
                      char *xbuf, int xbuf_size,unsigned int timeout_ms, int det_err)
{

    return at_cmd_line(esp.client, (unsigned char*)atcmd, atcmdlen, (unsigned char *)ack,(unsigned char *)xbuf, xbuf_size, timeout_ms, det_err);
}
/*
********************************************************************************
*Function    : esp_cmd_line
*Description : 通用at命令接口   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int esp_cmd_line( const char *atcmd,const char *ack,char *xbuf, int xbuf_size,unsigned int timeout_ms, int det_err)
{
    return esp_cmd_line_with_len(atcmd, 0, ack, xbuf, xbuf_size, timeout_ms, det_err);
}
/*
********************************************************************************
*Function    : esp_cmd_line_get
*Description : 通用数据回复获取接口  
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int esp_cmd_line_get(char *suffix,char *xbuf, int xbuf_size, unsigned int timeout_ms)
{
    return at_cmd_line_get(esp.client, suffix, (unsigned char *)xbuf, xbuf_size, timeout_ms);
}
/*
********************************************************************************
*Function    : esp_cmd_send
*Description : 通用数据发送接口   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int esp_cmd_send(const char *atcmd)
{
    return at_cmd_send(esp.client, (const unsigned char*)atcmd, 0);
}
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : esp_wakeup_ctrl
*Description : esp 休眠控制
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void esp_wakeup_ctrl(int power)
{

}
/*
********************************************************************************
*Function    : esp_power_ctrl
*Description : esp 电源控制
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void esp_power_ctrl(int flag)
{

}
/*
********************************************************************************
*Function    : esp_reset
*Description : esp 硬件复位
*Input       :
*Output      :
*Return      :
*Others      :  Command: AT+RST
                Reply  : OK
********************************************************************************
*/
void esp_reset(void)
{
    int i;

        if (esp_cmd_line("AT+RST\r\n", "OK", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {
            return ;
        }
    return ;
}
/*
********************************************************************************
*Function    : esp_sleep
*Description : esp 休眠
*Input       :
*Output      :
*Return      :
*Others      : Command:AT+SLEEP=<sleep mode>
               Reply  : OK
********************************************************************************
*/
void esp_sleep(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (esp_cmd_line("AT+SLEEP=1\r\n", "OK", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {
            return ;
        }
    }
    return ;
}
/*
********************************************************************************
*Function    : esp_wakeup
*Description : esp 唤醒
*Input       :
*Output      :
*Return      :
*Others      : Command:
********************************************************************************
*/
void esp_wakeup(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (esp_cmd_line("AT+SLEEP=0\r\n", "OK", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {
            return ;
        }
    }
    return ;
}
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : esp_restart
*Description : esp 重启指令
*Input       :
*Output      :
*Return      :
*Others      : Command: AT+RST
               Reply:   OK
********************************************************************************
*/
void esp_restart(void)
{
    int i;

        if (esp_cmd_line("AT+RST\r\n", "OK", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {
            return ;
        }else{
            if (esp_cmd_line("+++", "ready", 0, 0, 3000, 1) == AT_CMD_DATA_GET_OK) {
    
            }
        }
    return ;
}
/*
********************************************************************************
*Function    : esp_at_resp_detect
*Description : 检测是否回应AT指令
*Input       :
*Output      :
*Return      : 0   -- AT指令回应正常 -1   -- AT指令响应异常
*Others      : Command:AT
               Reply:  OK
********************************************************************************
*/
int esp_at_resp_detect(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (esp_cmd_line("AT\r\n", "OK", 0, 0, 2000, 0) == AT_CMD_ACK_OK) {
            return 0;
        }
    }
    return -1;
}
/*
********************************************************************************
*Function    : esp_deep_sleep
*Description : 设置为 Deep-sleep 模式 
*Input       : time 单位秒
*Output      :
*Return      : 0   -- 配置初始化成功  -1  -- 配置初始化失败
*Others      :  Command: AT+GSLP=3600000 
                Reply  : OK   
*Note        :
               1、设定时间到后，设备自动唤醒，调用深度睡眠唤醒桩，然后加载应用程序。
               2、对于 Deep-sleep 模式，唯一的唤醒方法是定时唤醒。
********************************************************************************
*/
int esp_deep_sleep(int time)
{
    char cmdbuf[128] = {'\r', '\n', 'O', 'K', '\0'}; 

    sprintf(cmdbuf,"AT+GSLP=%d\r\n",time);
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_ACK_OK) {
        return 0;
    }
    LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
    return -1;
}
/*
********************************************************************************
*Function    : esp_cwmode_config
*Description : 设置 ESP32-C2 设备的 Wi-Fi 模式
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功  -1  -- 配置初始化失败
*Others      : AT+CWMODE=<mode>[,<auto_connect>]
               <mode>：模式
                      0: 无 Wi-Fi 模式，并且关闭 Wi-Fi RF
                      1: Station 模式
                      2: SoftAP 模式
                      3: SoftAP+Station 模式
               <auto_connect>：切换 ESP32-C3 设备的 Wi-Fi 模式时（例如，从 SoftAP 或无 Wi-Fi 模式切换为 Station 模式或 SoftAP+Station 模式），
                               是否启用自动连接 AP 的功能，默认值：1。参数缺省时，使用默认值，也就是能自动连接。
                      0: 禁用自动连接 AP 的功能
                      1: 启用自动连接 AP 的功能，若之前已经将自动连接 AP 的配置保存到 flash 中，则 ESP32-C3 设备将自动连接 AP
********************************************************************************
*/
int esp_cwmode_config(int cwmode)
{
    char cmdbuf[128] = {'\r', '\n', 'O', 'K', '\0'}; 

    sprintf(cmdbuf,"AT+CWMODE=%d\r\n",cwmode);
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 3000, 1) == AT_CMD_ACK_OK) {
        return 0;
    }
    LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
    return -1;
}
/*
********************************************************************************
*Function    : esp_cwap_config
*Description : 设置 ESP32-C3 Station 需连接的 AP
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功   -1  -- 配置初始化失败
*Others      : Command: AT+CWJAP=[<ssid>],[<pwd>][,<bssid>][,<pci_en>][,<reconn_interval>][,<listen_interval>][,<scan_mode>][,<jap_timeout>][,<pmf>]
               Reply:   WIFI CONNECTED
                        WIFI GOT IP

                        OK
********************************************************************************
*/
int esp_cwap_config(char *ssid ,char *password)
{
    char cmdbuf[128] = {'\r', '\n', 'O', 'K', '\0'}; 
  
    sprintf(cmdbuf,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,password);
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 20000, 1) == AT_CMD_ACK_OK) {
        return 0;
    }
    LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
    return -1;
}
/*
********************************************************************************
*Function    : esp_cwjap_config
*Description : 保持上次链接方式 
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功 -1  -- 配置初始化失败
*Others      : Command:  AT+CWJAP
               Reply:    OK
********************************************************************************
*/
int esp_cwjap_config(void)
{
    if (esp_cmd_line("AT+CWJAP\r\n", "OK", 0, 0, 3000, 1) == AT_CMD_ACK_OK) {
        return 0;
    }
    LOG_E("Command: \"ERROR\" execute fail"); 
    return -1;
}
/*
********************************************************************************
*Function    : esp_rfpower_config
*Description : 发射功率设置
*Input       :
*Output      :
*Return      :
*Others      : Command: AT+RFPOWER=<wifi_power>
               Reply:   OK
********************************************************************************
*/
int esp_rfpower_config(int power)
{      
    char cmdbuf[128] = {'\r', '\n', 'O', 'K', '\0'}; 
  
    sprintf(cmdbuf,"AT+RFPOWER=%d\r\n",power);
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 20000, 1) == AT_CMD_ACK_OK) {
        return 0;
    }
    LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
    return -1;
}
/*
********************************************************************************
*Function    : esp_gsm_detect
*Description : 查看版本信息
*Input       :
*Output      :
*Return      :
*Others      : Command: AT+GMR
               Reply: AT version:3.2.0.0-dev(s-7c866c9 - ESP32C2 - Jul 21 2023 09:29:11)
                      SDK version:v5.0.2-376-g24b9d38a24-dirty
                      compile time(451007ec):Aug 16 2023 11:28:29
                      Bin version:3.1.0(ESP32C2-4MB)
********************************************************************************
*/
int esp_gsm_detect(void)
{      
    version_t *vs= get_version_ptr();
    char tmpbuf[256] = {'\r', '\n', 'O', 'K', '\0'};
    char *str=tmpbuf;
    char *AT_version; 
    char *SDK_version; 
    char *Compile_time; 
    char *Bin_version; 
    
   
    if (esp_cmd_line("AT+GMR\r\n", "AT version:", tmpbuf, sizeof(tmpbuf), 3000, 1) != AT_CMD_DATA_GET_OK) {
         LOG_E("Command: \"ERROR\" execute fail"); 
         return -1;
    }
    // AT_version
    AT_version = str;
    while (*str && *str != '\r') str++;
    if (*str) *str++ = '\0'; //skip ','
    
    // SDK_version
    SDK_version = strstr(str,"SDK version:")+strlen("SDK version:");
    while (*str && *str != '\r') str++;
    if (*str) *str++ = '\0'; //skip ','
    // Compile_time
    Compile_time = strstr(str,"compile time")+strlen("compile time");
    while (*str && *str != '\r') str++;
    if (*str) *str++ = '\0'; //skip ','
    
    Bin_version = strstr(str,"Bin version:")+strlen("Bin version:");
    while (*str && *str != '\r') str++;
    if (*str) *str++ = '\0'; //skip ','
    
    strcpy(vs->at_version,AT_version);
    strcpy(vs->sdk_version,SDK_version);
    strcpy(vs->compile_time,Compile_time);
    strcpy(vs->bin_version,Bin_version);
   
    return 0;
}
/*
********************************************************************************
*Function    : esp_cwlap_detect
*Description : 查询WIFI信息
*Input       :
*Output      :
*Return      :[+CWLAP:<ecn>,<ssid>,<rssi>,<mac>,<channel>,<freq_offset>,<freqcal_val>,<pairwise_cipher>,<group_cipher>,<bgn>,<wps> OK]
*Others      : Command: AT+CWLAP=lwp
               Reply:   +CWLAP:(3,"lwp",-25,"06:c1:56:72:ef:35",6,-1,-1,4,4,7,0)
                        OK
********************************************************************************
*/
int esp_cwlap_detect(char *ssid)
{      
    cwlap_t *lap= get_lap_ptr();
    char atcmd[32];
    char tmpbuf[256] = {'\r', '\n', 'O', 'K', '\0'};
    char *str=tmpbuf;
    char *mac;
    
    if( ssid == NULL ){
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+CWLAP=\"%s\"\r\n", ssid);
    atcmd[sizeof(atcmd)-1] = '\0';

    if (esp_cmd_line(atcmd, "+CWLAP:", tmpbuf, sizeof(tmpbuf), 3000, 1) != AT_CMD_DATA_GET_OK) {
        LOG_E("Command: \"ERROR\" execute fail"); 
        return -1;
    }
    
    //<ecn>
    while (*str && *str != '(') str++;
    if (*str) *str++ = '\0'; //skip ','
    lap->ecn = at_atoi(str);
    //<ssid>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    //rssid = str+1;
    
    //<rssi>
    while (*str && *str != ',') {
        if (*str == '\"' ) *str = '\0'; //skip ',
        str++;
    }
    if (*str) *str++ = '\0'; //skip ',
    lap->rssi = at_atoi(str);
    
    //<mac>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    mac = str+1;
    while (*str && *str != ',') {
        if (*str == '\"' ) *str = '\0'; //skip ',
        str++;
    }
    strcmp(lap->mac,mac);
    //<channel>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    lap->channel = at_atoi(str);

    //<freq_offset>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    lap->freq_offset = at_atoi(str);
    
    //<freqcal_val>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    lap->freqcal_val = at_atoi(str);
    
    //<pairwise_cipher>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    lap->pairwise_cipher = at_atoi(str);
    
    //<group_cipher>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    lap->group_cipher = at_atoi(str);
    
    //<bgn>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    lap->bgn = at_atoi(str);
    
    //<wps>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    lap->wps = at_atoi(str);

    return 0;
}
/*
********************************************************************************
*Function    : esp_sysram_detect
*Description : 查询当前剩余堆空间和最小堆空间
*Input       :
*Output      :
*Return      :
*Others      : Command:AT+SYSRAM?
               Reply:  +SYSRAM:<remaining RAM size>,<minimum heap size>
                       OK
********************************************************************************
*/
int esp_sysram_detect(void)
{  
    char tmpbuf[64] = {'\r', '\n', 'O', 'K', '\0'};
    char *str=tmpbuf;
    int  ram_size,heap_size;
 
    if (esp_cmd_line("AT+SYSRAM?\r\n", "+SYSRAM:", tmpbuf, sizeof(tmpbuf), 3000, 1) != AT_CMD_DATA_GET_OK) {
         LOG_E("Command: \"ERROR\" execute fail"); 
         return -1;
    }
    sscanf(str,"%d,%d",&ram_size,&heap_size);
    
    esp.sys.ram_size = ram_size;
    esp.sys.heap_size = heap_size;
 
    return 0;
}
/*
********************************************************************************
*Function    : esp_sysutemp_config
*Description : 设置时间戳
*Input       :
*Output      :
*Return      :
*Others      : Command: AT+SYSTIMESTAMP=<Unix_timestamp> 
               Reply:   OK
********************************************************************************
*/
int esp_sysutemp_config(int utempval)
{  
    char cmdbuf[128] = {'\r', '\n', 'O', 'K', '\0'}; 
  
    sprintf(cmdbuf,"AT+SYSTIMESTAMP=%d\r\n",utempval);
    if (esp_cmd_line(cmdbuf, "OK", 0, 0, 20000, 1) == AT_CMD_ACK_OK) {
        return 0;
    }
    LOG_E("Command: \"%s\" execute fail\n",cmdbuf); 
    return -1;
}
/*
********************************************************************************
*Function    : esp_sysutemp_detect
*Description : 查询系统时间戳
*Input       :
*Output      : 
*Return      :
*Others      : Command: AT+SYSTIMESTAMP?
               Reply: +CWSTATE:<state>,<"ssid">
********************************************************************************
*/
int esp_sysutemp_detect(void)
{      
    char tmpbuf[64] = {'\r', '\n', 'O', 'K', '\0'};
    char *str=tmpbuf;
    time_t utemp;
    rt_err_t ret = RT_EOK;
    
    
    if (esp_cmd_line("AT+SYSTIMESTAMP?\r\n", "+SYSTIMESTAMP:", tmpbuf, sizeof(tmpbuf), 3000, 1) != AT_CMD_DATA_GET_OK) {
         LOG_E("Command: \"ERROR\" execute fail"); 
         return -1;
    }

    utemp = at_atoi(str);

    struct tm* local_time = localtime(&utemp);
    
    LOG_D("local_time: %d-%02d-%02d %02d:%02d:%02d\n",
            local_time->tm_year + 1900, // 年份需要加上1900
            local_time->tm_mon + 1,     // 月份需要加上1
            local_time->tm_mday,
            local_time->tm_hour,
            local_time->tm_min,
            local_time->tm_sec
            );
    
    /* 设置日期 */
    ret = set_date(local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday);
    if (ret != RT_EOK){
        LOG_E("set RTC date failed\n");
        return ret;
    }
 
    /* 设置时间 */
    ret = set_time(local_time->tm_hour, local_time->tm_min,  local_time->tm_sec);
    if (ret != RT_EOK){
        LOG_E("set RTC time failed\n");
        return ret;
    }
    return 0;
}
/*
********************************************************************************
*Function    : esp_systemp_detect
*Description : 读取芯片内部摄氏温度值
*Input       :
*Output      : AT+SYSTEMP?
*Return      :
*Others      : Command: AT+SYSTEMP?
               Reply:  +SYSTEMP:<value>
                       OK
********************************************************************************
*/
int esp_systemp_detect(void)
{      
    char tmpbuf[64] = {'\r', '\n', 'O', 'K', '\0'};
    char *str=tmpbuf;
    int temp;
    
    if (esp_cmd_line("AT+SYSTEMP?\r\n", "+SYSTEMP:", tmpbuf, sizeof(tmpbuf), 3000, 1) != AT_CMD_DATA_GET_OK) {
         LOG_E("Command: \"ERROR\" execute fail"); 
         return -1;
    }

    temp = at_atoi(str);
    
    LOG_D("wifi temp [%d] ",temp); 
    return 0;
}
/*
********************************************************************************
*Function    : esp_rfpower_detect
*Description : 发射功率查询
*Input       :
*Output      :
*Return      :
*Others      :command: AT+RFPOWER?
              reply:+RFPOWER:<wifi_power>,<ble_adv_power>,<ble_scan_power>,<ble_conn_power>
                    OK
********************************************************************************
*/
int esp_rfpower_detect(void)
{      
    char tmpbuf[64] = {'\r', '\n', 'O', 'K', '\0'};
    int wifi_power,ble_adv_power,ble_scan_power,ble_conn_power;
    
    if (esp_cmd_line("AT+RFPOWER?\r\n", "+RFPOWER:", tmpbuf, sizeof(tmpbuf), 3000, 1) != AT_CMD_DATA_GET_OK) {
         LOG_E("Command: \"ERROR\" execute fail"); 
         return -1;
    }
    sscanf(tmpbuf,"%d,%d,%d,%d",&wifi_power,&ble_adv_power,&ble_scan_power,&ble_conn_power);
    
    esp.rfpower.wifi_power = wifi_power;
    esp.rfpower.ble_adv_power = ble_adv_power;
    esp.rfpower.ble_scan_power = ble_scan_power;
    esp.rfpower.ble_conn_power = ble_conn_power;

    return 0;
}
/*
********************************************************************************
*Function    : esp_cwstate_detect
*Description : ESP 
*Input       :
*Output      :
*Return      :
*Others      : command: +CWSTATE:<state>,<"ssid">
               reply:   +CWSTATE:2,"lwp"
********************************************************************************
*/
int esp_cwstate_detect(void)
{    
    char tmpbuf[64] = {'\r', '\n', 'O', 'K', '\0'};
    char *str=tmpbuf;
    char *ssid;
    char state;
    
    if (esp_cmd_line("AT+CWSTATE?\r\n", "+CWSTATE:", tmpbuf, sizeof(tmpbuf), 3000, 1) != AT_CMD_DATA_GET_OK) {
         LOG_E("Command: \"ERROR\" execute fail"); 
         return -1;
    }
    // <state>
    state = at_atoi(str);
    
    // <ssid>
    while (*str && *str != ',') str++;
    if (*str) *str++ = '\0'; //skip ','
    ssid = str+1;

    /*skip client_idx*/
    while (*str && *str != ',') {
        if (*str == '\"') *str = '\0'; 
        str++;
    }
    if (*str) *str++ = '\0'; //skip ',
    
    esp.wifista.stat = state;
    
    LOG_D("wifi network status [%d] ssid [%s]",state,ssid); 
    return 0;
}
/*
********************************************************************************
*Function    : esp_cipsta_detect
*Description : 查询 ESP32-C3 Station 的 IP 地址 
*Input       :
*Output      :
*Return      :
*Others      :command: AT+CIPSTA?\r\n
              reply:+CIPSTA:ip:"192.168.227.221"
                    +CIPSTA:gateway:"192.168.227.55"
                    +CIPSTA:netmask:"255.255.255.0"
********************************************************************************
*/
int esp_cipsta_detect(void)
{ 
    char tmpbuf[128] = {'\r', '\n', 'O', 'K', '\0'};
    char *str=tmpbuf;
    int res;
    char *ip;
    char *gateway;
    char *netmask;
    wifi_sta_t *sta=get_sta_ptr();
    
    res = esp_cmd_line("AT+CIPSTA?\r\n", "+CIPSTA", tmpbuf, sizeof(tmpbuf), 8000, 1);
    if ( res != AT_CMD_DATA_GET_OK) {
         LOG_E("Command: \"%s\" execute fail","AT+CIPSTA?"); 
         return -1;
    }
    
    str = strstr(str,"ip:")+strlen("ip:")+1;
    ip = str;
    
    while (*str && *str != '\"') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    str = strstr(str,"gateway:")+strlen("gateway:")+1;
    gateway = str;
    
    while (*str && *str != '\"') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    str = strstr(str,"netmask:")+strlen("netmask:")+1;
    netmask = str;
    
    while (*str && *str != '\"') str++;
    if (*str) *str++ = '\0'; //skip ',
    
    strcpy(sta->ip,ip);
    strcpy(sta->gateway,gateway);
    strcpy(sta->netmask,netmask);
    
    
    LOG_D("wifi network status: "); 
    LOG_D("                    ip:     [%s]",sta->ip); 
    LOG_D("                    gateway:[%s]",sta->gateway); 
    LOG_D("                    netmask:[%s]",sta->netmask); 

    return 0;
}
/*
********************************************************************************
*Function    : esp_sntp_config
*Description : 设置时区和 SNTP 服务器
*Input       :
*Output      :
*Return      : 0   -- 配置初始化成功  -1  -- 配置初始化失败
*Others      : command: AT+CIPSNTPCFG=<enable>,<timezone>[,<SNTP server1>,<SNTP server2>,<SNTP server3>]、
               reply  :  OK
********************************************************************************
*/
int esp_sntp_config(char *url)
{
    char cmdbuf[128] = {0}; 
    char tmpbuf[64] = {'\r', '\n', 'O', 'K', '\0'};
    int i;
    
    if( url == NULL ){
        sprintf(cmdbuf,"AT+CIPSNTPCFG=1,800,\"%s\"\r\n","pool.ntp.org");
    }else{
        sprintf(cmdbuf,"AT+CIPSNTPCFG=1,800,\"%s\"\r\n",url);
    }
    
    for (i = 0; i < 5; i++) {
        if (esp_cmd_line(cmdbuf, "+TIME_UPDATED", 0, 0, 8000, 1) == AT_CMD_ACK_OK) {
            break;
        }
    }
    if( i == 5 ){
        return -1;
    }
    
    if (esp_cmd_line("AT+CIPSNTPTIME?\r\n", "+CIPSNTPTIME:", tmpbuf, sizeof(tmpbuf), 20000, 1) == AT_CMD_DATA_GET_OK) {
     
        return 0;
    } 
 
    esp_sysutemp_detect();
    
    return -1;
}
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    : esp_network_detect
*Description : 获取网络状态 
*Input       :
*Output      :
*Return      :
*Others      : 
********************************************************************************
*/
int esp_ls_network(void)
{
    return esp.wifista.stat;
}
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    : __esp_wifi_sta_init
*Description : wifi 工作在STA模式下的初始化
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
int __esp_wifi_sta_init(void)
{    
    int res=-1;
    
    esp_cwmode_config(ESP_CWMODE_STA);
    
    res = esp_cwlap_detect(esp.wifista.ssid);
    if( res != 0 ){
        LOG_E("The SSID signal for WIFI could not be found");
        return -1;
    }
    
    res = esp_cwap_config(esp.wifista.ssid,esp.wifista.pwd);
    if( res != 0 ){
        LOG_E("wifi Network connection failure");
        return -1;
    }
    LOG_D("wifi connection successful");
    
    res = esp_cipsta_detect();
    if( res != 0 ){
        LOG_E("wifi Network connection failure");
        return -1;
    }
      
    res = esp_sntp_config(NULL);
    if( res != 0 ){
        LOG_E("NTP Synchronization failure");
        return -1;
    }
    
    LOG_D("NTP Synchronization Succeeded");

    return 0;
}