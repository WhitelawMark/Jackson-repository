
/*
 * msh_config.c
 *
 *  Created on: 2019年5月9日
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
     rt_kprintf("Please input '<factory|heart|worktime >' \n"); 
}
/*
********************************************************************************
Function      : config
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void config(int argc, char** argv)
{
    u32_t value;
  
    if (argc < 2 ) {
        show_usage();
        return;
    }
    if(strcmp(argv[1], "mode") == 0) {
         value = atoi(argv[2]); 
         if(value > 2 ){
             rt_kprintf("The value is (0~2)\n");
             return ;
         }
    }else if(strcmp(argv[1], "factory") == 0) {
         PPItemsFactory();
    }else{
        show_usage();
    }
}
MSH_CMD_EXPORT(config,config <mode|tick|time|...> );


