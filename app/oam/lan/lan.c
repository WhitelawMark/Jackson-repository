/*
 * lan.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include "app_lib.h"

#define DBG_TAG "lan"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>       


/*
********************************************************************************
********************************************************************************
*/
static u16_t g_packid;
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    :  lan_analy 
*Description :  lan数据解析接口函数
*Input       :
*Output      :  
*Return      :
*Others      :
********************************************************************************
*/
u8_t lan_analy(u8_t *pcmd, u32_t cmdlen, lan_lowlayer *from, void *arg)
{
    u8_t crcval;
    u8_t type;
    
    //1、合法性判断
    if (pcmd[0] != 0xA5 || pcmd[cmdlen-1] != 0x5A) {
        LOG_E("The frame header and frame tail are incorrect");
        return -1;
    }
    //2、LEN校验
    if (cmdlen != pcmd[1]+4 ||cmdlen > LAN_MAX_LEN) {
        LOG_E("Message length invalid");
        return -1;
    }
    //3、CRC校验
    crcval = check_sum(pcmd+1,cmdlen-3);  
    if (crcval != pcmd[cmdlen-2]) {
        LOG_E("Packet checksum is invalid");
        return -1;
    }
    //4、PACKETID
    g_packid  = (((((u16_t)(pcmd[3])) << 8 ) & 0xFF00) | pcmd[2]);
    
    //5、消息类型
    type = pcmd[4];  
 
    switch (type) {
        case 0x00:  //确认或否认
            //lan_ack(msg, len, from, arg);
            break;
        case 0x02:  //设置参数命令
            lan_set(pcmd+5, cmdlen-5, from, arg);
            break;
        case 0x03:  //读取参数命令
            lan_get(pcmd+5, cmdlen-5, from, arg);
            break;
        default:    //未知消息类型
            LOG_E("unknown message type %d\n", type);
            break;
    }

    return 0;
}   

/*
********************************************************************************
*Function    : lan_msg_process
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void lan_msg_process(void *data, int data_len)
{
    lan_msg_t *msg=data;
    
    lan_analy((u8_t *)msg->buf, msg->len,  msg->from, 0);
}
/*
********************************************************************************
*Function    : lan_recv_frame
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void lan_recv_frame(int sd, char *buf, int len)
{
    lan_msg_t msg;
   
    memcpy( msg.buf , buf, len);
    
    msg.len = len;
    msg.from = NULL;
	oam_thread_post(lan_msg_process, &msg, sizeof(msg));
}
/*
********************************************************************************
*Function    : lan_msg_post
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void lan_msg_post(lan_msg_t *msg)
{
    if (oam_thread_post((void(*)(void *, int))lan_msg_process, msg, sizeof(*msg)) != 0) {
        LOG_E("lan message post fail");
    }
}
/*
********************************************************************************
*Function    : lan_msg_recv
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void lan_msg_recv(lan_msg_t *msg, u8_t ch)
{
    //数据包太长,放弃
    if (msg->len >= sizeof(msg->buf)) {
        msg->len = 0;
    }

    if (msg->len > 0) {
        msg->buf[msg->len++] = ch;
        if (ch == 0x5A || msg->len == msg->buf[1]+4) {
            lan_msg_post(msg);
            msg->len = 0;
        }
    } else {
        if (ch == 0xA5) {
            msg->len=0;
            msg->buf[msg->len++] = ch;
        }
    }
}
/*
********************************************************************************
*Function    :  lan_send 
*Description :  
*Input       :
*Output      :  
*Return      :
*Others      :
********************************************************************************
*/
s32_t lan_send(u8_t flg,u8_t cmdid,u8_t *msg,u8_t msglen,lan_lowlayer *to)
{
    u8_t buff[256];
    
    buff[0]=0xA5;
    buff[1]=msglen+4;
   
    buff[2]=(u8_t)g_packid; 
    buff[3]=(u8_t)(g_packid >> 8); 
    
    buff[4]=flg;  
    buff[5]=cmdid;  
    
    memcpy(buff+6,msg,msglen);
    
    buff[6+msglen]=check_sum(buff+1,msglen+5);  
    buff[7+msglen]=0x5A;  
    
    LOG_HEX("lan_send", 16, buff, msglen+8);
    if (to) {
        return to(buff, msglen+8, 0);
    }
    return 0;
}
