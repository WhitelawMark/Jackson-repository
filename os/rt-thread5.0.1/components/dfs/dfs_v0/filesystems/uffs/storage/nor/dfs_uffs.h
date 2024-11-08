#ifndef __DFS_UFFS_H__
#define __DFS_UFFS_H__
/*
******************************************************************************
Copyright (C),  Fujian <Company>. Co., Ltd.
File name    ： dfs_uffs.h
Description  ： 头文件
Compile      ： 
Author       ： emb_wlq
Version      ： v1.0
Date         ： 
Others       ： 
History      ： 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期：2014.02.20         修改者：emb_wlq
   内容：v1.0

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
2）...

******************************************************************************
*/
/*
******************************************************************************
头文件
******************************************************************************
*/
#include "uffs_config.h"
#include "uffs/uffs_public.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
******************************************************************************
外部宏定义
******************************************************************************
*/
/* the UFFS ECC mode opitons  */
#ifndef RT_UFFS_ECC_MODE
#define RT_UFFS_ECC_MODE      1
#endif

/*
 * RT_UFFS_ECC_MODE:
 * 0, Do not use ECC
 * 1, UFFS calculate the ECC
 * 2, Flash driver(or by hardware) calculate the ECC
 * 3, Hardware calculate the ECC and automatically write to spare.
 */
#if RT_UFFS_ECC_MODE == 0
#define RT_CONFIG_UFFS_ECC_MODE UFFS_ECC_NONE
#elif RT_UFFS_ECC_MODE == 1
#define RT_CONFIG_UFFS_ECC_MODE UFFS_ECC_SOFT
#elif RT_UFFS_ECC_MODE == 2
#define RT_CONFIG_UFFS_ECC_MODE UFFS_ECC_HW
#elif RT_UFFS_ECC_MODE == 3
#define RT_CONFIG_UFFS_ECC_MODE UFFS_ECC_HW_AUTO
#endif

/* #define RT_CONFIG_UFFS_ECC_MODE  UFFS_ECC_HW_AUTO */
/* #define RT_CONFIG_UFFS_ECC_MODE  UFFS_ECC_SOFT */
/* #define RT_CONFIG_UFFS_ECC_MODE  UFFS_ECC_NONE */

/* enable this ,you need provide a mark_badblock/check_block funciton */
/* #define RT_UFFS_USE_CHECK_MARK_FUNCITON */

#if RT_CONFIG_UFFS_ECC_MODE == UFFS_ECC_SOFT      /* let uffs do soft ecc */
#define RT_CONFIG_UFFS_LAYOUT    UFFS_LAYOUT_UFFS /* UFFS_LAYOUT_FLASH */

#elif RT_CONFIG_UFFS_ECC_MODE == UFFS_ECC_HW_AUTO /* nand driver make ecc and do ecc correct */
#define RT_CONFIG_UFFS_LAYOUT    UFFS_LAYOUT_FLASH

#elif RT_CONFIG_UFFS_ECC_MODE == UFFS_ECC_NONE
#define RT_CONFIG_UFFS_LAYOUT    UFFS_LAYOUT_UFFS /* UFFS_LAYOUT_FLASH */

#else
#error "uffs under rt-thread do not support this ECC mode"
#endif /* RT_CONFIG_UFFS_ECC_MODE */

#if (!CONFIG_USE_STATIC_MEMORY_ALLOCATOR)  && (CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR)
#define RT_UFFS_MEMORY_ALLOCATOR  1  /* use system memory allocator */
#elif (CONFIG_USE_STATIC_MEMORY_ALLOCATOR) && (!CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR)
#define RT_UFFS_MEMORY_ALLOCATOR  0  /* use static memory allocator */
#else
#error "UFFS: CONFIG_USE_STATIC_MEMORY_ALLOCATOR ,CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR are invalid!"
#endif

//#if CONFIG_USE_STATIC_MEMORY_ALLOCATOR > 0
//#error "dfs_uffs only support CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR"
//#endif

#if defined(CONFIG_UFFS_AUTO_LAYOUT_USE_MTD_SCHEME)
#error "dfs_uffs not support CONFIG_UFFS_AUTO_LAYOUT_USE_MTD_SCHEME"
#endif

#if (RT_CONFIG_UFFS_ECC_MODE == UFFS_ECC_HW_AUTO) && (RT_CONFIG_UFFS_LAYOUT != UFFS_LAYOUT_FLASH)
#error "when use UFFS_ECC_HW_AUTO, you must use UFFS_LAYOUT_FLASH"

#elif (RT_CONFIG_UFFS_ECC_MODE == UFFS_ECC_SOFT) && (RT_CONFIG_UFFS_LAYOUT != UFFS_LAYOUT_UFFS)
#warning "when use UFFS_ECC_SOFT, it is recommended to use UFFS_LAYOUT_UFFS"
#endif

/*
******************************************************************************
外部变量声明
******************************************************************************
*/
extern const uffs_FlashOps uffs_Ops;

/*
******************************************************************************
全局函数声明
******************************************************************************
*/
void uffsSetupStorage(struct uffs_StorageAttrSt *attr, st_flash_device_t *flash);
int dfs_uffs_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __DFS_UFFS_H__ */


