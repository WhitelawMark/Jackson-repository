/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-04-16     RT-Thread    first version
 */

#include <rtthread.h>
#include <board.h>
#include <drv_common.h>
#include "app_lib.h"  
   
 
   
#ifdef RT_USING_DFS
#include <dfs_fs.h>
//#include <dfs_posix.h>
#endif

#define DBG_TAG "misc"
#define DBG_LVL DBG_LOG   
#include <rtdbg.h>

/*
********************************************************************************
********************************************************************************
cmd /c "python $PROJ_DIR$\..\tools\create_image.py $PROJ_DIR$\..\ Debug output\bootload.bin $PROJ_FNAME$ $CONFIG_NAME$"
*/
#ifdef  RT_USING_DFS
/*
********************************************************************************
*Function    :   dfs_mount_init 
*Description :   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
int dfs_mount_init(void)
{
    if (dfs_mount("flash", "/", "uffs", 0, 0) == 0){
        LOG_I("env: dfs_mount_init, successful");
    } else {
        LOG_E("env: dfs_mount_init, failed");
    }
    return 0;
}
INIT_ENV_EXPORT(dfs_mount_init);
#endif
/*
********************************************************************************
Function      : app_init
Description   : 系统初始化 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
#if 0
#define ESP_EN_3V    GET_PIN(B,4)
void esp_power_3v(void)
{
    rt_pin_mode(ESP_EN_3V, PIN_MODE_OUTPUT);
    rt_pin_write(ESP_EN_3V, PIN_HIGH);
    rt_thread_mdelay(500);
    rt_kprintf("esp_power_3v=%d \r\n",ESP_EN_3V);

}
#endif
int app_init(void)
{
    // esp_power_3v();
    app_get_chip_id();
    
    app_softvs_init();
    
    app_parm_init();
    
    app_ulog_init();

    app_version();
    
#ifdef SYSWATCH_USING
    syswatch_setrecallback(PPItemSync);
#endif
    
    upgrade_init();
    
    return 0;
}
/*
********************************************************************************
*Function    :   main 
*Descriptio  :   
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/ 
int main(void)
{
    app_init();


    return RT_EOK;
}
