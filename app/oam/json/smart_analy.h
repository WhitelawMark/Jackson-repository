/*
 * smart_analy.h
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#ifndef __SMART_ANALY_H_
#define __SMART_ANALY_H_


#define  get_gwid()   "123456"
 
/*
********************************************************************************
********************************************************************************
*/
#define SMART_OK        200     /*success成功*/
#define SMART_FAIL      400     /*fail失败*/
#define SMART_UNSUPPORT 401     /*unsupport 不支持*/
#define SMART_UAUTH     402     /*unauthorized 未授权*/
#define SMART_TIMEOUT   403     /*requested Time-out 请求超时*/
#define SMART_NOFIND    404     /*no found   信息未查到*/
#define SMART_BINDED    405     /*binded 网关被绑定*/
#define SMART_PENDING   406     /*pending 处于…状态*/
#define SMART_EXISTED   407     /*existed  存在*/
#define SMART_PARM      408     /*param   参数错误*/
#define SMART_MSG       409     /*message exist   信息已存在*/




/*
********************************************************************************
********************************************************************************
*/

typedef int (smart_response)(char *msg, int msglen, void *arg);
typedef int (json_lowlayer)(const char *msg, int msglen, void *arg);
/*
 * smart_msg 信息处理
 */
typedef struct {
    char  msgtype;
    char  from[33];
    char  to[33];
    char  encrypt;
    char  broadcast;
    void  *arg;
} smart_msg_t; 



typedef struct {
    json_lowlayer *from;
    u32_t msgtype;
    u32_t subtype;
    void *arg;
} js_msg_t;
/*
********************************************************************************
********************************************************************************
*/
 u8_t smart_analy(u8_t msgtype,cJSON* root, json_lowlayer *from,void *arg);
 u8_t smart_response_send(char *action,cJSON* data,smart_msg_t *psmsg,int result,js_msg_t *to,void *arg);
 u8_t smart_oam_send(char *action,char *from,cJSON* data,int result);
 u8_t cjson_analy(u8_t msgtype,const char *pmsg,u16_t ulen, json_lowlayer *from,void *arg);
#endif 
/* __SMART_ANALY_H_ */

 