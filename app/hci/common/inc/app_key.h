/*
 *  key.h
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */

#ifndef __APP_KEY_H_
#define __APP_KEY_H_
/*
********************************************************************************
********************************************************************************
*/ 
#define KEY1_Pin           59
#define KEY2_Pin           58
#define KEY3_Pin           57
#define KEY4_Pin           56
#define KEY5_Pin           55
/*
********************************************************************************
********************************************************************************
*/ 
enum
{		 	
    KEY_LOCK  = 0,      
    KEY_ULOCK= 1 ,       
}; 
typedef enum
{		 	
    KEY_VALUE_NONE,      
    KEY_VALUE_SHORT_1,   
    KEY_VALUE_LONG_1,    
}KEY_VAL_E; 

typedef struct _KEY_MSG
{
    u32_t keyval;          /* 触发类型   */        
    u32_t keyeid;          /* 按键编号   */
}keyMsg_t;

/*
********************************************************************************
********************************************************************************
*/ 
void KeySetCb(rt_err_t (*pCmdCb)(u32_t keyeid));
void KeyClrCb(void);
void key_lock(void);
void key_ulock(void);
#endif
/*__APP_KEY_H_*/

