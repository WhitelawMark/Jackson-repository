/*
 * qled_sample.h
 *
 * Change Logs:
 * Date           Author            Notes
 * 2020-06-22     qiyongzhong       first version
 */

#ifndef __QLED_SAMPLE_H__
#define __QLED_SAMPLE_H__

#include <qled.h>
#include "drv_gpio.h"
#ifdef QLED_USING_SAMPLE

//#include <drv_common.h>

#ifndef QLED_RUN_PIN
#define QLED_RUN_PIN     -1
#endif

#ifndef QLED_SOS_PIN
#define QLED_SOS_PIN      GET_PIN(B, 7) //25//GET_PIN(B, 9)
#endif

#endif

#endif

