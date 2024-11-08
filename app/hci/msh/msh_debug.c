
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
extern int https_send(const char *pmsg, int ulen);
extern s32_t blue_package(u8_t *buff,u8_t buflen,u8_t flg,u8_t cmdid,u8_t *msg,u8_t msglen);
/*
********************************************************************************
********************************************************************************
*/ 
#if 0

static profile_info_t g_profile_info[20];
/*
********************************************************************************
Function      : esim
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static int esim(int argc, char** argv)
{
    //char buf[] = "BF2D820141A082013DE34D5A0A984405703091167803F14F10A0000005591010FFFFFFFF89000011009F700100910C6368696E6120756E69636F6D92036368699301009410202122232425262728292A2B2C2D2E2F950101E33B5A0A986840605122084174214F10A0000005591010FFFFFFFF89000012009F700101910447414741920E7265647465615F70726F66696C65950102E33B5A0A986811222204131550F54F10A0000005591010FFFFFFFF89000014009F700100910447414741920E7265647465615F70726F66696C65950102E3445A0A986840225122083157054F10A0000005591010FFFFFFFF89000013009F70010091023133921972656474656174657374345F31363838393534373833313731950102E32C5A0A986840705122082199664F10A0000005591010FFFFFFFF89000015009F700100920534363030389501029000";
    char *buf= 0;
    char hex_buf[1024] = { 0 };
    int i;
    char num = 0;
    int size;


    buf = argv[1];
    
    for (i = 0; i < sizeof(buf)/2; i++) {
        sscanf(buf + i * 2, "%2hhx", &hex_buf[i]);
    }

    //for (i = 0; i < sizeof(buf)/2; i++) {
    //    printf("0x%02x ", hex_buf[i]);
    //}
    //printf("\n");
    
    rt_lpa_get_profile_info(hex_buf, g_profile_info, &num, 20);
    for (i = 0; i <num; i++) {
        rt_kprintf("iccid: %s, type: %d, state: %d\r\n", g_profile_info[i].iccid, g_profile_info[i].type, g_profile_info[i].state);//: type: %d, state: %d, ICCID: %s ii,
    }
    return 0;
}
#endif
/*
********************************************************************************
*Function    :   ulog_float_text 
*Description :   
*Input       :
*Output      :finsh
*Return      :components
*Others      :rt-thread[5.0.1]
********************************************************************************
*/
static int ulog_float(int argc, char** argv)
{
    float data;
    data=35.26;
    

    LOG_D("%f",data);
    rt_kprintf("%f",data);
    rt_kprintf("%d",rt_rng_read());
    
    time_t seconds;
 
    seconds = time((time_t *)NULL);
    rt_kprintf("%d\n", seconds);
    
    return 0;
}


/*
********************************************************************************
Function      : timeset
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void timeset(void)
{
    u8_t timestr[]="2023-08-3 11-45-00";
    int res;
    rt_uint32_t year,  month,  day;
    rt_uint32_t hour,  minute,  second;
    
    
    res =sscanf(timestr,"%d-%d-%d %d-%d-%d",&year,&month,&day,&hour,&minute,&second);
         
         
    set_date(year, month, day);
        
    set_time(hour, minute, second);
}
/*
********************************************************************************
Function      : debug
Description   :   A5 04 00 00 12 01 17 5A
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void debug(int argc, char** argv)
{
    if (argc < 2 ) {

        return;
    }

    
    if(strcmp(argv[1], "float") == 0) {
        ulog_float(argc,argv);
    }else if(strcmp(argv[1], "esim") == 0) {
      //  esim(argc,argv);
    }
}   
MSH_CMD_EXPORT(debug,debug); 