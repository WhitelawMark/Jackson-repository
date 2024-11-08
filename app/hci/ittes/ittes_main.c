/*
 *  show.c
 *
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <board.h>
#include <rtthread.h>
#include "app_lib.h"


#define DBG_TAG "itte"
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
#define DEF_ITTESCMD_FN(fn)  static int fn(int argc, char** argv)
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
*Function    :  ITTECmd0001 
*Description :  进入厂测模式
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0001)
{
    u8_t factoryflg;
    
    if (argc < 2 ) {
        rt_kprintf("Please enter valid parameters \n"); 
        return 0;
    }
    
    factoryflg = atoi(argv[2]);
    if(factoryflg){
        LOG_D("[%s]:itte enter facory mode ",__func__); 
    }else{
        LOG_D("[%s]:itte exit facory mode",__func__); 
    }
    PPItemWrite(PP_ITTE_MODE, (u8_t*)&factoryflg,  PPItemSize(PP_ITTE_MODE));   
        
    rt_kprintf("OK\n",__func__); 

    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0002 
*Description :  产品序列号
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0002)
{
    u8_t snstr[64];
    u32_t ulen;
    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    PPItemRead(PP_PRD_PSN, snstr,  PPItemSize(PP_PRD_PSN));  
    
    ulen = strlen(snstr) ;
    
    if( ulen != PPItemSize(PP_PRD_PSN) ){
        PPItemWrite(PP_PRD_PSN, snstr, ulen );  
    }else{
        rt_kprintf("ERROR\n"); 
    }
    rt_kprintf("OK\n"); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0003
*Description :  设备编号
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0003)
{
    u8_t snstr[64];
    u32_t ulen;
    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    PPItemRead(PP_DEVICE_ID, snstr,  PPItemSize(PP_DEVICE_ID));  
    
    ulen = strlen(snstr) ;
    
    if( ulen != PPItemSize(PP_DEVICE_ID) ){
        PPItemWrite(PP_DEVICE_ID, snstr, ulen );  
    } 
    rt_kprintf("OK\n"); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0004
*Description :  版本信息
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0004)
{
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
  
    rt_kprintf("hw_version %s \n",SYS_BOARD_VERSION); 
    rt_kprintf("soft_version %s \n",SYS_SOFE_VERSION); 
    rt_kprintf("pub_time %s \n",SYS_CREATE_CODE_TIME); 
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0005
*Description :  模组ICCID查询
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0005)
{    
    u32_t cmdtype;
    u8_t  iccid[64]={0};

    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    cmdtype = atoi(argv[2]);
    if(cmdtype==0){
        PPItemRead(PP_ESIM_CCID0,iccid,PPItemSize(PP_ESIM_CCID0));
        rt_kprintf("+ICCID0:%s\n",iccid);
    }else if(cmdtype==1){
        PPItemRead(PP_ESIM_CCID1,iccid,PPItemSize(PP_ESIM_CCID1));
        rt_kprintf("+ICCID1:%s\n",iccid);
    }else if(cmdtype==2){
        PPItemRead(PP_ESIM_CCID2,iccid,PPItemSize(PP_ESIM_CCID2));
        rt_kprintf("+ICCID2:%s\n",iccid);
    }
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0012
*Description :   MQTT 设置
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0012)
{
    u32_t cmdtype;
    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    cmdtype = atoi(argv[2]);
    if(cmdtype==0){
        PPItemWrite(PP_MQT_URL,argv[3],PPItemSize(PP_MQT_URL));
        rt_kprintf("+URL: %s \n",argv[3]);
    }else if(cmdtype==1){
        u32_t port;
        
        PPItemWrite(PP_MQT_PORT,&port,PPItemSize(PP_MQT_PORT));
        rt_kprintf("+PORT: %d \n",port);
    }
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0014
*Description :  证书设置
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0014)
{
    u32_t cmdtype;
    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    cmdtype = atoi(argv[2]);
    if(cmdtype==0){

    }else if(cmdtype==1){
      
    }else if(cmdtype==2){
      
    }
    
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0015
*Description :  网络
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0015)
{
    u32_t cmdtype;
    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    cmdtype = atoi(argv[2]);
    if(cmdtype==0){

    }else if(cmdtype==1){
        rt_kprintf("+CSQ:%d\n",30); 
    }
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0016
*Description :  网络切换　
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0016)
{
    u32_t cmdtype;
    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    cmdtype = atoi(argv[2]);
    if(cmdtype==0){

    }else if(cmdtype==1){
    
    }
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0017
*Description :  BLE配置　
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0017)
{
    u32_t cmdtype;

    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    cmdtype = atoi(argv[2]);
    if(cmdtype==0){

    }else if(cmdtype==1){
    
    }
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0018
*Description :  WIFI配置　
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0018)
{
    u32_t cmdtype;

    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    cmdtype = atoi(argv[2]);
    if(cmdtype==0){

    }else if(cmdtype==1){
    
    }
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0011
*Description :  LED控制
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0011)
{
    u32_t cmdtype;
    u32_t onoff;
    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    cmdtype = atoi(argv[2]);
    if(cmdtype==0){

    }else if(cmdtype==1){
      
    }else if(cmdtype==2){
      
    }
    rt_kprintf("OK\n");
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0021
*Description :  查询陀螺仪
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0021)
{
    
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd0022
*Description :  查询GPS
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd0022)
{
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    LOG_D("[%s]:$1 %s $2 %s",__func__,argv[1],argv[1]); 
    rt_kprintf("%s \n",__func__); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd1001
*Description :  磁盘测试
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd1001)
{
  
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
  
    LOG_D("[%s]:$1 %s $2 %s",__func__,argv[1],argv[1]); 
    rt_kprintf("%s \n",__func__); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd1002
*Description :  设备重启
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd1002)
{
    syswatch_reboot();
    rt_kprintf("ITTE $1002\n"); 
    rt_kprintf("OK\n"); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd1003
*Description :  恢复出厂化
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd1003)
{
    PPItemsFactory();
    rt_kprintf("ITTE $1003\n"); 
    rt_kprintf("OK\n"); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd1004
*Description :  设备关机
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd1004)
{
    /* 功能补充  关机 */
    rt_kprintf("ITTE $1004\n"); 
    rt_kprintf("OK\n"); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd1010
*Description :  FLASH 擦除，使用该操作后，默认未进行厂测
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd1010)
{
    W25QXXEraseChip();
    
    rt_kprintf("ITTE $1010\n"); 
    rt_kprintf("OK\n"); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd1011
*Description :  电池电压查询
*Input       :  
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd1011)
{
    //rt_kprintf("ITTE $1011 %d\n",colle_get_value(ADC_CHX_VBAT)); 
    rt_kprintf("OK\n"); 
    return 0;
}
/*
********************************************************************************
*Function    :  ITTECmd1012
*Description :  厂测完成标记
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
DEF_ITTESCMD_FN(ITTECmd1012)
{    
  
    if (argc < 2 ) {
        rt_kprintf("Please \n"); 
        return 0;
    }
    
    LOG_D("[%s]:$1 %s $2 %s",__func__,argv[1],argv[1]); 
    rt_kprintf("%s \n",__func__); 
    return 0;
}
/*
********************************************************************************
********************************************************************************
*/
static const struct itte_list_cmd {
    char *cmdid;
    char *describe;
    int (*docmd)(int argc, char** argv);
} itte_cmd_table[] = {
    {"$0001","<factory mode >",   ITTECmd0001},    /* 厂测模式 */
    {"$0002","<set sn>",          ITTECmd0002},    /* 产品序列号 */
    {"$0003","<set devid>",       ITTECmd0003},    /* 设备编号*/
    {"$0004","<get version>",     ITTECmd0004},    /* 版本信息 */
    {"$0010","<get imei>",        ITTECmd0005},    /* 查询IMEI */
    {"$0011","<get iccid>",       ITTECmd0011},    /* 查询ICCID */
    {"$0012","<set mqtt>",        ITTECmd0012},    /* 设置MQTT */
    {"$0014","<dl ca>",           ITTECmd0014},    /* 证书设置 */
    {"$0015","<network>",         ITTECmd0015},    /* 网络*/
    {"$0016","<switch sel>",      ITTECmd0016},    /* 网络切换 */
    {"$0017","<ble set >",        ITTECmd0017},    /* BLE配置 */
    {"$0018","<wifi set>",        ITTECmd0018},    /* WIFI配置 */
    {"$0020","<led test>",        ITTECmd0011},    /* LED控制 */
    {"$0021","<get grl>",         ITTECmd0021},    /* 查询陀螺仪 */
    {"$0022","<get gps>",         ITTECmd0022},    /* 查询GPS */
    {"$1001","<fs test>",         ITTECmd1001},    /* 磁盘测试 */
    {"$1002","<reboot> ",         ITTECmd1002},    /* 设备重启 */
    {"$1003","<factory>",         ITTECmd1003},    /* 恢复出厂化 */
    {"$1004","<shutdown>",        ITTECmd1004},    /* 设备关机  */
    {"$1010","<FLASH Chip Erase>",ITTECmd1010},    /* FLASH 擦除  */
    {"$1011","<get battery val>", ITTECmd1011},    /* 电池电压查询 */
    {"$1012","<set factory flg>", ITTECmd1012},    /* 厂测完成标记 */
    {0, 0}
};
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
    int table_size, i;
    
    rt_kprintf("cmdid describe  \n"); 
   
    table_size = sizeof(itte_cmd_table)/sizeof(itte_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        rt_kprintf("[%s]:%s\n",itte_cmd_table[i].cmdid,itte_cmd_table[i].describe); 
    }
}
/*
********************************************************************************
*Function    : ittes
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
void itte(int argc, char** argv)
{
    const struct itte_list_cmd *cmd;
    int table_size, i;
    
    if (argc < 2 ) {
        show_usage();
        return;
    }
    table_size = sizeof(itte_cmd_table)/sizeof(itte_cmd_table[0]) - 1;
    for (i = 0; i < table_size; i++) {
        if(strcmp(argv[1], itte_cmd_table[i].cmdid) == 0) {
            break;
        }
    }
    if (i == table_size) {
        show_usage();
        return;
    }
    cmd = &itte_cmd_table[i];
    
    if (cmd->docmd) {
        cmd->docmd(argc, argv);
    }
} 
MSH_CMD_EXPORT(itte,itte <help|all|...> );