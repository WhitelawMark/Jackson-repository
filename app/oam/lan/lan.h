/*
 * lan.h
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#ifndef __BLUE_H_
#define __BLUE_H_


/*
********************************************************************************
********************************************************************************
*/
#define LAN_MAX_LEN     (251)
#define LAN_ERR_LEN     LAN_MAX_LEN


#define BLUE_SET 0X02
#define BLUE_GET 0X03
#define BLUE_ACK 0X10
/*
********************************************************************************
********************************************************************************
*/


typedef int (lan_lowlayer)(u8_t *msg, u32_t msglen, void *arg);

typedef struct {
    lan_lowlayer *from;
    void *arg;
    u32_t len;
    u8_t buf[LAN_MAX_LEN];
} lan_msg_t;

typedef struct {
    lan_lowlayer *to;
    void *arg;

} lanlayer_t;
/*
********************************************************************************
********************************************************************************
*/
void lan_recv_frame(int sd, char *buf, int len);
s32_t lan_send(u8_t flg,u8_t cmdid,u8_t *msg,u8_t msglen,lan_lowlayer *to);
void lan_set(u8_t *msg, u32_t len, lan_lowlayer *from, void *arg);
void lan_get(u8_t *msg, u32_t len, lan_lowlayer *from, void *arg);
void lan_msg_recv(lan_msg_t *msg, u8_t ch);
#endif 
/* __BLUE_H_ */

 