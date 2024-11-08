/*
 *  msh_system.c
 *
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
#include "app_lib.h" 

#define DBG_TAG "tcp"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
struct client {
    rt_device_t device;
    rt_sem_t  rx_sem;
    u32_t enable;
}client;
#define COMM_BUF_LEN               1024
static rt_device_t console;
    
static rt_uint8_t     CommRxBuf[COMM_BUF_LEN];
static int console_send_lowlayer(u8_t *msg, u32_t msglen, void *arg);
/*
********************************************************************************
********************************************************************************
*/
/*
******************************************************************************** 
*Function    : console_send_lowlayer
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
static int console_send_lowlayer(u8_t *msg, u32_t msglen, void *arg)
{
   rt_device_write(console, 0, msg, msglen);
   return 0;
}
/*
******************************************************************************** 
*Function    : console_rx_ind
*Description : AT命令
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
static rt_err_t console_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(client.rx_sem);

    return RT_EOK;
}
 /*
******************************************************************************** 
*Function    : console_exit
*Description :  
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
void console_exit( void )
{
    client.enable = 0;

}
/*
********************************************************************************
Function      : show_usage
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static void comm(int argc, char **argv)
{
    struct rt_semaphore console_rx_notice;
    rt_err_t (*old_rx_ind)(rt_device_t dev, rt_size_t size);

    rt_base_t int_lvl;
    rt_uint16_t old_open_flag;
    char ch;   

    if (argc != 2 || strcmp(argv[1], "client") != 0) {
        rt_kprintf("Please input '<client>' \n");
        return;
    }

    rt_sem_init(&console_rx_notice, "rx_noti", 0, RT_IPC_FLAG_FIFO);
    client.rx_sem = &console_rx_notice;

    //backup console configure
    int_lvl = rt_hw_interrupt_disable();
    console = rt_console_get_device();
    if (console) {
        old_open_flag = console->open_flag;
        console->open_flag &= (~RT_DEVICE_FLAG_STREAM);
        /* backup RX indicate */
        old_rx_ind = console->rx_indicate;
        rt_device_set_rx_indicate(console, console_rx_ind);
    }
    rt_hw_interrupt_enable(int_lvl);

    //entry to at mode
    client.enable = 1;



    while (1) {
        while (rt_device_read(console, 0, &ch, 1) == 0) {
             rt_sem_take(client.rx_sem, RT_WAITING_FOREVER);
        }
   
        if (client.enable == 0) {
            break;
        }
    }

    //exit at mode
    client.enable = 0;

    //restore console configure
    int_lvl = rt_hw_interrupt_disable();
    if (console) {
        console->open_flag = old_open_flag;
        /* restore RX indicate */
        if (old_rx_ind) {
            rt_device_set_rx_indicate(console, old_rx_ind);
        }
    }
    rt_hw_interrupt_enable(int_lvl);

    client.rx_sem = RT_NULL;
    rt_sem_detach(&console_rx_notice);
}

FINSH_FUNCTION_EXPORT(comm, comm <client>)
MSH_CMD_EXPORT(comm, RT-Thread comm component cli: comm <client>);
