/*
 * esp_btser.h
 *
 *  Created on: 2023年08月24日
 *      Author: lwp
 */
 
#ifndef _BT_SER_H_
#define _BT_SER_H_

/*
********************************************************************************
********************************************************************************
*/ 


/*
********************************************************************************
********************************************************************************
*/ 
int esp_btser_init(void);
void esp_btser_disable(void);
int esp_btser_send(u8_t *msg, u32_t msglen, void *arg);
#endif 
/* _ESP_BTSER_H_ */
