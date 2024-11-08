/*
 * smart_port.h
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#ifndef __SMART_PORT_H_
#define __SMART_PORT_H_
/*
********************************************************************************
********************************************************************************
*/
enum
{		 	
    SMART_RPT_POWERON   = 0,    
    SMART_RPT_HEARTBEAT = 1,    
    SMART_RPT_OTA       = 2,    
    SMART_RPT_LOWBAT    = 3,    
    SMART_RPT_ELECFENCE = 4,    
}; 
/*
********************************************************************************
********************************************************************************
*/
int smart_report_message(int evt);
int smart_mqtt_recv(char *buf, int len);
#endif 
/*__SMART_PORT_H_*/