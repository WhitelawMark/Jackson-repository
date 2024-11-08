/*
 *  at.h
 *  net block
 *  Created on: 2018年11月29日
 *      Author: root
 */

#ifndef AT_H_
#define AT_H_
#include <stdio.h>
#include <string.h>
#include "rtthread.h"


#define SOCK_DGRAM       1  /*UDP*/
#define SOCK_STREAM      2  /*TCP*/
#define SOCK_MQTT        4  /*MQTT*/
#define SOCK_FTP         5  /*FTP*/
#define SOCK_BLE         6  /*BLE*/
#define SOCK_HTTPS       7  /*HTTPS*/

typedef  unsigned int    at_size_t;
typedef  int             at_ssize_t;

#define at_snprintf      rt_snprintf
#define at_strlen        strlen
#define at_strncpy       strncpy
#define at_strcmp        strcmp
#define at_memcpy        memcpy
#define at_memcmp        memcmp
#define at_memset        memset
#define at_atoi          atoi

#define at_isupper(c)    (((c)>='A')&&((c)<='Z'))
#define at_islower(c)    (((c)>='a')&&((c)<='z'))
#define at_isdigit(c)    (((c)>='0')&&((c)<='9'))
#define at_isspace(c)    ((c)==' ' || (c)=='\t'  || (c)=='\r' || (c)=='\n')

enum {
    EV_TX,
    EV_RX,
    EV_CONNECT
};

typedef struct {
    long counter;
} atomic_t;

typedef struct at_addr {
    unsigned short port;
    char ip[128];
} at_addr_t;

/*NB message queue*/
typedef struct {
    struct rt_mutex lock;
    rt_list_t list;
} at_mq_t;

/*NB message*/
typedef struct {
    rt_list_t entry;
    int type;
    char msg[0];
} at_msg_t;


#if 1
#pragma inline
static int __atomic_read(volatile long *addr)
{
    __asm("LDR R0, [R0]\n");
    return (int)addr;
}
#define atomic_read(v)   __atomic_read(&(v)->counter)


#pragma inline
static void __atomic_set(volatile long *addr, long value)
{
    //__asm("STR R1, [R0]\n");
    __asm(
    "lpset:\n"
    "       LDREX R2, [R0]\n"
    "       STREX R2, R1, [R0]\n"
    "       TEQ   R2, #0\n"
    "       BNE   lpset\n" );
}
#define atomic_set(v, value)   __atomic_set(&(v)->counter, value)

#else
#define atomic_read(v)          (*((volatile long *)(&(v)->counter)))           //原子操作,如果不是需改为汇编实现
#define atomic_set(v, value)    (*((volatile long *)(&(v)->counter)) = (value)) //原子操作,如果不是需改为汇编实现
#endif

#pragma inline
static int __atomic_cmpxchg(volatile long *addr, long old, long new)
{
     /*R0 ---> addr
      *R1 ---> old
      *R2 ---> new*/
    __asm(
    "lpchg:\n"
    "     LDREX     R3, [R0]\n"
    "     TEQ       R3, R1\n"
    "     BNE       exitchg\n"
    "     STREX     R3, R2, [R0]\n"
    "     TEQ       R3, #0\n"
    "     BNE       lpchg\n"
    "     MOV       R3, R1\n"
    "exitchg:\n"
    "     MOV       R0, R3\n");

    return (int)addr;
}
#define atomic_cmpxchg(v, old, new)  __atomic_cmpxchg(&(v)->counter, old, new)

#define at_message_type(m)    rt_container_of(m, at_msg_t, msg)->type

int at_mq_init(at_mq_t *mq);

int at_mq_detach(at_mq_t *mq);

void *at_message_alloc(at_size_t size);

void at_message_free(void *msg);

void *at_message_get(at_mq_t *mq);

void at_message_put(at_mq_t *mq, void *msg, void (*notify)(int evtype), int evtype);


#endif /* AT_H_ */
