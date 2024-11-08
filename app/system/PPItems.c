/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#include "board.h"
#include "app_lib.h"
#include <fcntl.h>
/*
********************************************************************************
********************************************************************************
*/
 
#define LOG_TAG              "app.parm"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

#define SYS_PARM_PPITEM           "/parm"
/*
********************************************************************************
********************************************************************************
*/
#define PPITEM_MSG_UNIT             4
#define PPITEM_MSG_NUM              20
#define PPITEM_MSG_MAX_SIZE         (PPITEM_MSG_UNIT*PPITEM_MSG_NUM)

#define PPITEM_THREAD_PRIORITY      9
#define PPITEM_THREAD_STACK_SIZE    4096
#define PPITEM_THREAD_TIMESLICE     5
/*
********************************************************************************
********************************************************************************
*/

static struct rt_messagequeue mq;

static char msg_pool[PPITEM_MSG_MAX_SIZE];
#if 0
static rt_device_t  flash_device = RT_NULL;    /* 设备句柄 */
#endif
static struct  rt_thread  g_filetid ;
static FileOpQMq_t filemsg;
static FileOpQMq_t *pmsg=&filemsg;


static rt_uint8_t ppitem_thread_stack[PPITEM_THREAD_STACK_SIZE];
/*
********************************************************************************
********************************************************************************
*/
typedef struct {
   u8_t   *pmem;                                       /* SRAM 地址 */
   u8_t   size;                                        /* 长度 */
   char const *pdef; 
} REG_PPITEM_T;

struct {
    #ifdef PPDEF
    #undef PPDEF
    #endif
    #define PPDEF(name, size,ppnam) \
    char name[size];
    #include "PPdef.h"
}G_PP_RAM;

#define PP_MAX_BUF_LEN   sizeof(G_PP_RAM)

char *const PP_RAM = (char*)&G_PP_RAM;

const REG_PPITEM_T Reg_PPItems[PP_MAX_NUM] = { 
    #ifdef PPDEF
    #undef PPDEF
    #endif
    /* pmem, size,att,pmem,func*/
    #define PPDEF(name, size,ppnam) \
    { (u8_t *)(G_PP_RAM.name),size,ppnam},
    #include "PPdef.h"
};
void (*ppitem_parm_init)(void);
/*
********************************************************************************
                          对下接口函数
********************************************************************************
*/
/*
********************************************************************************
*Function    : __PPItemReadData
*Description : 从文件读数据
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
static u8_t __PPItemReadData(void)
{
    int  fd;
    int  res;

    fd = open(SYS_PARM_PPITEM, O_RDONLY);
    if( fd == -1 ){
        LOG_E("open file error");  
        return -1;
    }
    lseek(fd, 0, SEEK_CUR);
    
    res = read(fd, PP_RAM, PP_MAX_BUF_LEN); 
    
    res = res;
    close(fd); 
    return 0;
}
/*
********************************************************************************
*Function    : __PPItemWriteData
*Description : 写入数据到文件
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
static u8_t __PPItemWriteData(void)
{
    int  fd;
    int res;
 
    fd = open(SYS_PARM_PPITEM, O_CREAT | O_RDWR);
    if( fd == -1 ){
        LOG_E("open file error");  
        return -1;
    }
    res = write(fd, PP_RAM, PP_MAX_BUF_LEN);
    LOG_D("res write %d",res);  
    close(fd);
    return res;
}
/*
********************************************************************************
                               对外输出函数
********************************************************************************
*/
/*
********************************************************************************
*Function    : esp_set_function
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void PPItemSetInit(void (*fn)(void))
{
    ppitem_parm_init = fn;
}
/*
********************************************************************************
*Function    : PPItemGetptr
*Description : 读数据
*Input       : id
*Output      : None
*Return      : pmem
*Others      : None
********************************************************************************
*/
char *PPItemGetptr(u32_t id)
{
    if( id > PP_MAX_NUM){
        return NULL;
    }
    return (char*)Reg_PPItems[id].pmem;
}
/*
********************************************************************************
*Function    : PPItemSize
*Description : 获取SIZE
*Input       : None
*Output      : None
*Return      : size
*Others      : None
********************************************************************************
*/
u8_t PPItemSize(u32_t id)
{
    return Reg_PPItems[id].size;
}
/*
********************************************************************************
*Function    : PPItemRead
*Description : 读数据
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
u32_t PPItemRead(u32_t id, void *ptr,u32_t ulen)
{
    if( id > PP_MAX_NUM){
        return PP_ERR_EMID;
    }
    if( ulen != Reg_PPItems[id].size ){
        return PP_ERR_ULEN;
    }
    memcpy(ptr, Reg_PPItems[id].pmem,Reg_PPItems[id].size);
    return PP_ERR_NONE;
}

/*
********************************************************************************
*Function    : PPItemWrite
*Description : PP写数据
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
u32_t PPItemWrite(u32_t id, void *ptr,u32_t ulen)
{    
    if( id > PP_MAX_NUM){
        return PP_ERR_EMID;
    }
    if(ulen == 0){
        ulen = Reg_PPItems[id].size;
    }
    if( ulen > Reg_PPItems[id].size ){
        return PP_ERR_ULEN;
    }
    if(memcmp(Reg_PPItems[id].pmem,(u8_t*)ptr,Reg_PPItems[id].size)==0){
        return PP_ERR_NONE;
    }
    memcpy(Reg_PPItems[id].pmem,(u8_t*)ptr,Reg_PPItems[id].size);
    
    pmsg->msgtype = PP_MSG_TIMEOUT;
    
    rt_mq_send(&mq, &pmsg, sizeof(void*));
    return PP_ERR_NONE;
}
/*
********************************************************************************
*Function    : PPItemSync
*Description : 初始化
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/ 
void PPItemSync(void)
{
    pmsg->msgtype = PP_MSG_SYNC;
 
    rt_mq_send(&mq, &pmsg, sizeof(void*));
}
/*
********************************************************************************
*Function    : PPItemsFactory
*Description : 恢复出厂化
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/ 
void PPItemsFactory(void)
{
    pmsg->msgtype = PP_MSG_REST_FACTORY;
 
    rt_mq_send(&mq, &pmsg, sizeof(void*));
}
/*
********************************************************************************
*Function    : PPItemsMKFS
*Description : 磁盘格式化
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/ 
void PPItemsMKFS(void)
{
    pmsg->msgtype = PP_MSG_DISK_FORMATT;
 
    rt_mq_send(&mq, &pmsg, sizeof(void*));
}
/*
********************************************************************************
                                
********************************************************************************
*/
/*
********************************************************************************
*Function    : __PPItemsRecovery
*Description : 恢复出厂化
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
static s8_t __PPItemsRecovery(void)
{
    u32_t value;   
    u32_t ulen;
    
    for(int id=0 ; id < PP_MAX_NUM ; id++ ){
        if(Reg_PPItems[id].size < 5 ){
            value =  atoi((const char*)Reg_PPItems[id].pdef);
            memcpy(Reg_PPItems[id].pmem,(u8_t*)&value,Reg_PPItems[id].size);
        }else{
            ulen = MIN(Reg_PPItems[id].size,strlen(Reg_PPItems[id].pdef));
            memset(Reg_PPItems[id].pmem,0,Reg_PPItems[id].size);
            memcpy(Reg_PPItems[id].pmem,(u8_t*)Reg_PPItems[id].pdef,ulen);
             
        }
    }
    
    __PPItemWriteData();
    return 0;       
}
/*
********************************************************************************
*Function    : __PPItemsParmInit
*Description : PP参数初始化
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/
static void __PPItemsParmInit(void)
{
    if( __PPItemReadData()==0){
        if(ppitem_parm_init != NULL ){
            ppitem_parm_init();
        }    
    }else{
        __PPItemsRecovery();
    }
}
/*
********************************************************************************
*Function    : PPItemThread
*Description : 文件管理线程
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/ 
static void PPItemsThread(void *arg)
{    
    FileOpQMq_t *pmsg;
    rt_err_t      result;
    u32_t timeout=RT_WAITING_FOREVER;
   
    for(;;){
          result = rt_mq_recv(&mq, &pmsg, sizeof(void*), timeout);
          if( result >= RT_EOK){
              if(pmsg->msgtype == PP_MSG_SYNC){
                  __PPItemWriteData();
                  rt_kprintf("pp sync\r\n");
              }else if(pmsg->msgtype == PP_MSG_REST_FACTORY){
                  __PPItemsRecovery(); 
                  rt_hw_cpu_reset();
              }else if(pmsg->msgtype == PP_MSG_DISK_FORMATT){   
                  dfs_mkfs("uffs","flash"); 
                  rt_hw_cpu_reset();
              }else if(pmsg->msgtype == PP_MSG_TIMEOUT){
                  timeout=5000;
              }
          }else{
               timeout=RT_WAITING_FOREVER;
               __PPItemWriteData();
               LOG_D("PPItemWriteSyncToFile ");
          }   
    }
}  
/*
********************************************************************************
*Function    : PPItemsInit
*Description : 文件管理线程
*Input       : None
*Output      : None
*Return      : None
*Others      : None
********************************************************************************
*/ 
int PPItemsInit(void)
{
    rt_err_t result;
    
    result = rt_mq_init(&mq, "file_mq",&msg_pool[0], sizeof(void *),sizeof(msg_pool), RT_IPC_FLAG_FIFO);
    if (result != RT_EOK){
         LOG_D("init static msg queue failed.\n");
         return result;
    }
    __PPItemsParmInit();
    
    result = rt_thread_init(&g_filetid,
                             "pparm",
                             PPItemsThread, 
                             (void*)1, 
                             &ppitem_thread_stack[0], 
                             PPITEM_THREAD_STACK_SIZE, 
                             PPITEM_THREAD_PRIORITY, 
                             PPITEM_THREAD_TIMESLICE);
    if (result == RT_EOK){
        rt_thread_startup(&g_filetid);
    }else{
        return result;
    }
    return RT_EOK;
}
INIT_APP_EXPORT(PPItemsInit);
