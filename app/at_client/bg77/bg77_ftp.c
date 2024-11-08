/*
 *  bg77_ftp.c
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
#include "bg77_ftp.h"


#define DBG_TAG "bg77"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
 
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    : bg77_ftp_close
*Description :
*Input       :
*Output      :
*Return      : 0   -- if success, -1 -- if fail.
*Others      : AT+QFTPCLOSE =<client_idx>
               \r\r\nOK
               +QFTPCLOSE : <client_idx>,<result>
               <result> Integer type. Result of the command execution
                       -1 Failed to close network
                        0 Network closed successfully
               here we detect "\r\nOK",  +QMTCLOSE: need wait 2 minute
********************************************************************************
*/
static int bg77_ftp_close(void)
{
    char atcmd[32];

    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPCLOSE \r");
    atcmd[sizeof(atcmd)-1] = '\0';

    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 5000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }

    return 0;
}
/*
******************************************************************************** 
*Function    : bg77_ftp_config
*Description : 
*Input       : filetype 
*              timeout   
*              username
*              password
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      :
******************************************************************************** 
*/	 
static int bg77_ftp_config(int filetype, int transmode,int timeout, const char *username, const char *password)
{
    char atcmd[128];
    char xbuf[20];

    //1、设置 PDP 上下文为 1
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPCFG=\"contextid\",1\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    //2、配置OTA FTP帐号密码
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPCFG=\"account\",\"%s\",\"%s\"\r", username,password);
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", xbuf, sizeof(xbuf), 5000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    //3、文件类型：二进制
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPCFG=\"filetype\",%d\r", filetype);//filetype = 0
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", xbuf, sizeof(xbuf), 5000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    
    //4、FTP主动模式
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPCFG=\"transmode\",%d \r", transmode);//transmode=1
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", xbuf, sizeof(xbuf), 5000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    //5、设置最大响应时间（默认为 90 秒）
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPCFG=\"rsptimeout\",%d\r", timeout);// timeout=90
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", xbuf, sizeof(xbuf), 5000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    return 0;
}    
/*
******************************************************************************** 
*Function    : bg77_ftp_open
*Description :
*Input       :
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      :
******************************************************************************** 
*/
static int bg77_ftp_open(char *ip ,unsigned short port)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20];  
    
    //登录FTP服务器
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPOPEN=\"%s\",%u\r", ip, (unsigned int)(port));
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "+QFTPOPEN:", xbuf, sizeof(xbuf), 5000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    str = xbuf;

    /*skip client_idx*/
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    /*result*/
    res = at_atoi(str);
    if (res != 0) {
        return -1;
    }

    return 0;
}
/*
******************************************************************************** 
*Function    : bg77_ftp_get
*Description : ftp 
*Input       :
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      :
******************************************************************************** 
*/
static int bg77_ftp_get(char *filename)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20];
 
  
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPGET=\"%s\",\"UFS:%s\"\r", filename,filename);
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "+QFTPGET", xbuf, sizeof(xbuf), 10000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    str = xbuf;

    /*skip client_idx*/
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    /*filesize*/
    res = at_atoi(str);
    if ( res != 0) {
        LOG_D("res=[%d]",res);
        return res;
    }

    return 0;
}
/*
******************************************************************************** 
*Function    : bg77_ftp_put
*Description : ftp 
*Input       :
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      :
******************************************************************************** 
*/
static int bg77_ftp_put(char *filename)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20];
 
  
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFTPPUT=\"%s\",\"UFS:%s\",0\r", filename,filename);
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "+QFTPPUT", xbuf, sizeof(xbuf), 2000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    str = xbuf;

    /*skip client_idx*/
    while (*str && *str != ',') str++;
    if (*str) str++; //skip ','

    /*filesize*/
    res = at_atoi(str);
    if ( res != 0) {
        LOG_D("res=[%d]",res);
        return res;
    }

    return 0;
}
/*  
********************************************************************************
Function      : __bg77_ftp_download
Description   : ftp 文件下载操作
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
int __bg77_ftp_download(socket_t *socket, ftp_msg_t *msg)
{    
    int res; 
      
    res = bg77_ftp_config(0, 1,90,  msg->username, msg->password);
    if( res != 0){
        LOG_E("File configuration failed");
        return -1;
    }
    res = bg77_ftp_open( msg->addr.ip,msg->addr.port);
    if( res != 0){
        LOG_E("File open failed ");
        return -1;
    }
     
    msg->filesize = bg77_ftp_get(msg->filename);
    
    if(msg->filesize < 0){
        bg77_ftp_close();
        return -1;
    }
    bg77_ftp_close();
 
    bg77_export_file(msg->filename,msg->filesize,msg);
    
    return 0;
}   
/* 
********************************************************************************
Function      : __bg77_ftp_upload
Description   : 文件上传操作函数 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
int __bg77_ftp_upload(socket_t *socket, ftp_msg_t *msg)
{
    int res;
 
    res = bg77_import_file(msg->filename,2048,msg);
    if( res != 0){
        LOG_E("File import failed! filename[%s]",msg->filename);
        return -1;
    }
   
    res = bg77_ftp_config(0, 1,90, msg->username, msg->password);
    if( res != 0){
        LOG_E("File configuration failed username = %s password =%s",msg->username, msg->password);
        return -1;
    }
    res = bg77_ftp_open(msg->addr.ip,msg->addr.port);
    if( res != 0){
        LOG_E("File open failed");
        return -1;
    }
     
    bg77_ftp_put(msg->filename);
    
    bg77_ftp_close();
    
    return 0;
}
/*
********************************************************************************
*Function    : bg77_ftp_post
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_ftp_post(int type, char *ip, int port, const char *filename,
                      const char *username,const char *password, 
                      ftp_device_t *device)
{
    socket_msg_t *msg;
    ftp_msg_t *ftp_msg;
    ftp_device_t *device_msg;


    if (ip == RT_NULL || filename == RT_NULL || username == RT_NULL|| password == RT_NULL) {
        return -1;
    }
    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(ftp_msg_t)+sizeof(ftp_device_t));
    if (msg == RT_NULL) {
        return -1;
    }
    ftp_msg = (ftp_msg_t *)(msg->msg);
    at_memset(ftp_msg, 0, sizeof(ftp_msg_t));

    device_msg = (ftp_device_t *)(msg->msg+sizeof(ftp_msg_t));
    
    at_memcpy(device_msg,   device,   sizeof(ftp_device_t));
    at_memcpy(ftp_msg->addr.ip,   ip,   at_strlen(ip)+1);
    ftp_msg->addr.port  = port;
 
    at_memcpy(ftp_msg->filename,   filename, at_strlen(filename)+1);
    at_memcpy(ftp_msg->username,   username, at_strlen(username)+1);
    at_memcpy(ftp_msg->password,   password, at_strlen(password)+1);

    msg->sd = at_socket_create(SOCK_FTP);
 
    msg->type = (msg_type_t)type;
  //  msg->arg = arg;
    msg->res_cb = 0;

    return __bg77_socket_msg_post(msg);
}
/*
********************************************************************************
Function      : bg77_import_file
Description   : 文件导出操作 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_import_file(char *filename,int filesize,ftp_msg_t *msg)
{     
    int handle;
    int offset=0;
    char *pbuf;
    
    if(msg->device.finit){
        filesize = msg->device.finit(0);
    }
    if(filesize <= 0){
        LOG_E(" filesize=[%d]",filesize);
        return -1;
    }
    handle = bg77_qfopen(filename,1);
    if( handle < 0 ){
        LOG_E("bg77_qfopen handle=[%d]",handle);
        return -1;
    }  
    pbuf = (char *)at_message_alloc(1024);
    if (pbuf == RT_NULL) {
        return -1;
    }
    for(int i=0,offset=0,res = 512;offset<filesize; ){
        if(msg->device.fread ){
            res = msg->device.fread(msg->device.address+offset,pbuf,res);
        }else{
            memset(pbuf,i,res);i++;
        }
        bg77_qfseed(handle,offset,0);
    
        res = bg77_qfwrite(handle,res,pbuf,res);
        if(res <= 0){
            LOG_E("bg77_qfwrite res=[%d]",res);
            break;
        }
        LOG_D("[%08x] size:[%04d]recv=[%02x %02x %02x %02x %02x %02x %02x %02x]",
              offset,res,pbuf[0],pbuf[1],pbuf[2],pbuf[3],pbuf[4],pbuf[5],pbuf[6],pbuf[7]);
        offset+=res;
    }
    bg77_qfclose(handle);
    
    at_message_free(pbuf);
    
    if(msg->device.operation){
        msg->device.operation(0,filesize);
    }
    LOG_D("File import filesize =%d,offset =%d",filesize,offset);
    LOG_D("File import completed");

    return 0;
}
/*
********************************************************************************
Function      : bg77_export_file
Description   : 文件导出操作 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_export_file(char *filename,int filesize,ftp_msg_t *msg)
{    
    int res;
    int handle;
    int offset=0;
    char *pbuf;
    
    handle = bg77_qfopen(filename,2);
    if( handle < 0 ){
        LOG_E("bg77_qfopen handle=[%d]",handle);
        return -1;
    }  
    if(filesize == 0){
        bg77_qfseed(handle,0,2);
        filesize = bg77_qfposition(handle);
        if(filesize < 0){
           LOG_E("bg77_qfposition filesize=[%d]",filesize);
           return -1;
        }
        LOG_D("File filesize =%d ",filesize);
    }
    if(msg->device.finit){
        msg->device.finit(0);
    }
    pbuf = (char *)at_message_alloc(1024);
    if (pbuf == RT_NULL) {
        return -1;
    }
    rt_thread_mdelay(500);
    for(offset=0;offset<filesize; ){
        
        bg77_qfseed(handle,offset,0);
        
        res = bg77_qfread(handle,512,pbuf,600);
        
        LOG_D("[%08x] size:[%04d]recv=[%02x %02x %02x %02x %02x %02x %02x %02x]",
              offset,res,pbuf[0],pbuf[1],pbuf[2],pbuf[3],pbuf[4],pbuf[5],pbuf[6],pbuf[7]);
        
        if(res < 0 ){
            break; 
        }
        if(msg->device.fwrite){
            msg->device.fwrite(msg->device.address+offset,pbuf,res);
        }
        offset+=res;
    }
    bg77_qfclose(handle);
    
    at_message_free(pbuf);
    
    bg77_qfdel(filename);
    
    if(msg->device.operation){
        msg->device.operation(0,filesize);
    }
    LOG_D("File export filesize =%d,offset =%d",filesize,offset);

    return 0;
} 
