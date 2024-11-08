/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 */

#ifndef __DRV_RNG_H__
#define __DRV_RNG_H__

#include <drv_common.h>
#include <board.h>

#ifdef __cplusplus
extern "C" {
#endif

int rt_rng_read(void);
int rt_hw_rng_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_RNG_H__ */

