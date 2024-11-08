/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */
#ifndef PPITEMS_H
#define PPITEMS_H
 
/*
********************************************************************************
********************************************************************************
*/
#define PP_SYNC_EVENT            (0x01)    // 数据同步存储到存储器中           
#define PP_RE_TIMEOUT_EVENT      (0x02) 
#define PP_RESUME_FACTORY_EVENT  (0x04)  
#define PP_FORCE_FORMAT_EVENT    (0x08)  
#define PP_ROOT_RESUME_EVENT     (0x10) 
#define PP_REBOOT_EVENT          (0x20) 

#define PP_ERR_NONE                0x00
#define PP_ERR_ULEN                0xFF
#define PP_ERR_EMID                0x01

#define PP_MSG_SYNC                1
#define PP_MSG_REST_FACTORY        2
#define PP_MSG_DISK_FORMATT        3
#define PP_MSG_TIMEOUT             4


#define pparm_sync_noti()        pparm_evt_noti(PP_SYNC_EVENT)
#define pparm_delay_noti()       pparm_evt_noti(PP_RE_TIMEOUT_EVENT)
#define pparm_factory_noti()     pparm_evt_noti(PP_RESUME_FACTORY_EVENT)

//#define syswatch_reboot() 
/*
********************************************************************************
********************************************************************************
*/

typedef struct _FILE_OP_MQ
{
    u8_t  msgtype;         /* 信息类型 */     
    u8_t  *pmsg;           /* 文件名(这里用标识表示) */  
    u8_t  datalen;         /* 数据长度 */
}FileOpQMq_t;              /*文件操作        */


enum pparm_lock {
    PPARM_ULOCK  = 0 ,
    PPARM_LOCK   = 1 ,
};

typedef enum {
    #define PPDEF(name, size, defval) \
    name,
   #include "PPdef.h"
    PP_MAX_NUM
}REG_PP_ID_E;
/*
********************************************************************************
********************************************************************************
*/
void PPItemsFactory(void);  /* 恢复出厂化 */
void PPItemsMKFS(void);     /* 格式化磁盘 */
void PPItemSync(void);      /* 同步写入FLASH */

u32_t PPItemWrite(u32_t id, void *ptr,u32_t ulen); /* 写入数据 */
u32_t PPItemRead(u32_t id, void *ptr,u32_t ulen);  /* 查询数据 */

void PPItemSetInit(void (*fn)(void));
 
 u8_t PPItemSize(u32_t id);
u8_t *PPItemPtr(u32_t id);
 
 
#endif
/* PPITEMS_H */ 
