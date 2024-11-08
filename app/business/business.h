/*
 *  business.h
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */

/* Data types redefintion headers. */
#ifndef __BUSINESS_H_
#define __BUSINESS_H_
/*
********************************************************************************
********************************************************************************
*/ 
/*
 * 传感器ID标识
 */
typedef enum {
    RTSCPUTemp  ,   
    RTSBatVol   ,  
    RTS_MAX,
}SID_E;

/*
 * 状态寄存器
 */
typedef enum {
    BUSI_SYS,          /* 系统状态  */

}BSN_E;

/*
********************************************************************************
********************************************************************************
*/ 
u32_t RTGetSidValue(SID_E eid);

u32_t busi_get_status(int type);
int busi_get_remain_time(void);
void business_start(void);
void business_process(void);
 
#endif
/*__BUSINESS_H_*/

