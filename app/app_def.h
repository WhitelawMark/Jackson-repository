
#ifndef __APP_DEF_H_
#define __APP_DEF_H_


#ifndef MAX
# define MAX(x,y) ((x) < (y) ? (y) : (x))
#endif
#ifndef MIN
# define MIN(x,y) ((x) > (y) ? (y) : (x))
#endif
#define JS_STR_EN    "EN"
#define JS_STR_DIS   "DIS"  
#define JS_GET_EN(x)              ( ((x) == (1)) ? (JS_STR_EN) : (JS_STR_DIS) )

#define JS_STR_OPEN    "open"
#define JS_STR_CLOSE   "close"  
#define JS_GET_SWS(x)              ( ((x) == (1)) ? (JS_STR_OPEN) : (JS_STR_CLOSE) )

#define OS_COUNTOF(a) (sizeof(a) / sizeof(a[0]))

#define ARRAYNUM(arr_name)      (uint32_t)(sizeof(arr_name) / sizeof(*(arr_name)))

typedef enum {CLOSE = 0, OPEN = !CLOSE} State;
#endif
/*__APP_DEF_H_*/
