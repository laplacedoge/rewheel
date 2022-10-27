#include "queue.h"

#include <stdlib.h>
#include <string.h>

static int queue_config_load_default(queue_config_t *config)
{
    if (config == NULL)
    {
        return QUE_ERR_BAD_ARG;
    }

    config->ndsize_max = QUE_DEF_NDSIZE_MAX;
    config->nodnum_max = QUE_DEF_NODNUM_MAX;

    return QUE_OK;
}

static int queue_node_delete(queue_node_t *node)
{
    if (node == NULL)
    {
        return QUE_ERR_BAD_ARG;
    }

    free(node->data);
    free(node);

    return QUE_OK;
}

int queue_create(queue_context_t **context, queue_config_t *config)
{
    int ret;
    queue_context_t *ctx;

    if (context == NULL)
    {
        return QUE_ERR_BAD_ARG;
    }

    ctx = (queue_context_t *)malloc(sizeof(queue_context_t));
    if (ctx == NULL)
    {
        return QUE_ERR_NO_MEM;
    }

    /* init head and tail node */
    ctx->nod_head = NULL;
    ctx->nod_tail = NULL;

    /* init configuration */
    if (config == NULL)
    {
        queue_config_load_default(&ctx->conf);
    }
    else
    {
        memcpy(&ctx->conf, config, sizeof(queue_config_t));
    }

    /* init status */
    ctx->stat.nod_num = 0;
    ctx->stat.nhdata_size = 0;

    *context = ctx;

    return QUE_OK;
}

int queue_delete(queue_context_t *context)
{
    if (context == NULL)
    {
        return QUE_ERR_BAD_ARG;
    }

    if (context->stat.nod_num > 0)
    {
        queue_node_t *nod;

        while (context->nod_head != NULL)
        {
            nod = context->nod_head;
            context->nod_head = context->nod_head->next;
            queue_node_delete(nod);
        }
    }
    free(context);

    return QUE_OK;
}

int queue_status(queue_context_t *context, queue_status_t *status)
{
    if (context == NULL || status == NULL)
    {
        return QUE_ERR_BAD_ARG;
    }

    memcpy(status, &context->stat, sizeof(queue_status_t));

    return QUE_OK;
}

int queue_enqueue(queue_context_t *context, const void *data, size_t size)
{
    queue_node_t *nod;
    void *dat;
    int ret;

    if (context == NULL || data == NULL || size == 0)
    {
        return QUE_ERR_BAD_ARG;
    }

    if (context->stat.nod_num == context->conf.nodnum_max)
    {
        return QUE_ERR_FULL_QUE;
    }

    if (size > context->conf.ndsize_max)
    {
        return QUE_ERR_OVERLONG_NDATA;
    }

    dat = malloc(size);
    if (dat == NULL)
    {
        return QUE_ERR_NO_MEM;
    }

    nod = (queue_node_t *)malloc(sizeof(queue_node_t));
    if (dat == NULL)
    {
        free(dat);
        return QUE_ERR_NO_MEM;
    }

    memcpy(dat, data, size);

    nod->next = NULL;
    nod->data = dat;
    nod->size = size;

    if (context->nod_tail == NULL)
    {
        context->nod_head = nod;
        context->nod_tail = nod;

        context->stat.nhdata_size = size;
    }
    else
    {
        context->nod_tail->next = nod;
        context->nod_tail = nod;
    }

    context->stat.nod_num++;

    return QUE_OK;
}

int queue_peek(queue_context_t *context, void *data, size_t *size)
{
    if (context == NULL || data == NULL && size == NULL)
    {
        return QUE_ERR_BAD_ARG;
    }

    if (context->stat.nod_num == 0)
    {
        return QUE_ERR_EMPTY_QUE;
    }

    if (data != NULL)
    {
        memcpy(data, context->nod_head->data, context->nod_head->size);
    }

    if (size != NULL)
    {
        *size = context->nod_head->size;
    }

    return QUE_OK;
}

int queue_dequeue(queue_context_t *context, void *data, size_t *size)
{
    queue_node_t *nod;

    if (context == NULL || data == NULL && size == NULL)
    {
        return QUE_ERR_BAD_ARG;
    }

    if (context->stat.nod_num == 0)
    {
        return QUE_ERR_EMPTY_QUE;
    }

    if (data != NULL)
    {
        memcpy(data, context->nod_head->data, context->nod_head->size);
    }

    if (size != NULL)
    {
        *size = context->nod_head->size;
    }

    nod = context->nod_head;
    context->nod_head = nod->next;
    if (context->nod_tail == nod)
    {
        context->nod_tail = NULL;
    }
    queue_node_delete(nod);

    if (context->nod_head == NULL)
    {
        context->stat.nhdata_size = 0;
    }
    else
    {
        context->stat.nhdata_size = context->nod_head->size; 
    }
    context->stat.nod_num--;

    return QUE_OK;
}

int queue_drop(queue_context_t *context)
{
    queue_node_t *nod;

    if (context == NULL)
    {
        return QUE_ERR_BAD_ARG;
    }

    if (context->stat.nod_num == 0)
    {
        return QUE_ERR_EMPTY_QUE;
    }

    nod = context->nod_head;
    context->nod_head = nod->next;
    if (context->nod_tail == nod)
    {
        context->nod_tail = NULL;
    }
    queue_node_delete(nod);

    if (context->nod_head == NULL)
    {
        context->stat.nhdata_size = 0;
    }
    else
    {
        context->stat.nhdata_size = context->nod_head->size; 
    }
    context->stat.nod_num--;

    return QUE_OK;
}
