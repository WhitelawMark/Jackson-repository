/*
 *  app_gpio.c
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


/*
********************************************************************************
********************************************************************************
*/

#define GPIO_DEF(x,y)  #x,x,y
/*
********************************************************************************
********************************************************************************
*/

/* 
 * 输入IO口列表 
 */
const gpio_t GpioInList[] = {
    {GPIO_DEF(LED_SYS_PIN  , PIN_MODE_INPUT_PULLUP)},

};

/* 
 * 输出IO口列表 
 */
const gpio_t GpioOutList[] = {
    {GPIO_DEF(LED_SYS_PIN,            PIN_MODE_OUTPUT)},

}; 
/*
********************************************************************************
********************************************************************************
*/
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
static void show_usage(void)
{
    int size;
    
    rt_kprintf("gpio write <pin> <0|1>\r\n");
    rt_kprintf("gpio read <pin>\r\n");
    rt_kprintf("gpio mode <pin> <0-4>, 0-Output;1-Input;2-InputPullup;3-InputPulldown;4-OutputOd\r\n");
    rt_kprintf("gpio read <pin>\r\n");
        

    rt_kprintf("gpio input list\r\n");
    rt_kprintf("==========================================\r\n");
    size = sizeof(GpioInList) / sizeof(GpioInList[0]);
    for (int i = 0; i < size; i++) {
        rt_kprintf("PIN:[%02d](%17s)\r\n",GpioInList[i].num,GpioInList[i].name);
    }
    rt_kprintf("==========================================\r\n");

    rt_kprintf("gpio output list\r\n");
    rt_kprintf("==========================================\r\n");
    size = sizeof(GpioOutList) / sizeof(GpioOutList[0]);
    for (int i = 0; i < size; i++) {
        rt_kprintf("PIN:[%02d](%17s)\r\n",GpioOutList[i].num,GpioOutList[i].name);
    }
    rt_kprintf("==========================================\r\n");
}
/*
********************************************************************************
Function      : gpio_cmd
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int gpio_cmd(int argc, char **argv)
{
    u8_t pinnum;
    u8_t regval;
    u8_t dohelp = 0;

    if (argc < 3) {
        dohelp = 1;
        goto out;
    }

    if (strstr(argv[1], "set") || strstr(argv[1], "write")) {
        if (argc < 4) {
            dohelp = 1;
            goto out;
        }

        pinnum = atoi(argv[2]);
        regval = atoi(argv[3]);
        rt_pin_write(pinnum, regval);
        rt_kprintf("gpio %s pin_%d: %d\r\n", argv[1], pinnum, regval);
    } else if (strstr(argv[1], "get") || strstr(argv[1], "read")) {
        pinnum = atoi(argv[2]);
        regval = rt_pin_read(pinnum);
        rt_kprintf("gpio %s pin_%d: %d\r\n", argv[1], pinnum, regval);
    } else if (strstr(argv[1], "mode")) {
        rt_base_t mode;
        if (argc < 4) {
            dohelp = 1;
            goto out;
        }
        pinnum = atoi(argv[2]);
        mode = atoi(argv[3]);
        
        rt_pin_mode(pinnum, mode);
        rt_kprintf("gpio %s pin_%d, mode: %d\r\n", argv[1], pinnum, mode);
    } else {
        dohelp = 1;
        goto out;
    }

out:
    if (dohelp) {
        show_usage();
    }

    return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(gpio_cmd, __cmd_gpio, set and get gpio pin.);
/*
********************************************************************************
Function      : app_gpio_init
Description   : GPIO初始化
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int app_gpio_init(void)
{
    u16_t i;
    int insize;
    int outsize;
    
  //  gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);
    
    insize = sizeof(GpioInList) / sizeof(GpioInList[0]);
    outsize = sizeof(GpioOutList) / sizeof(GpioOutList[0]);

    /* 输入IO口配置 */
    for (i = 0; i < insize; i++) {
        /* 配置引脚为输入 */
        rt_pin_mode(GpioInList[i].num, GpioInList[i].mode);
    }

    /* 输出IO口配置 */
    for (i = 0; i < outsize; i++) {
        rt_pin_mode(GpioOutList[i].num, GpioOutList[i].mode);
    }
    
 
    rt_kprintf("app_gpio_init.\r\n");
 
    return 0;
}
INIT_DEVICE_EXPORT(app_gpio_init); 