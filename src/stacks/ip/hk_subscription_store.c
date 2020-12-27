#include "hk_subscription_store.h"

#include <stdbool.h>

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_util.h"

typedef struct
{
    hk_chr_t *chr;
    int *sockets;
    size_t number_of_sockets;
} hk_subscription_t;

hk_subscription_t *subscriptions = NULL;

static esp_err_t hk_subscription_store_get_subscription(hk_chr_t *chr, hk_subscription_t **subscription)
{
    if (*subscription != NULL)
    {
        HK_LOGE("Should never happen.");
        return ESP_ERR_INVALID_ARG;
    }

    hk_ll_foreach(subscriptions, current_subscription)
    {
        if (current_subscription->chr == chr)
        {
            *subscription = current_subscription;
        }
    }

    return ESP_OK;
}

esp_err_t hk_subscription_store_get(hk_chr_t *chr, int **sockets, size_t *number_of_sockets)
{
    esp_err_t ret = ESP_OK;
    hk_subscription_t *subscription = NULL;
    RUN_AND_CHECK(ret, hk_subscription_store_get_subscription, chr, &subscription);

    if (subscription != NULL)
    {
        *sockets = subscription->sockets;
        *number_of_sockets = subscription->number_of_sockets;
    }
    else
    {
        ret = ESP_ERR_NOT_FOUND;
        *sockets = NULL;
        *number_of_sockets = 0;
    }

    return ret;
}

esp_err_t hk_subscription_store_add(hk_chr_t *chr, int socket)
{
    esp_err_t ret = ESP_OK;
    hk_subscription_t *subscription = NULL;
    RUN_AND_CHECK(ret, hk_subscription_store_get_subscription, chr, &subscription);

    if (subscription == NULL)
    {
        subscription = subscriptions = hk_ll_init(subscriptions);
        subscription->chr = chr;
        subscription->sockets = NULL;
    }

    if (subscription->sockets == NULL)
    {
        HK_LOGV("Creating subscription list for %x and adding socket %d.", (uint)chr, socket);
        subscription->sockets = (int *)malloc(sizeof(int));
        subscription->number_of_sockets = 1;
        subscription->sockets[0] = socket;
    }
    else
    {
        HK_LOGV("Adding socket %d to subscription list of %x.", socket, (uint)chr);
        int *sockets_array_old = subscription->sockets;
        subscription->number_of_sockets++;
        subscription->sockets = (int *)malloc(sizeof(int) * subscription->number_of_sockets);
        subscription->sockets[subscription->number_of_sockets - 1] = socket;
        memcpy(subscription->sockets, sockets_array_old, sizeof(int) * (subscription->number_of_sockets - 1));
        free(sockets_array_old);
    }

    return ret;
}

esp_err_t hk_subscription_store_remove(hk_chr_t *chr, int socket)
{
    esp_err_t ret = ESP_OK;
    hk_subscription_t *subscription = NULL;
    RUN_AND_CHECK(ret, hk_subscription_store_get_subscription, chr, &subscription);

    if (subscription != NULL)
    {
        if (subscription->number_of_sockets - 1 > 0)
        {
            int index_of_socket = -1;
            for (size_t i = 0; i < subscription->number_of_sockets; i++)
            {
                if (subscription->sockets[i] == socket)
                {
                    index_of_socket = i;
                    break;
                }
            }

            if (index_of_socket >= 0)
            {
                subscription->number_of_sockets--;
                HK_LOGD("Removing subscription of socket %d at list of %x.", socket, (uint)subscription->chr);
                int *sockets_array_old = subscription->sockets;
                subscription->sockets = (int *)malloc(sizeof(int) * subscription->number_of_sockets);
                memcpy(
                    subscription->sockets,
                    sockets_array_old,
                    sizeof(int) * index_of_socket);
                memcpy(
                    subscription->sockets + index_of_socket,
                    sockets_array_old + index_of_socket + 1,
                    sizeof(int) * (subscription->number_of_sockets - index_of_socket));
                free(sockets_array_old);
            }
            else
            {
                HK_LOGD("Cannot remove subscription of socket %d at list of %x, as socket was not found in list.", socket, (uint)subscription->chr);
            }
        }
        else
        {
            subscription->number_of_sockets = 0;
            free(subscription->sockets);
            subscription->sockets = NULL;
        }
    }
    else
    {
        HK_LOGD("Cannot remove subscription of socket %d at list of %x, as list was not found.", socket, (uint)subscription->chr);
    }

    return ret;
}

esp_err_t hk_subscription_store_remove_all(int socket)
{
    HK_LOGD("Removing all subscriptions for socket %d.", socket);
    esp_err_t ret = ESP_OK;

    hk_ll_foreach(subscriptions, current_subscription)
    {
        esp_err_t ret_local = hk_subscription_store_remove(current_subscription->chr, socket);
        if (ret_local != ESP_OK)
        {
            HK_LOGE("Error removing socket %d for %x.", socket, (uint)current_subscription->chr);
            ret = ret_local;
        }
    }

    return ret;
}

void hk_subscription_store_free()
{
    hk_ll_foreach(subscriptions, subscription_item)
    {
        free(subscription_item->sockets);
    }

    hk_ll_free(subscriptions);
    subscriptions = NULL;
}