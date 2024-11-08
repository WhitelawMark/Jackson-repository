/*
 *  show.c
 *
 *  Created on: 2019Äê5ÔÂ9ÈÕ
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
 
#include "app_lib.h"


#define DBG_TAG "misc"
#define DBG_LVL DBG_LOG   
#include <rtdbg.h>

#ifdef RT_USING_PM
static rt_uint8_t run_mode = PM_RUN_MODE_NORMAL_SPEED;
/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
*Function    : mode_loop
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static rt_uint8_t mode_loop(void)
{
     rt_uint8_t mode = 1;
     
     run_mode++;
     switch (run_mode)
     {
       case 0:
       case 1:
       case 2:
       case 3:
           mode = run_mode;
         break;
       case 4:
          mode = 2;
          break;
       case 5:
          mode = 1;
          break;
       case 6:
          mode = run_mode = 0;
          break;
     }
    return mode;
}
/*
********************************************************************************
*Function    : pm_loop
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void pm_loop(void)
{
   int count=20;
  
    while (count--){
        rt_pm_request(PM_SLEEP_MODE_LIGHT);
        
        rt_thread_mdelay(1000);
        
        rt_pm_run_enter(mode_loop());
        
        rt_pm_release(PM_SLEEP_MODE_NONE);
    }
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
static void show_usage(void)
{
    rt_kprintf("Please input '<help|loop|run|sleep>' \n"); 
    rt_kprintf("             'pm <run> <high|normal|medium|low>' \n"); 
    rt_kprintf("             'pm <sleep>'<none|light|deep|standby|shutdown>\n"); 
}
/*
********************************************************************************
*Function    : pm
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void pm(int argc, char** argv)
{
    if (argc < 2 ) {
        show_usage();
        return;
    }
  
    if(strcmp(argv[1], "loop") == 0) {
        pm_loop();
    }else if(strcmp(argv[1], "run") == 0) { 
        if(strcmp(argv[2], "high") == 0) { 
             rt_pm_run_enter(PM_RUN_MODE_HIGH_SPEED);
        }else if(strcmp(argv[2], "normal") == 0) {  
             rt_pm_run_enter(PM_RUN_MODE_NORMAL_SPEED);
        }else if(strcmp(argv[2], "medium") == 0) {    
             rt_pm_run_enter(PM_RUN_MODE_MEDIUM_SPEED);
        }else if(strcmp(argv[2], "low") == 0) {    
             rt_pm_run_enter(PM_RUN_MODE_LOW_SPEED );
        }
    }else if(strcmp(argv[1], "sleep") == 0) { 
        if(strcmp(argv[2], "none") == 0) {    
             rt_pm_release(PM_SLEEP_MODE_NONE);
        }else if(strcmp(argv[2], "light") == 0) {   
             rt_pm_request(PM_SLEEP_MODE_LIGHT);
        }else if(strcmp(argv[2], "deep") == 0) {   
             rt_pm_request(PM_SLEEP_MODE_DEEP);
        }else if(strcmp(argv[2], "standby") == 0) {  
             rt_pm_request(PM_SLEEP_MODE_STANDBY);
        }else if(strcmp(argv[2], "shutdown") == 0) {   
             rt_pm_request(PM_SLEEP_MODE_SHUTDOWN);
        }
    }else{
        show_usage();
    }
}
MSH_CMD_EXPORT(pm,pm <help|loop|run|sleep> );
#endif
/*RT_USING_PM*/

  