
/*
 * lan_port.c
 *
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
#include "app_lib.h"
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/
typedef enum {
    #define LOGDEF(eid, name,def) \
    eid,
    #include "ulog_lvl.h"
    ULOG_MAX_NUM
}ULOG_E;

static struct {
    char   *name;
    int    def;
} ulog_table[]={
    #ifdef LOGDEF
    #undef LOGDEF
    #endif
    #define LOGDEF(eid, name,def) \
    name,def,
    #include "ulog_lvl.h"
};

/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
Function      : app_ulog_name
Description   : 系统日志设置 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
static char *app_ulog_name(int eid)
{
    if(eid < ULOG_MAX_NUM){
        return ulog_table[eid].name;
    }
    return 0;
}

/*
********************************************************************************
Function      : app_ulog_init
Description   : 系统日志初始化 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void app_ulog_init(void)
{
    u8_t ulog_lvl[36];

    PPItemRead(PP_ULOG_LVL, &ulog_lvl,36);

    for(int i=0;i< ULOG_MAX_NUM;i++){
        if(ulog_lvl[i] <3 || ulog_lvl[i] > 7 ){
            ulog_lvl[i] = LOG_LVL_WARNING;
        }
        if( i == 0 ){
            ulog_global_filter_lvl_set(ulog_lvl[i]);
        }else{
            ulog_tag_lvl_filter_set(ulog_table[i].name,ulog_lvl[i]);
        }
    }
    ulog_tag_lvl_filter_set("itte",LOG_LVL_DBG);
}
/*
********************************************************************************
Function      : app_ulog_set
Description   : 系统日志设置 
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/ 
static int app_ulog_set(char *name , int lvl)
{
    u8_t ulog_lvl[36];
    
    PPItemRead(PP_ULOG_LVL, ulog_lvl,36);
    
    if( lvl < 3 || lvl > 7 ){
        lvl = LOG_LVL_WARNING;
        return -1;
    }
    for(int i=0;i< ULOG_MAX_NUM;i++){
        if(strcmp( ulog_table[i].name, name) == 0) {
            ulog_lvl[i] = lvl;
            if( i == LOGLVL_ALL ){
                ulog_global_filter_lvl_set(ulog_lvl[i]);
            }else{
                ulog_tag_lvl_filter_set(name,lvl);
            }
            
            PPItemWrite(PP_ULOG_LVL, ulog_lvl,36);
            return 0;
        }
    }
       
    return -1;
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
    rt_kprintf("\nPlease input: name <level> \n\n");
    
    rt_kprintf("<name>:<");
    for(int i=0;i< ULOG_MAX_NUM;i++){
        rt_kprintf(" %s ",app_ulog_name(i));
        if(app_ulog_name(i+1)){
            rt_kprintf("|");
        }else{
            rt_kprintf(">");
        }
    }
    rt_kprintf("\n \n");
    rt_kprintf("<level> :    \n");
    rt_kprintf("          Error   : 3  \n");
    rt_kprintf("          Warning : 4  \n");
    rt_kprintf("          Info    : 6  \n");
    rt_kprintf("          Debug   : 7  \n");    
}
/*
********************************************************************************
Function      : ulog
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
static void ulog(int argc, char** argv)
{
    u32_t lvl;
  
    if (argc < 3 ) {
        show_usage();
        return;
    }
    
    lvl = atoi(argv[2]); 
    if( lvl < 3 || lvl > 7){
        show_usage();
        return;
    }
    for(int i=0;i<ULOG_MAX_NUM;i++){
        if(strcmp(argv[1], app_ulog_name(i) ) == 0) {
             app_ulog_set(argv[1] ,  lvl);
             return ;
        }
    }
    show_usage();
}
MSH_CMD_EXPORT(ulog,Control system log output);