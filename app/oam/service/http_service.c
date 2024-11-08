/*
 * https_service.c
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
#include <stdio.h>
#include <string.h>
#include <board.h>
#include <rtthread.h>
#include "type.h"
#include "at.h"
#include "at_client.h"
#include "esp.h"
#include "PPItems.h"
#include "esp_https.h"
#include "oam_thread.h" 

#define DBG_TAG "https"
#define DBG_LVL DBG_LOG   //DBG_LOG
#include <rtdbg.h>
/*
********************************************************************************
********************************************************************************
*/ 
 