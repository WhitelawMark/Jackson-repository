
/*
 * lan_port.c
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

/*
********************************************************************************
********************************************************************************
*/

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
Function      : esp_cmd
Description   :   A5 04 00 00 12 01 17 5A
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void esp_cmd(int argc, char** argv)
{
    if (argc < 2 ) {
        show_usage();
        return;
    }

    if(strcmp(argv[1], "bleinit") == 0) {
        esp_btser_init();
    }else if(strcmp(argv[1], "mqtt") == 0) {
        if(strcmp(argv[2], "pub") == 0) {
            esp_mqtt_send("hellword",strlen("hellword"));
        }
        
    }else if(strcmp(argv[1], "http") == 0) {
        if(strcmp(argv[2], "download") == 0) {
            https_download("http://ir.hongwaimaku.com/vipdownloadpar.php?kfid=020677&mac=ad993deffed32786&tokens=biPv8NNfefYc0c5f4724aY1d1aP2Ao4Y","/irdata.bin");
        }else if(strcmp(argv[2], "start") == 0) {
            https_service_init();
        }
    }
}   
MSH_CMD_EXPORT(esp_cmd,esp_cmd); 