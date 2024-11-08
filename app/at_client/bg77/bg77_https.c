/*
 *  bg77_https.c
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
#include "bg77_https.h"


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
Function      : __bg77_https_download
Description   : 文件下载
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
int __bg77_https_download(socket_t *socket, https_msg_t *msg)
{    
    char atcmd[128];

    at_snprintf(atcmd, sizeof(atcmd), "AT+QHTTPCFG=\"sslctxid\",1\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QHTTPCFG=\"sslversion\",1,1\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QHTTPCFG=\"ciphersuite\",1,0xFFFF\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    
    at_snprintf(atcmd, sizeof(atcmd), "AT+QHTTPCFG=\"seclevel\",1,2\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"cacert\",1,\"cacert.pem\"\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"clientcert\",1,\"clientcert.pem\"\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    at_snprintf(atcmd, sizeof(atcmd), "AT+QSSLCFG=\"clientkey\",1,\"clientkey.pem\"\r");
    atcmd[sizeof(atcmd)-1] = '\0';
    if (bg77_cmd_line(atcmd, "\r\nOK", 0, 0, 2000, 1) != AT_CMD_ACK_OK) {
        return -1;
    }
    LOG_D("[%s]:http url=[%s]",__func__,msg->url);
    LOG_D("[%s]:http ca=[%s],client=[%s],userkey=[%s]",__func__,msg->ca_file,msg->client_file,msg->key_file);

    return 0;
}

/*  
********************************************************************************
Function      : __bg77_https_upload
Description   : 文件下载
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
int __bg77_https_upload(socket_t *socket, https_msg_t *msg)
{    
    LOG_D("[%s]:http url=[%s]",__func__,msg->url);
    LOG_D("[%s]:http ca=[%s],client=[%s],userkey=[%s]",__func__,msg->ca_file,msg->client_file,msg->key_file);

    return 0;
}
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    : bg77_https_download
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_https_download(int sd,char *url,char *ca_file,char *client_file, char *key_file,
                     void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    https_msg_t *http_msg;
    char *buf;
    unsigned int len = 0;
    unsigned int slen=0;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }
   
    //socket not used
    if (atomic_read(&bg77.sockets[sd].used) != 1) {
        return -1;
    }
    //check url  
    if(url == NULL){
        return -1;
    }
    len=at_strlen(url)+at_strlen(ca_file)+at_strlen(client_file)+at_strlen(key_file)+4;
    
    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(https_msg_t)+len);
    if (msg == RT_NULL) {
        return -1;
    }
    //
    http_msg = (https_msg_t *)(msg->msg);
    at_memset(http_msg, 0, sizeof(https_msg_t)+len);
    buf = (char *)(msg->msg + sizeof(https_msg_t));
    
    slen = at_strlen(url);
    at_strncpy(buf, url,slen); 
    http_msg->url = buf;
    buf+=slen+1;
    
    if(ca_file != NULL){
        slen = at_strlen(ca_file);
        at_strncpy(buf, ca_file,slen); 
        http_msg->ca_file = buf;
        buf+=slen+1;
    }else{
        http_msg->ca_file = NULL;
    }
    
    if(client_file != NULL){
        slen = at_strlen(client_file);
        at_strncpy(buf, client_file,slen); 
        http_msg->client_file = buf;
        buf+=slen+1;
    }else{
        http_msg->client_file = NULL;
    }
    
    if(key_file != NULL){
        slen = at_strlen(key_file);
        at_strncpy(buf, key_file,slen); 
        http_msg->key_file = buf;
        buf+=slen+1;
    }else{
        http_msg->key_file = NULL;
    }
    
    msg->sd = sd;
    msg->type = MSG_HTTP_DOWNLOAD;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __bg77_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : bg77_https_upload
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_https_upload(int sd,char *url,char *ca,char *client, char *key,
                     void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    https_msg_t *http_msg;
    char *buf;
    unsigned int len = 0;
    unsigned int slen=0;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }
   
    //socket not used
    if (atomic_read(&bg77.sockets[sd].used) != 1) {
        return -1;
    }
    if(url == NULL){
        return -1;
    }
    len=at_strlen(url)+at_strlen(ca)+at_strlen(client)+at_strlen(key)+4;
    
    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(https_msg_t)+len);
    if (msg == RT_NULL) {
        return -1;
    }
    //
    http_msg = (https_msg_t *)(msg->msg);
    at_memset(http_msg, 0, sizeof(https_msg_t)+len);
    buf = (char *)(msg->msg + sizeof(https_msg_t));
    
    slen = at_strlen(url);
    at_strncpy(buf, url,slen); 
    http_msg->url = buf;
    buf+=slen+1;
    
    if(ca != NULL){
        slen = at_strlen(ca);
        at_strncpy(buf, ca,slen); 
        http_msg->ca_file = buf;
        buf+=slen+1;
    }else{
        http_msg->ca_file = NULL;
    }
    
    if(client != NULL){
        slen = at_strlen(client);
        at_strncpy(buf, client,slen); 
        http_msg->client_file = buf;
        buf+=slen+1;
    }else{
        http_msg->client_file = NULL;
    }
    
    if(key != NULL){
        slen = at_strlen(key);
        at_strncpy(buf, key,slen); 
        http_msg->key_file = buf;
        buf+=slen+1;
    }else{
        http_msg->key_file = NULL;
    }
    
    msg->sd = sd;
    msg->type = MSG_HTTP_UPLOAD;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __bg77_socket_msg_post(msg);
}
