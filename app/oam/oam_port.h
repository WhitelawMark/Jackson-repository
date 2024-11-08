/*
 * oam_port.h
 *
 *  Created on: 2023年7月17日
 *      Author: lwp
 */

#ifndef LAN_PORT_H_
#define LAN_PORT_H_

#define OAM_KEY_PRESS  1

#define oam_timer_reinit    oam_timer_init
/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
********************************************************************************
*/

 
/*
********************************************************************************
********************************************************************************
*/
int oam_is_ready(void);
int oam_is_connect(void);
/*
********************************************************************************
********************************************************************************
*/

u16_t oam_get_le_word(u8_t *msg);
u32_t oam_get_le_dword(u8_t *msg);
void oam_set_le_word(u8_t *msg, u16_t value);
void oam_set_le_dword(u8_t *msg, u32_t value);

 
void oam_start(void);
void oam_timer_init(void);
void oam_timer_start(int timeout);
void oam_timer_stop(void);

u32_t oam_get_runtime(void);
void oam_timer_setcb( void (*func)(void *arg));
rt_err_t oam_send_msg(int msgtype,int arg1,int arg2,int arg3 ,void *arg);
#endif /* LAN_PORT_H_ */
