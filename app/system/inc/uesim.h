/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */                                                                        
#ifndef __UESIM_H                                                      
#define __UESIM_H                                                      
                                                                          
/*
********************************************************************************
********************************************************************************
*/                                                                      
typedef struct PROFILE_INFO {
    char iccid[21];         // 20-digit ICCID, padded with F
    char type;              // 0 test, 1 provisioning, 2 operational
    char state;              // 0 disabled, 1 enabled
} profile_info_t;

/*
********************************************************************************
********************************************************************************
*/                                                                         
                                       
/*
********************************************************************************
********************************************************************************
*/
int rt_lpa_get_profile_info(char* buf, profile_info_t* pi, char* num, char max_num);
#endif 
/* __UESIM_H */
