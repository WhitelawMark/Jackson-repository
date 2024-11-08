
#ifndef __APP_LIB_H_
#define __APP_LIB_H_
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <board.h>
#include <rtthread.h>
#include <drivers/pin.h>

#ifdef RT_USING_RTC
#include <sys/time.h>
#include <unistd.h> 
#endif
#include <fcntl.h>

#include <sys/types.h> 
#include <sys/stat.h> 


#include "type.h"

#include "app_def.h"
#include "ustring.h"  
#include "pit.h" 
#include "dfs_fs.h" 
#include "qled.h" 
 
#include "version.h" 
#include "mx25uxx.h"
#include "drv_rng.h"
#include "drv_flash/drv_flash.h"
#include "reg_edit.h"
#include "bg77.h" 
#include "esp_btser.h" 
#include "esp_client.h" 
#include "PPItems.h"
#include "syswatch.h" 

#include "uesim.h"
#include "adc_thread.h"
#include "business.h"

#include "app_gpio.h" 
#include "app_init.h" 
#include "app_led.h" 
#include "app_key.h"
#include "app_ulog.h"

#include "bt_service.h"
#include "http_service.h"
#include "esp_client.h"

#include "quectel_client.h"

#include "oam_thread.h"
#include "upgrade.h"


#include "lan.h" 
#include "cJSON.h" 

#include "oam_thread.h"
#include "oam_port.h" 
#include "ota.h" 

#include "smart_analy.h"
#include "smart_port.h"
#endif
/*__APP_LIB_H_*/
