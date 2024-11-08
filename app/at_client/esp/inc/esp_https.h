/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __ESP_HTTPS_H                                                          
#define __ESP_HTTPS_H    

/*
********************************************************************************
********************************************************************************
*/
int __esp_https_download(socket_t *socket, https_msg_t *msg);
/*
********************************************************************************
********************************************************************************
*/
int esp_https_download(int sd,char *url,char *filename, void(*res_cb)(int res, void *arg), void *arg);

int esp_https_init(char *ssid,char *pwd);
#endif 
/* __ESP_HTTPS_H */
