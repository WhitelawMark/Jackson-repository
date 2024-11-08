
/*
 *  msh_system.c
 *
 *  Created on: 2019Äê5ÔÂ9ÈÕ
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
#include "app_lib.h" 


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
    rt_kprintf("Please input '<ota>' \n");

}
/*
********************************************************************************
Function      : ctrl
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
void ctrl(int argc, char** argv)
{
    if (argc < 2 ) {
        show_usage();
        return;
    }
    if(strcmp(argv[1], "mkfs") == 0) {
        dfs_mkfs("uffs","flash"); 
    }else{
        show_usage();
    }
}
MSH_CMD_EXPORT(ctrl,ctrl  <>);