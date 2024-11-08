/*
 * uart_client.c
 *
 *  Created on: 2023年7月17日
 *      Author: lwp
 */
#include <board.h>                                                     
#include <rtthread.h>                                                  
#include "app_lib.h"                                                   
                                                                       
                                                                          
/*
********************************************************************************
********************************************************************************
*/ 
#ifndef UART_DATA_FLAG                                                 
#define UART_DATA_FLAG   0x7e
#endif
#define UART_RX_EVENT (1 << 0)               /* 串口接收事件标志 */

#define COMM_BUF_LEN               1024

#define COMM_THREAD_PRIORITY        7
#define COMM_THREAD_STACK_SIZE     1024
#define COMM_THREAD_TIMESLICE       5

                                                                          
/*
********************************************************************************
********************************************************************************
*/ 
static rt_uint8_t CommRxBuf[COMM_BUF_LEN];
static struct  rt_thread    g_comm;
static struct  rt_semaphore op_compelete_sem;
static struct  rt_event     event;                /* 事件控制块 */
static rt_device_t          uart_device = RT_NULL;    /* 设备句柄 */

static rt_uint8_t comm_thread_stack[COMM_THREAD_STACK_SIZE];

                                                                          
/*
********************************************************************************
********************************************************************************
*/ 
/*                                                                        
********************************************************************************
*Function    : comm_sendstr                                      
*Description :                                                            
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
 
void comm_sendstr(rt_uint8_t *pstr,rt_uint32_t ulen)
{
    rt_device_write(uart_device, 0, pstr, ulen );

}
/*                                                                        
********************************************************************************
*Function    : comm_send_lowlayer                                      
*Description :                                                            
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
static int comm_send_lowlayer(u8_t *msg, u32_t msglen, void *arg)
{
    (void)arg;

    comm_sendstr((rt_uint8_t *)msg, msglen);

    return 0;
}
/*                                                                        
********************************************************************************
*Function    : uart_notification                                      
*Description :                                                            
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
static rt_err_t uart_notification(rt_device_t dev, rt_size_t size)
{
    rt_event_send(&event, UART_RX_EVENT);    
    return RT_EOK;
}
/*                                                                        
********************************************************************************
*Function    : uart_open                                      
*Description : 回调函数                                                          
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
static rt_err_t uart_open(const char *name)
{
    rt_err_t res;
 
    uart_device = rt_device_find(name);
    if (uart_device != RT_NULL){
        res = rt_event_init(&event, "event", RT_IPC_FLAG_FIFO);
        if (res != RT_EOK){
            rt_kprintf("init static semaphore failed.\n");
            return -1;
        } 
        res = rt_device_set_rx_indicate(uart_device, uart_notification);
        if (res != RT_EOK){
            rt_kprintf("set %s rx indicate error.%d\n",name,res);
            return -RT_ERROR;
        }
        res = rt_device_open(uart_device, RT_DEVICE_OFLAG_RDWR |
                             RT_DEVICE_FLAG_INT_RX );
        if (res != RT_EOK){
            rt_kprintf("open %s device error.%d\n",name,res);
            return -RT_ERROR;
        }
    }else{
        rt_kprintf("can't find %s device.\n",name);
        return -RT_ERROR;
    }
    
    return RT_EOK;
} 
/*                                                                        
********************************************************************************
*Function    : uart_thread                                      
*Description : 数据接收过滤任务                                                          
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
void uart_thread(void *pdata)
{
    rt_uint32_t e;
    unsigned char   data;
    rt_err_t res;
   
   comm_sendstr((rt_uint8_t *)"AT\r\n", 4);
    while(1){
       res = rt_event_recv(&event, UART_RX_EVENT,RT_EVENT_FLAG_AND |
                               RT_EVENT_FLAG_CLEAR,5000, &e);  /* 接收事件 */
       if( res == RT_EOK ){
           while ( rt_device_read(uart_device, 0, &data, 1) == 1) {
             //  lan_msg_recv(lan_msg, data);
               rt_kprintf("[%c] [%d]",data,data);
               //rt_device_write(uart_device, 0, &data, 1 );
             
           }
           
  
       }else{
           comm_sendstr((rt_uint8_t *)"AT\r\n", 4);
       }
    }
}
/*                                                                        
********************************************************************************
*Function    : uart_client_init                                      
*Description : 数据接收过滤任务                                                          
*Input       :                                                            
*Output      :                                                            
*Return      :                                                            
*Others      :                                                            
********************************************************************************
*/
int uart_client_init(void)
{ 
    rt_err_t result;
     
    uart_open("lpuart1");
   
    result = rt_sem_init(&op_compelete_sem, "op_compelete_sem", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK){
        rt_kprintf("init static semaphore failed.\n");
        return -1;
    }  
 
    result = rt_thread_init(&g_comm,
                            "pccli",
                            uart_thread, 
                            (void*)1, 
                            &comm_thread_stack[0], 
                            COMM_THREAD_STACK_SIZE, 
                            COMM_THREAD_PRIORITY, 
                            COMM_THREAD_TIMESLICE);
    if (result ==  RT_EOK){
        rt_thread_startup(&g_comm);
    }else{
        return -1;
    }
    return 0;
}
//INIT_APP_EXPORT(uart_client_init);
