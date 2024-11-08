/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __ESP_CMD_H                                                          
#define __ESP_CMD_H    

/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
********************************************************************************
*/
int esp_cmd_line( const char *atcmd,const char *ack,
                  char *xbuf, int xbuf_size,
                  unsigned int timeout_ms, int det_err);


int esp_cmd_line_with_len(const char *atcmd, int atcmdlen,
                          const char *ack,char *xbuf, int xbuf_size,
                          unsigned int timeout_ms, int det_err);

int esp_cmd_line_get(char *suffix,char *xbuf, int xbuf_size, unsigned int timeout_ms);

int esp_cmd_send(const char *atcmd);
/*
********************************************************************************
********************************************************************************
*/
/*
********************************************************************************
********************************************************************************
*/

void esp_reset(void);
void esp_restart(void);

void esp_wakeup(void);
void esp_sleep(void);

void esp_power_ctrl(int flag);
void esp_wakeup_ctrl(int power);

int esp_at_resp_detect(void);
int esp_cwjap_config(void);
int esp_cwmode_config(int cwmode);
int esp_cwap_config(char *ssid ,char *password);
int esp_sntp_config(char *url);
int esp_rfpower_config(int power);
int esp_sysutemp_config(int utempval);

int esp_cwstate_detect(void);
int esp_cwlap_detect(char *ssid);
int esp_cipsta_detect(void);
int esp_sysutemp_detect(void);
int esp_rfpower_detect(void);
int esp_sysram_detect(void);
int esp_gsm_detect(void);
int esp_systemp_detect(void);

int esp_ls_network(void);

/*
********************************************************************************
********************************************************************************
*/
int __esp_wifi_sta_init(void);
#endif 
/* __ESP_CMD_H */
