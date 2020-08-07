#include "hk_subscription_store.h"

#include <stdbool.h>

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"

typedef struct
{
    hk_chr_t *chr;
    hk_session_t **sessions;
} hk_subscription_t;

hk_subscription_t *subscriptions = NULL;

hk_subscription_t *hk_subscription_store_get_subscription(hk_chr_t *chr)
{
    hk_subscription_t *subscription = NULL;

    hk_ll_foreach(subscriptions, current_subscription)
    {
        if (current_subscription->chr == chr)
        {
            subscription = current_subscription;
        }
    }

    return subscription;
}

hk_session_t **hk_subscription_store_get_sessions(hk_chr_t *chr)
{
    hk_subscription_t *subscription = hk_subscription_store_get_subscription(chr);
    if (subscription == NULL)
    {
        return NULL;
    }

    return subscription->sessions;
}

void hk_subscription_store_add_session(hk_chr_t *chr, hk_session_t *session)
{
    hk_subscription_t *subscription = hk_subscription_store_get_subscription(chr);

    if (subscription == NULL)
    {
        subscription = subscriptions = hk_ll_init(subscriptions);
        subscription->chr = chr;
        subscription->sessions = NULL;
    }
    else
    {
        hk_ll_foreach(subscription->sessions, current_session)
        {
            if (*current_session == session)
            {
                // Subscription exists, doing nothing.
                return;
            }
        }
    }

    HK_LOGD("Adding session %d (%x) to subscription list of %x.", 
        session->socket, (uint)session, (uint)chr);
    subscription->sessions = hk_ll_init(subscription->sessions);
    *subscription->sessions = session;
}

void hk_subscription_store_remove_session_from_subscription(hk_chr_t *chr, hk_session_t *session)
{
    hk_subscription_t *subscription = hk_subscription_store_get_subscription(chr);

    if (subscription != NULL)
    {
        hk_ll_foreach(subscription->sessions, current_session)
        {
            if (*current_session == session)
            {
                HK_LOGD("Removing session %d from subscription list of %x.", 
                    session->socket, (uint)chr);
                subscription->sessions = hk_ll_remove(subscription->sessions, current_session);
                return;
            }
        }
    }

    HK_LOGD("Could not remove subscription of session %d in subscription list of %x. It was not found.", 
        session->socket, (uint)chr);
}

void hk_subscription_store_remove_session(hk_session_t *session)
{
    HK_LOGD("Removing all subscriptions for session %d.", session->socket);

    hk_ll_foreach(subscriptions, current_subscription)
    {
        for (hk_session_t **current_session = current_subscription->sessions; current_session != NULL;)
        {   
            hk_session_t *current_session_ptr = *current_session;
            if (current_session_ptr == session)
            {
                hk_session_t **next = hk_ll_next(current_session);
                current_subscription->sessions = hk_ll_remove(current_subscription->sessions, current_session);
                current_session = next;
            }
            else
            {
                current_session = hk_ll_next(current_session);
            }
        }
    }
}

void hk_subscription_store_free()
{
    hk_ll_foreach(subscriptions, subscription_item)
    {
        hk_ll_free(subscription_item->sessions);
    }

    hk_ll_free(subscriptions);
    subscriptions = NULL;
}