/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __OTA_H                                                          
#define __OTA_H                                                          
 
 
#include "flashmap.h" 
/*
********************************************************************************
********************************************************************************
*/
int ota_ls_running(void);
void ota_start(void);
void ota_init(void);
#endif 
/* __OTA_H */
