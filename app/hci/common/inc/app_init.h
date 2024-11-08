 /*
 *  app_init.h
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#ifndef _APP_INIT_H_
#define _APP_INIT_H_
 
/*
********************************************************************************
********************************************************************************
*/ 

/*
********************************************************************************
********************************************************************************
*/ 
void app_version(void);

void app_softvs_init(void);

void app_parm_init(void);

char *app_get_softvs(void);

void app_get_chip_id(void);

char app_board_type(void);

u32_t *board_get_cpuid(void);


void app_set_otastat(u8_t stat);

int sys_set_time(int year, int mon,int day,int hour,int min,int sec);

int app_init(void);
#endif 
/* _APP_INIT_H_ */