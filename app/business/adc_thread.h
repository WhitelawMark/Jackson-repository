/*
 *  colle_thread.h
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#ifndef __COLLE_THREAD_H_
#define __COLLE_THREAD_H_
/*
********************************************************************************
********************************************************************************
*/ 
 
#define  ADC_CHX_VBAT 1
#define  ADC_CHX_TEMP 2

/*
********************************************************************************
********************************************************************************
*/ 
signed short colle_get_value(int  eid);
signed short colle_get_adc_value(int  eid);
#endif
/*__COLLE_THREAD_H_*/
