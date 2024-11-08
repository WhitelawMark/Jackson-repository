/*
 * at.c
 *
 *  Created on: 2023Äê07ÔÂ17ÈÕ
 *      Author: lwp
 */
#include <string.h>
#include "at.h"

/*
********************************************************************************
*Function    : at_mq_init
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int at_mq_init(at_mq_t *mq)
{
    rt_mutex_init(&mq->lock, "nb_mq", RT_IPC_FLAG_FIFO);
    rt_list_init(&mq->list);

    return 0;
}
/*
********************************************************************************
*Function    : at_mq_detach
*Description :
*Input       :
*Output      :
*Return      : 0 if success, -1 if fail.
*Others      :
********************************************************************************
*/
int at_mq_detach(at_mq_t *mq)
{
    RT_ASSERT(rt_list_isempty(&mq->list));

    rt_mutex_detach(&mq->lock);

    return 0;
}

/*
***************************************************************************************************
*Function    : at_message_alloc
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
***************************************************************************************************
*/
void *at_message_alloc(at_size_t size)
{
    at_msg_t *msg;

    msg = (at_msg_t *)rt_malloc(sizeof(at_msg_t) + size);
    if (msg == RT_NULL) {
        return RT_NULL;
    }
    return (void *)(msg->msg);
}

/*
***************************************************************************************************
*Function    : at_message_free
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
***************************************************************************************************
*/
void at_message_free(void *msg)
{
    if (msg) {
        rt_free(rt_container_of(msg, at_msg_t, msg));
    }
}

/*
***************************************************************************************************
*Function    : at_message_get
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
***************************************************************************************************
*/
void *at_message_get(at_mq_t *mq)
{
    at_msg_t *msg;

    rt_mutex_take(&mq->lock, RT_WAITING_FOREVER);

    if (!rt_list_isempty(&mq->list)) {
        msg = rt_list_entry(mq->list.next, at_msg_t, entry);
        rt_list_remove(&msg->entry);

        rt_mutex_release(&mq->lock);

        return (void *)(msg->msg);
    }

    rt_mutex_release(&mq->lock);

    return RT_NULL;
}

/*
***************************************************************************************************
*Function    : at_message_put
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
***************************************************************************************************
*/
void at_message_put(at_mq_t *mq, void *msg, void (*notify)(int evtype), int evtype)
{
    rt_mutex_take(&mq->lock, RT_WAITING_FOREVER);

    /* put message to end of queue */
    rt_list_insert_before(&mq->list, &rt_container_of(msg, at_msg_t, msg)->entry);

    rt_mutex_release(&mq->lock);

    if (notify) {
        notify(evtype);
    }
}
