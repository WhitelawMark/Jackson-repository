/*
 *  bg77_file.c
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
#include "bg77_file.h"


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
********************************************************************************
*/
/*
********************************************************************************
Function      : bg77_qfdel
Description   : 删除UFS下的文件名
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_qfdel(char *filename)
{
    char atcmd[128];
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFDEL=\"%s\"\r",filename);
    atcmd[sizeof(atcmd)-1] = '\0';
    if ( bg77_cmd_line(atcmd, "OK", 0, 0, 5000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    return 0;
}
/*
********************************************************************************
Function      : bg77_qfopen
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_qfopen(char *filename,int mode)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20]; 
 
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFOPEN=\"%s\",%d\r", filename,mode);
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    if (bg77_cmd_line(atcmd, "+QFOPEN:", xbuf, sizeof(xbuf), 2000, 1) != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    str = xbuf;

    /*skip client_idx*/
    while (*str && *str != ' ') str++;
    if (*str) str++; //skip ','

    /*filehandle*/
    res = at_atoi(str);
   
    return res;
}
/*
********************************************************************************
Function      : bg77_qfclose
Description   :  
Input         :AT+QFCLOSE=20
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_qfclose(int handle)
{
    char atcmd[128];
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFCLOSE=%d\r",handle);
    atcmd[sizeof(atcmd)-1] = '\0';
    if ( bg77_cmd_line(atcmd, "OK", 0, 0, 5000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    return 0;
}
/*
********************************************************************************
Function      : bg77_qfseed
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_qfseed(int handle,int offset,int position)
{
    char atcmd[128];
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFSEEK=%d,%d,%d\r",handle,offset,position);
    atcmd[sizeof(atcmd)-1] = '\0';
    if ( bg77_cmd_line(atcmd, "OK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    return 0;
}
/*
********************************************************************************
Function      : bg77_qfposition
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_qfposition(int handle)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20]; 
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFPOSITION=%d\r", handle);
    atcmd[sizeof(atcmd)-1] = '\0';

    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    res = bg77_cmd_line(atcmd, "+QFPOSITION: ", xbuf, sizeof(xbuf), 2000, 1) ;
    if ( res != AT_CMD_DATA_GET_OK) {
        return -1;
    }
    str = xbuf;

    /*file size*/
    res = at_atoi(str);
    
    return res;
}
/*
********************************************************************************
Function      : bg77_qfwrite
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_qfwrite(int handle,int timeout,char *buf,int size)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20];
 
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFWRITE=%d,%d,%d\r",handle,size,timeout);
    atcmd[sizeof(atcmd)-1] = '\0';

    res = bg77_cmd_line(atcmd, "CONNECT", 0, 0, 5000, 1);
    if ( res != AT_CMD_ACK_OK) {
        LOG_E("cmd result error =[%d]",res);
        return -1;
    }
 
    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    
    res = bg77_cmd_line_with_len(buf,size ,"+QFWRITE", xbuf, sizeof(xbuf), 5000, 1);
    if ( res != AT_CMD_DATA_GET_OK) {
        LOG_E("cmd result error =[%d]",res);
        return -1;
    }
    str = xbuf;
    /*res*/
    res = at_atoi(str+1);
    if ( res <= 0) {
        LOG_E("qfwrite size =[%d]",res);
        return -1;
    }

    
    LOG_D("res=[%d] size=[%d]",res,size);
    
    return res;
}
/*
********************************************************************************
Function      : bg77_qfread
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_qfread(int handle,int size,char *buf,int bufsize)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[30];
 
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFREAD=%d,%d\r",handle,size);
    atcmd[sizeof(atcmd)-1] = '\0';
    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    
    res = bg77_cmd_line(atcmd, "CONNECT", xbuf, sizeof(xbuf), 8000, 1);
    if ( res != AT_CMD_DATA_GET_OK) {
        LOG_E("cmd result error =[%d]",res);
        return -1;
    }
    str = xbuf;
    /*bufsize*/
    bufsize = at_atoi(str+1);
    if ( bufsize <= 0) {
        LOG_E("qfread size =[%d]",bufsize);
        return -1;
    }
    res = bg77_cmd_line_get(0, buf, bufsize, 5000);
   
    return res;
}
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
Function      : bg77_qfupl
Description   : 上传文件 (一次性上传完成，无应答处理)
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int bg77_qfupl(char *filename,char *buf,int size)
{
    char *str;
    int res;
    char atcmd[128];
    char xbuf[20];
 
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFUPL=\"%s\",%d\r",filename,size);
    atcmd[sizeof(atcmd)-1] = '\0';
    res = bg77_cmd_line(atcmd, "CONNECT", 0, 0, 5000, 1);
    if ( res != AT_CMD_ACK_OK) {
        LOG_E("[%s]:cmd result error =[%d]",__func__,res);
        return -1;
    }
 
    at_strncpy(xbuf, "\r\n", sizeof(xbuf));
    xbuf[sizeof(xbuf)-1] = '\0';
    
    res = bg77_cmd_line_with_len(buf,size ,"+QFUPL", xbuf, sizeof(xbuf), 5000, 1);
    if ( res != AT_CMD_DATA_GET_OK) {
        LOG_E("[%s]:cmd result error =[%d]",__func__,res);
        return -1;
    }
    str = xbuf;
    /*res*/
    res = at_atoi(str+1);
    if ( res <= 0) {
        LOG_D("[%s]:res=[%d] size=[%d]",__func__,res,size);
        return -1;
    }

    return 0;
}
/*
********************************************************************************
Function      : __bg77_qfupl_ack
Description   : 分包上传，有应答机制
Input         : src 源文件
                dsc 目标文件
Output        :
Return        :
Others        :
********************************************************************************
*/
int __bg77_qfupl_ack(char *fpsrc,char *fpdsc)
{    
    int  fd;
    int  size,offset,fplen; 
    char *str;
    int  res;
    char atcmd[128];
    char xbuf[20];
    char *pbuf;
    
    
    /*检查文件合法性*/
    if( fpsrc == NULL || fpdsc == NULL ){
        LOG_E("[%s]:fpsrc or fpdsc is NULL ",__func__);
        return 0;
    }
 
    fd = open( fpsrc , O_RDONLY);
    if( fd == -1 ){ 
        LOG_E("[%s]:Unable to find file %s ",__func__,fpsrc);
        return 0;
    }
    
    /* 获取文件长度 */ 
    lseek(fd, 0, SEEK_CUR);
    fplen = lseek(fd, 0, SEEK_END);
    
    lseek(fd, 0, SEEK_SET);
    
    /* 申请缓存 */ 
    size = 1024 ;
    pbuf = at_message_alloc(1024);
    if( pbuf == NULL ){
        LOG_E("[%s]: at_message_alloc !!!",__func__);
        return -2;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QFUPL=\"%s\",%d\r",fpdsc,fplen);
    atcmd[sizeof(atcmd)-1] = '\0';
    
    res = bg77_cmd_line(atcmd, "CONNECT", 0, 0, 5000, 1);
    if ( res != AT_CMD_ACK_OK) {
        LOG_E("[%s]:cmd result error =[%d]",__func__,res);
        at_message_free(pbuf);
        return -2;
    }
    
    for(offset=0;offset < fplen ; ){
      
        size = read(fd,  (char*)pbuf, size);
      
        if( offset+size < fplen ){
            res = bg77_cmd_line_with_len(pbuf,size ,"A", 0, 0, 5000, 1);
            if ( res != AT_CMD_ACK_OK) {
                 LOG_E("[%s]:cmd result error =[%d]",__func__,res);
                 break;
            }
        }else{
            at_strncpy(xbuf, "\r\n", sizeof(xbuf));
            xbuf[sizeof(xbuf)-1] = '\0';
            size = fplen-offset;
            res = bg77_cmd_line_with_len(pbuf,size ,"+QFUPL", xbuf, sizeof(xbuf), 5000, 1);
            if ( res != AT_CMD_DATA_GET_OK) {
                LOG_E("[%s]:cmd result error =[%d]",__func__,res);
                break;
            }
            str = xbuf;
            /*res*/
            res = at_atoi(str+1);
            if ( res <= 0) {
                LOG_D("[%s]: res=[%d] size=[%d]",__func__,res,size);
                break;
            }
        }
    }
    
    at_message_free(pbuf);
    
    return 0;
}
/*
********************************************************************************
Function      : bg77_file_import
Description   : 从UFFS文件系统导入到 BG77内部的文件系统 
Input         : 
Output        :
Return        :
Others        :
********************************************************************************
*/
static int __bg77_file_import(char *to,char *from)
{     
    int  fd;
    int  fplen; 
    int  handle;
    int  offset=0;
    char *pbuf;
    int  res;
    
    if(to == NULL || from == NULL ){
        LOG_E("[%s]:fpsrc or fpdsc is null",__func__);
        return -1;
    }
    
    fd = open( from , O_RDONLY);
    if( fd == -1 ){ 
        LOG_E("[%s]:Unable to find file %s ",__func__,from);
        return 0;
    }
    
    /* 获取文件长度 */ 
    lseek(fd, 0, SEEK_CUR);
    fplen = lseek(fd, 0, SEEK_END);
    
    lseek(fd, 0, SEEK_SET);
    
    handle = bg77_qfopen(to,1);
    if( handle < 0 ){
        LOG_E("[%s]:bg77_qfopen handle=[%d]",__func__,handle);
        return -1;
    }  
    pbuf = (char *)at_message_alloc(1024);
    if (pbuf == RT_NULL) {
        return -1;
    }
    for(offset=0,res = 512;offset<fplen; ){
        res = read(fd,pbuf,res);

        bg77_qfseed(handle,offset,0);
    
        res = bg77_qfwrite(handle,res,pbuf,res);
        if( res <= 0 ){
            LOG_E("[%s]:bg77_qfwrite res=[%d]",__func__,res);
            break;
        }
        LOG_HEX("HEX", 16, (uint8 const *)pbuf, res);
        
        offset+=res;
    }
    bg77_qfclose(handle);
    
    at_message_free(pbuf);
    

    LOG_D("[%s]:File import filesize =%d,offset =%d",__func__,fplen,offset);
    LOG_D("[%s]:File import completed",__func__);

    return 0;
}
/*
********************************************************************************
Function      : bg77_file_export
Description   : 文件导出操作 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static int __bg77_file_export(char *to,char *from)
{    
    int fd;
    int res;
    int handle;
    int offset=0;
    int fplen;
    char *pbuf;
    
    if(to == NULL || from == NULL ){
        LOG_E("[%s]:fpsrc or fpdsc is null",__func__);
        return -1;
    }
    
    handle = bg77_qfopen(from,2);
    if( handle < 0 ){
        LOG_E("[%s]:bg77_qfopen handle=[%d]",__func__,handle);
        return -1;
    }  

    bg77_qfseed(handle,0,2);
    fplen = bg77_qfposition(handle);
    if(fplen < 0){
        LOG_E("[%s]: bg77_qfposition filesize=[%d]",__func__,fplen);
        return -1;
    }
    LOG_D("[%s]:File filesize =%d ",__func__,fplen);
    
    fd = open(to , O_WRONLY|O_CREAT);
    if( fd < 0 ) {
        return -1;
    }
    pbuf = (char *)at_message_alloc(1024);
    if (pbuf == RT_NULL) {
        return -1;
    }

    for(offset=0;offset<fplen; ){
        
        bg77_qfseed(handle,offset,0);
        
        res = bg77_qfread(handle,512,pbuf,600);
        if(res < 0 ){
            break; 
        }
        LOG_HEX("File Test", 16, (uint8 const *)pbuf, res);
        res = write(fd,  (char*)pbuf, res);
        if(res < 0 ){
            break; 
        }

        offset+=res;
    }
    bg77_qfclose(handle);
    
    at_message_free(pbuf);

    LOG_D("[%s]:File export filesize =%d,offset =%d",__func__,fplen,offset);

    return 0;
}
/*
********************************************************************************
Function      : __bg77_file_operation
Description   :   
Input         :
Output        :
Return        :
Others        :   
********************************************************************************
*/
int __bg77_file_operation(socket_t *socket, file_msg_t *msg)
{
    switch( msg->type){
        case FILE_UPL:
            __bg77_qfupl_ack(msg->to,msg->from);
            break;
        case FILE_DWL:
          
            break;
        case FILE_IMP:
             __bg77_file_export(msg->to,msg->from);
            break;
        case FILE_EMP:
             __bg77_file_import(msg->to,msg->from);
            break;
        default:
            break;
    }
    return 0;
}
/*
********************************************************************************
*Function    : bg77_file_operation
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_file_operation(int sd,int type ,char *to,char *from, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    file_msg_t *file_msg;
    char *buf;
    unsigned int len = 0;
    unsigned int slen;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }
    //check file msg
    if (to == NULL || from == NULL) {
        return -1;
    }
    len = at_strlen(to)+at_strlen(from)+2;
    
    //socket not used
    if (atomic_read(&bg77.sockets[sd].used) != 1) {
        return -1;
    }
    
    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(file_msg_t)+len);
    if (msg == RT_NULL) {
        return -1;
    }
    //copy msg to buf
    file_msg = (file_msg_t *)(msg->msg);
    at_memset(file_msg, 0, sizeof(file_msg_t)+len);
    buf = (char *)(msg->msg + sizeof(file_msg_t));

    file_msg->type = type;
    
    slen = at_strlen(to);
    at_strncpy(buf, to,slen); 
    file_msg->to = buf;
    buf+=slen+1;

    slen = at_strlen(from);
    at_strncpy(buf, from,slen); 
    file_msg->from = buf;
    
    /* 消息信息 */
    msg->sd = sd;
    msg->type = MSG_FILE;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __bg77_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : bg77_file_export
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_file_export(int sd,char *to,char *from, void(*res_cb)(int res, void *arg), void *arg)
{
    return bg77_file_operation(sd, FILE_EMP ,to,from, res_cb, arg);
}
/*
********************************************************************************
*Function    : bg77_file_import
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_file_import(int sd,char *fpsrc,char *fpdsc, void(*res_cb)(int res, void *arg), void *arg)
{
    return bg77_file_operation(sd, FILE_IMP ,fpsrc,fpdsc, res_cb, arg);
}
