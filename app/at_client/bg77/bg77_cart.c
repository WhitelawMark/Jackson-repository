/*
 *  bg77_cart.c
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
********************************************************************************
*/
/*
********************************************************************************
*Function    : bg77_cart_read
*Description : 证书专用函数
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static int bg77_cart_read(const char *filename,char *buf,int buflen)
{
    int  fd;
    int ulen; 

    fd = open( filename , O_RDONLY);
    if( fd == -1 ){ 
        LOG_E("Unable to find file %s ",filename);
        return 0;
    }
    
    lseek(fd, 0, SEEK_CUR);

    ulen = read(fd,  (char*)buf, buflen);
    
    close(fd);
    
    return ulen;
}
/*
********************************************************************************
*Function    : _bg77_cart_qfupl
*Description : 本文件仅用于证书上传使用，需支持文件系统才可以使用该接口
*Input       :
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      :
********************************************************************************
*/
static int bg77_cart_qfupl(char *filename)
{
    char filepath[64];
    int  res;
    char *pbuf;
    int  filesize;

    sprintf(filepath,"/%s",filename);
    
    filesize = bg77_file_size(filepath);
    if( filesize <= 0 ||  filesize > 4096){
        return -1;
    }
    
    pbuf = at_message_alloc(filesize);
    if( pbuf == NULL ){
        LOG_E("%s at_message_alloc !!!",__func__);
        return -2;
    }

    res = bg77_cart_read(filepath,pbuf,filesize);
    if( res <= 0 ){
        at_message_free(pbuf);
        return -3;    
    }
        
    res = bg77_qfupl(filename,pbuf,filesize);
    if( res < 0 ){
        at_message_free(pbuf);
        return -4; 
    }

    at_message_free(pbuf);
    
    return 0;
}
/*
********************************************************************************
*Function    : bg77_cart_updata
*Description : 
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
static int bg77_cart_updata(char *filename)
{
    LOG_D("[%s] file %s ",__func__,filename);
    
    bg77_qfdel(filename);

    bg77_cart_qfupl(filename);
    
    return 0;
}
/*  
********************************************************************************
Function      : __bg77_cert_upload
Description   : 证书上传
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
int __bg77_cert_upload(socket_t *socket, cart_msg_t *msg)
{    

    if( msg->type == CART_UPL ){
      
        bg77_cart_updata(msg->ca);

        bg77_cart_updata(msg->client);

        bg77_cart_updata(msg->key);
        
    }else if( msg->type == CART_DEL ){
      
        bg77_qfdel(msg->ca);
    
        bg77_qfdel(msg->client);
        
        bg77_qfdel(msg->key);
    }
    
    return 0;
}
/*
********************************************************************************
*Function    : bg77_cert_upload
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int bg77_cert_upload(int sd,int type ,char *ca,char *client, char *key,
                     void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    cart_msg_t *cart_msg;
    char *buf;
    int number;
    unsigned int len = 0;

    if (sd < 0 || sd >= BG77_MAX_SOCKET) {
        return -1;
    }
    
    if(ca == NULL || client == NULL || key == NULL){
        return -1;
    }
    /* Data cache length calculation */
    number = 0;

    len += (at_strlen(ca) + 1);
    number++;
        
    len += (at_strlen(client) + 1);
    number++;
 
    len += (at_strlen(key) + 1);
    number++;

    //socket not used
    if (atomic_read(&bg77.sockets[sd].used) != 1) {
        return -1;
    }
 
    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(cart_msg_t)+len);
    if (msg == RT_NULL) {
        return -1;
    }
    // Certificate information copy
    cart_msg = (cart_msg_t *)(msg->msg);
    at_memset(cart_msg, 0, sizeof(cart_msg_t));
    buf = (char *)(msg->msg + sizeof(cart_msg_t));
   
    cart_msg->type = type;
    
    LOG_D("[%s] size %d  cart_msg_t size %d ",__func__,sizeof(socket_msg_t) + sizeof(cart_msg_t)+len,sizeof(cart_msg_t));
    
    strcpy(buf, ca); 
    cart_msg->ca = buf;
    buf+= (at_strlen(ca) + 1);
    
    strcpy(buf, client); 
    cart_msg->client = buf;
    buf+= (at_strlen(client) + 1);
    
    strcpy(buf, key); 
    cart_msg->key = buf;
    
    // Message information copy
    msg->sd = sd;
    msg->type = MSG_CERT_UPLOAD;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __bg77_socket_msg_post(msg);
}
