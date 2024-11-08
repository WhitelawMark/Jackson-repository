#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_HOOK_USING_FUNC_PTR
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 512
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 512
//#define RT_USING_FAL
/* kservice optimization */

#define RT_DEBUG
#define RT_DEBUG_COLOR

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_PAGE_MAX_ORDER 11
#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_SMALL_MEM_AS_HEAP
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 256
#define RT_CONSOLE_DEVICE_NAME "uart1"
//#define RT_CONSOLE_DEVICE_NAME "lpuart1"


#define RT_VER_NUM 0x50001
#define RT_USING_HW_ATOMIC
#define RT_USING_CPU_FFS
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M33

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10
#define RT_USING_MSH
#define RT_USING_FINSH
#define FINSH_USING_MSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_CMD_SIZE 80
#define MSH_USING_BUILT_IN_COMMANDS
#define FINSH_USING_DESCRIPTION
#define FINSH_ARG_MAX 10

/* DFS: device virtual file system */


/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_UNAMED_PIPE_NUMBER 64
#define RT_USING_SYSTEM_WORKQUEUE
#define RT_SYSTEM_WORKQUEUE_STACKSIZE 2048
#define RT_SYSTEM_WORKQUEUE_PRIORITY 23
#define RT_USING_SERIAL
#define RT_USING_SERIAL_V1
#define RT_SERIAL_RB_BUFSZ 512
#define RT_USING_PIN
#define RT_USING_I2C
#define RT_USING_ADC
#define RT_USING_RTC
#define RT_USING_ALARM
#define RT_USING_I2C_BITOPS

#if 1
#define RT_USING_PM
#define PM_TICKLESS_THRESHOLD_TIME 2
//#define PM_USING_CUSTOM_CONFIG
#define PM_ENABLE_DEBUG
#define PM_ENABLE_SUSPEND_SLEEP_MODE
#define PM_ENABLE_THRESHOLD_SLEEP_MODE
#define PM_LIGHT_THRESHOLD_TIME 5
#define PM_DEEP_THRESHOLD_TIME 20
#define PM_STANDBY_THRESHOLD_TIME 100
#endif
/* Using USB */


/* C/C++ and POSIX layer */

#define RT_LIBC_DEFAULT_TIMEZONE 8

/* POSIX (Portable Operating System Interface) layer */
#define RT_LIBC_USING_TIME

/* Interprocess Communication (IPC) */


/* Socket is in the 'Network' category */

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_POSIX
#define DFS_USING_WORKDIR
#define DFS_FD_MAX 16
#define RT_USING_DFS_V1
#define DFS_FILESYSTEMS_MAX 2
#define DFS_FILESYSTEM_TYPES_MAX 2

/* Network */


/* Utilities */
#define RT_USING_ULOG
#define ULOG_OUTPUT_LVL_D
#define ULOG_OUTPUT_LVL 7
#define ULOG_ASSERT_ENABLE
#define ULOG_LINE_BUF_SIZE 256

/* log format */
#define ULOG_USING_ISR_LOG
#define ULOG_USING_COLOR
#define ULOG_OUTPUT_TIME
#define ULOG_OUTPUT_LEVEL
#define ULOG_OUTPUT_TAG
/* end of log format */
#define ULOG_BACKEND_USING_CONSOLE
#define ULOG_USING_FILTER
/* end of Utilities */

/* RT-Thread Utestcases */

#define RT_USING_RYM
#define YMODEM_USING_FILE_TRANSFER
#define RT_USING_UTEST
#define UTEST_THR_STACK_SIZE 4096
#define UTEST_THR_PRIORITY 20
/* RT-Thread online packages */

#define PKG_USING_EASYBLINK
#define PKG_EASYBLINK_MAX_LED_NUMS 1
#define PKG_USING_EASYBLINK_V201
#define PKG_USING_QLED
#define QLED_TOTAL 5
#define QLED_TIME_UNIT_MS 10
#define QLED_THREAD_NAME "qled"
#define QLED_THREAD_STACK_SIZE 512
#define QLED_THREAD_PRIO 3
#define PKG_USING_QLED_LATEST_VERSION


#define QLED_USING_SAMPLE
/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* IoT Cloud */


/* security packages */


/* language packages */

/* JSON: JavaScript Object Notation, a lightweight data-interchange format */


/* XML: Extensible Markup Language */


/* multimedia packages */

/* LVGL: powerful and easy-to-use embedded GUI library */


/* u8g2: a monochrome graphic library */


/* tools packages */


/* system packages */
#define PKG_USING_SYSWATCH
#define SYSWATCH_USING_TEST
#define SYSWATCH_EXCEPT_RESOLVE_MODE_0
#define SYSWATCH_EXCEPT_RESOLVE_MODE 0
#define SYSWATCH_EXCEPT_TIMEOUT 60
#define SYSWATCH_EXCEPT_CONFIRM_TMO 15
#define SYSWATCH_EXCEPT_RESUME_DLY 15
#define SYSWATCH_THREAD_PRIO 0
#define SYSWATCH_THREAD_STK_SIZE 512
#define SYSWATCH_THREAD_NAME "syswatch"
#define SYSWATCH_WDT_NAME "wdt"
#define SYSWATCH_WDT_TIMEOUT 5
#define PKG_USING_SYSWATCH_LATEST_VERSION
/* enhanced kernel services */


/* acceleration: Assembly language or algorithmic acceleration packages */


/* CMSIS: ARM Cortex-M Microcontroller Software Interface Standard */


/* Micrium: Micrium software products porting for RT-Thread */


/* peripheral libraries and drivers */

/* sensors drivers */


/* touch drivers */


/* Kendryte SDK */


/* AI packages */


/* Signal Processing and Control Algorithm Packages */


/* miscellaneous packages */

/* project laboratory */

/* samples: kernel and components samples */


/* entertainment: terminal games and other interesting software packages */


/* Arduino libraries */


/* Projects */


/* Sensors */


/* Display */


/* Timing */


/* Data Processing */


/* Data Storage */

/* Communication */


/* Device Control */


/* Other */


/* Signal IO */


/* Uncategorized */

#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32U5

/* Hardware Drivers Config */

#define SOC_STM32U575ZI

/* Onboard Peripheral Drivers */


/* On-chip Peripheral Drivers */

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
// #define BSP_USING_UART2
#define BSP_USING_LPUART1
// #define BSP_USING_UART4
//#define RT_SERIAL_USING_DMA
/* Board extended module Drivers */


#define BSP_USING_I2C2
#define BSP_I2C2_SCL_PIN    29 
#define BSP_I2C2_SDA_PIN    30

#define BSP_RTC_USING_LSI
#define BSP_USING_ONCHIP_RTC
#define BSP_USING_ON_CHIP_FLASH


#define BSP_USING_ADC1
#define BSP_USING_RNG
#endif
