/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __ESP_BT_H                                                          
#define __ESP_BT_H    

/*
********************************************************************************
********************************************************************************
*/

int __esp_ble_send(socket_t *socket, ble_msg_t *msg);

/*
********************************************************************************
********************************************************************************
*/
int esp_ble_init(char *name, char *pin,int (*recv)(void *arg));
int esp_ble_send(int sd, u8_t *buf, u32_t len, void *arg);
#endif 
/* __ESP_BT_H */
