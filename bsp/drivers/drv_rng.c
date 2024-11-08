/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2018-11-06     balanceTWK        first version
 * 2019-04-23     WillianChan       Fix GPIO serial number disorder
 * 2020-06-16     thread-liu        add STM32MP1
 * 2020-09-01     thread-liu        add GPIOZ
 * 2020-09-18     geniusgogo        optimization design pin-index algorithm
 */

#include <board.h>
#include "drv_rng.h"

#ifdef BSP_USING_RNG

static RNG_HandleTypeDef hrng;

/**
  * @brief RNG Read Function
  * @param None
  * @retval None
  */

int rt_rng_read(void)
{
    uint32_t random;
    
    if (HAL_RNG_GenerateRandomNumber(&hrng, &random) != HAL_OK)
    {
        /* Random number generation error */
        Error_Handler();
    }
    return random;
}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */

int rt_hw_rng_init(void)
{
    hrng.Instance = RNG;
    hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
    if (HAL_RNG_Init(&hrng) != HAL_OK)
    {
        Error_Handler();
    }
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_rng_init);
#endif /* BSP_USING_RNG */
