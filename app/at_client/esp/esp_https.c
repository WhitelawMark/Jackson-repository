/*
 *  esp_https.c
 *
 *  Created on: 2023年08月24日
 *
 *      Author: lwp
 *     
 *      Note  : 本接口函数目前只实现http的基本下载功能，未进行测试固件升级。
 *              该功能还需要实际进行验证 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <drivers/pin.h>
#include <sys/time.h>
#include <unistd.h> 
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include "type.h"
#include "ustring.h"
#include "at.h"
#include "at_client.h"
#include "esp.h"
#include "esp_https.h"



#define DBG_TAG "esp"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>

#define HTTPS_ACK_TAG    "+HTTPCLIENT:"
/*
********************************************************************************
********************************************************************************
*/
extern esp_at_t esp;
/*
********************************************************************************
                             内部功能函数
********************************************************************************
*/ 

/*
********************************************************************************
*Function    : __esp_https_get_data
*Description : 
*Input       :
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      : Command: AT+HTTPCLIENT=2,0,"http://www.baidu.com/img/bdlogo.gif",,,0,"Range: bytes=0-499"\r 
               Reply  :+HTTPCLIENT:<size>,<data>
                        OK
--------------------------------------------------------------------------------
AT+HTTPCLIENT=<opt>,<content-type>,<"url">,[<"host">],[<"path">],<transport_type>[,<"data">][,<"http_req_header">][,<"http_req_header">][...]
   <opt>：HTTP 客户端请求方法：
          1：HEAD
          2：GET
          3：POST
          4：PUT
          5：DELETE
   <content-type>：客户端请求数据类型：
          0：application/x-www-form-urlencoded
          1：application/json
          2：multipart/form-data
          3：text/xml
   <”url”>：HTTP URL，当后面的 <host> 和 <path> 参数为空时，本参数会自动覆盖这两个参数。
   <”host”>：域名或 IP 地址。
   <”path”>：HTTP 路径。
   <transport_type>：HTTP 客户端传输类型，默认值为 1：
          1：HTTP_TRANSPORT_OVER_TCP
          2：HTTP_TRANSPORT_OVER_SSL
   <”data”>：当 <opt> 是 POST 请求时，本参数为发送给 HTTP 服务器的数据。当 <opt> 不是 POST 请求时，这个参数不存在（也就是，不需要输入逗号来表示有这个参数）。
   <”http_req_header”>：可发送多个请求头给服务器。
********************************************************************************
*/
static int __esp_https_get_data(char *url, int mode,int offset,int packlen,char *pdu,int pdulen)
{
    char atcmd[256];
    char *tmpbuf;
    char  *str;
    int   recvlen;
    int   tmplen;
    int   ulen;
    int   offer;
    
    tmplen = pdulen+512;
    tmpbuf = rt_malloc(tmplen);
    if( tmpbuf == NULL ){
        LOG_E("[%s]: Memory malloc exception!!!",__func__);
        return -3;
    }

    at_snprintf(atcmd, sizeof(atcmd), "AT+HTTPCLIENT=2,0,\"%s\",,,%d,\"Range: bytes=%d-%d\"\r\n",
                                      url,mode,offset,offset+packlen-1);
    
    atcmd[sizeof(atcmd)-1] = '\0';
    if ( esp_cmd_send(atcmd) != AT_CMD_ACK_OK) {
        rt_free(tmpbuf);
        return -1;
    }
    
    recvlen = esp_cmd_line_get("OK",tmpbuf, tmplen, 10*1000);
    if ( recvlen <= 0) {
        rt_free(tmpbuf);
        return -1;
    }
    str = tmpbuf;
    offer = 0;
    
    do{
       //检查数据包长度
       if( tmpbuf+ recvlen < str){
           break;
       }
       //定位 +HTTPCLIENT:
       str = strstr(str,HTTPS_ACK_TAG);
       if(str == NULL){
           break;
       }
       str = str+strlen(HTTPS_ACK_TAG);
       //获取+HTTPCLIENT: 的长度值
       ulen = at_atoi(str);
       //过滤，
       while (*str && *str != ',') str++;
       if (*str) *str++ = '\0'; //skip ','
       //检查合法性，并且提取数据
       if(offer+ulen < pdulen){
           memcpy(pdu+offer,str,ulen);
       }else{
           LOG_D("[%s]: Memory overrun :[%d] ",__func__,pdulen);
       }
       str += ulen;
       offer+=ulen;
       
    }while(1);
    
    pdulen = offer;
    
    LOG_D("[%s]: pdulen:[%d] ",__func__,pdulen);
    
    LOG_HEX("__esp_https_get_data", 16, (uint8_t const *)pdu, pdulen);
 
    rt_free(tmpbuf);

    return pdulen;   
}
/*
********************************************************************************
*Function    : __esp_https_download
*Description :
*Input       :
*Output      :
*Return      : >=0 socket id , -1 if fail.
*Others      : 
********************************************************************************
*/
int __esp_https_download(socket_t *socket, https_msg_t *msg)
{
    int res;
    int pdulen,wrlen;
    int offer=0;
    char *pdubuf;
    int   fd;
    
    pdulen = msg->packlen*2;
    pdubuf = rt_malloc( pdulen );
    if( pdubuf == NULL ){
        LOG_E("[%s]: Memory malloc exception !!!",__func__);
        return -3;
    }
    
    LOG_D("[%s]: URL:[%s] ",__func__,msg->url);
    LOG_D("[%s]: packlen:[%d] ",__func__,msg->packlen);
    
    fd = open(msg->filename , O_CREAT | O_RDWR);
    if( fd < 0 ){ 
        LOG_E("[%s]: Failed to open %s file ",__func__,msg->filename);
        rt_free(pdubuf);
        return -1;
    }

    offer = 0;
    lseek(fd, offer, SEEK_SET);
    
    do{
       res =  __esp_https_get_data(msg->url,msg->mode,offer,msg->packlen,pdubuf, pdulen );
       if( res < 0 ){
           LOG_E("[%s]:https data get exception [res:%d]",__func__,res);
           break;
       } 
       if(res != 0 ){
          wrlen = res ;
          res = write(fd, pdubuf, wrlen); 
          if(res < 0 ){
              LOG_E("[%s]:File write failure [res:%d] packlen = %d",__func__,res);
              break;
          }
       }
       if( wrlen != msg->packlen ){
           LOG_D("[%s]: Complete transmission res[%d]",__func__,wrlen);
           break;
       } 
       LOG_D("[%s]:offer =[%d] res =[%d]",__func__,offer,res);
    
       offer+=res;

    }while(1);

    rt_free(pdubuf);
    
    return 0;
}
/*
********************************************************************************
                             对外接口函数
********************************************************************************
*/ 
/*
********************************************************************************
*Function    : esp_https_download
*Description : 使用https进行下载接口函数
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int esp_https_download(int sd,char *url,char *filename, void(*res_cb)(int res, void *arg), void *arg)
{
    socket_msg_t *msg;
    https_msg_t  *https_msg;
    
    if (sd < 0 || sd >= ESP_MAX_SOCKET) {
        return -1;
    }
 
    //socket not used
    if (atomic_read(&esp.sockets[sd].used) != 1) {
         return -1;
    }
    
    if( url == NULL ){
         return -1;
    }
    
    msg = (socket_msg_t *)at_message_alloc(sizeof(socket_msg_t) + sizeof(https_msg_t) );
    if (msg == RT_NULL) {
        return -1;
    }
    
    https_msg = (https_msg_t *)(msg->msg);
    at_memset(https_msg, 0, sizeof(https_msg_t));
    strcpy( https_msg->url ,  url );
    
    if( filename == NULL ){
        strcpy( https_msg->filename ,"/http");
    }else{
        strcpy( https_msg->filename ,filename);
    }
    
    https_msg->packlen = 1024;
    https_msg->write = 0; 
    https_msg->mode = 2; 
    
    msg->sd = sd;
    msg->type = MSG_HTTP_DOWNLOAD;
    msg->res_cb = res_cb;
    msg->arg = arg;

    return __esp_socket_msg_post(msg);
}
/*
********************************************************************************
*Function    : esp_https_init
*Description : https 初始化函数（设备只需要工作在STA模式就可以使用下载，）
*Input       :
*Output      :
*Return      : 
*Others      :
********************************************************************************
*/
int esp_https_init(char *ssid,char *pwd)
{   
    esp_set_wifi(ssid,pwd);
    
    esp_set_function(__esp_wifi_sta_init,0,0);
    
    return 1;
}