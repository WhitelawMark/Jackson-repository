 /*
 * main.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */


#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include "app_lib.h"

/*
********************************************************************************
********************************************************************************
*/ 
static pFunction Jump_To_Application;
static uint32_t JumpAddress;

#pragma location = 0x08001000
__root const char bootvs[4]=SYS_SOFT_VS;

/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  PowerOnMsg
**  功能描述:  打印版本信息 
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void PowerOnMsg(void)
{
    int vsid=0;
    
    vsid = atoi(bootvs);
    kprintf("**************************************************#\r\n");
    kprintf("%s %s\r\n", SYS_CMPY_NAME,SYS_SOFE_FACT);
    kprintf("Software version: V%d.%d.%d \r\n", vsid/100,(vsid/10)%10,vsid%10);
    kprintf("Created time    : %s \r\n", SYS_CREATE_CODE_TIME);     
    kprintf("Flash ID        : %x \r\n", FlashReadID());    
    kprintf("FLASH_SIZE : 0X%x \r\n", 2*FLASH_BANK_SIZE);    
}
/*
********************************************************************************
**  函数名称:  Bl_Errorflg
**  功能描述:  记录当前错误标识
**  输入参数:  当前错误标记
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void Bl_Errorflg(unsigned char err)
{
    FlashWriteByte(FLASH_ERROR_FLG,err); 
}
/*
********************************************************************************
**  函数名称:  JumpToApp
**  功能描述:  跳转到应用程序
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void JumpToApp(void)
{
   // drv_wdt_init();
    
    bl_log(LG_INF,"Jump to the application");

    delay_1ms(500);


    if ((*(__IO uint32_t*)(APPLICATION_ADDRESS + APPLICATION_HEAD_SIZE) & 0x2FFE0000 ) == 0x20000000){ 
        JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + APPLICATION_HEAD_SIZE + 4);
        Jump_To_Application = (pFunction) JumpAddress;
        __set_MSP(*(__IO uint32_t*) (APPLICATION_ADDRESS + APPLICATION_HEAD_SIZE));
              
        Jump_To_Application();
    }

    bl_log(LG_INF, "Application exception");
}

/*
********************************************************************************
**  函数名称:  BoardInit
**  功能描述:  板级初始化
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void BoardInit(void)
{
    HAL_Init();
      
    SystemClock_Config();
    
    Icache_Init();
 
    drv_uart_init();
    
    drv_flash_init();
   
    syslogsetlevel(LG_DMP);
    
    PowerOnMsg();
}
/*
********************************************************************************
**  函数名称:  key_scan
**  功能描述:   
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static int key_scan(void)
{
    static int sec=200;
    int cnt;
    char c;
    int cursec;
    /*电脑串口输入D */
    for (cnt = 100; cnt > 0; cnt--) {
        if (DrvUartGetChar((u8_t*)&c)) {
            if(c == 'd'){
                return 1;
            }
        }
        
        cursec =  cnt/40;
        if( sec != cursec ){
            kprintf("%d",cursec+1);/*321打印*/
            sec = cursec;
        }
        delay_1ms(50);
    }
    kprintf("\n");
    return 0;
}
/*
********************************************************************************
**  函数名称:  Bl_LocalCodeUpdata
**  功能描述:  使用终端进行升级，单板升级
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void Bl_LocalCodeUpdata(void)
{
    if (key_scan() == 1) {
         Main_Menu();
    }
}
typedef void (*app_func_t)(void);
 void qbt_jump_to_app(void)
{
    // typedef void (*app_func_t)(void);
    uint32_t app_addr =(uint32_t)0x8020200;
    uint32_t stk_addr = *((__IO uint32_t *)app_addr);
    app_func_t app_func = (app_func_t)(*((__IO uint32_t *)(app_addr + 4)));
   bl_log(LG_INF,"Jump to app rong");
    if ((((uint32_t)app_func & 0xff000000) != 0x08000000) || ((stk_addr & 0x2ff00000) != 0x20000000))
    {
//        LOG_E("No legitimate application.");
//        return;
    }
//    rt_kprintf("Jump to application running ... \n");
//    rt_thread_mdelay(200);
//    LOG_E("Jump ----\n");
    __set_MSP(stk_addr);

    __disable_irq();
    // LOG_E("Jump ----13\n");
    for (int i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;/*Interrupt Clear Enable Registers*/
        NVIC->ICPR[i] = 0xFFFFFFFF;/*Interrupt Set Pending Registers*/
    }
/*    for(int i=0; i<124; i++)
    {
        HAL_NVIC_DisableIRQ(i);
        HAL_NVIC_ClearPendingIRQ(i);
    }*/
//     rt_kprintf("666666688 \n");
     HAL_DeInit();
    // HAL_RCC_DeInit();
    
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;


    
    // __set_CONTROL(0);
    // __set_MSP(stk_addr);
    // rt_kprintf("8888995 \n");
    app_func();//Jump to application running
    
//    LOG_E("Qboot jump to application fail.");
}

/*
********************************************************************************
**  函数名称:  BoardInit
**  功能描述:  板级初始化
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
****************3****************************************************************
*/
int main(void)
{
    BoardInit();

    //本地下载程序
    Bl_LocalCodeUpdata();

    //代码恢复
    Bl_UpdataCodeRecovery();

    //升级代码
    Bl_CodeUPData_Deal();

    //跳转应用程序
    qbt_jump_to_app();
//    JumpToApp();

    while (1){
        delay_1ms(50);
    }
}