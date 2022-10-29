#include "stream.h"

#include <stdlib.h>
#include <string.h>

static int stream_config_load_default(stream_config_t *config)
{
    if (config == NULL)
    {
        return STM_ERR_BAD_ARG;
    }

    config->is_circbuff_static = 0;
    config->circbuff = NULL;
    config->circbuff_cap = STM_DEF_CIRCBUFF_CAP;

    return STM_OK;
}

int stream_create(stream_handle_t **handle, stream_config_t *config)
{
    stream_handle_t *hd;
    uint8_t * buff;

    if (handle == NULL)
    {
        return STM_ERR_BAD_ARG;
    }

    hd = (stream_handle_t *)malloc(sizeof(stream_handle_t));
    if (hd == NULL)
    {
        return STM_ERR_NO_MEM;
    }

    if (config == NULL)
    {
        stream_config_load_default(&hd->conf);
    }
    else
    {
        if (config->circbuff_cap == 0)
        {
            free(hd);
            return STM_ERR_BAD_CONF;
        }
        memcpy(&hd->conf, config, sizeof(stream_config_t));
    }

    hd->circbuff_head = 0;
    hd->circbuff_tail = 0;
    hd->circbuff_size = ((hd->conf.circbuff_cap >> 3) + 1) << 3;
    hd->circbuff_cap = hd->conf.circbuff_cap;

    hd->stat.cap = hd->circbuff_cap;
    hd->stat.free = hd->stat.cap;
    hd->stat.used = 0;

    /* staticly using circular buffer or dynamicly */
    if (hd->conf.is_circbuff_static == 1)
    {
        if (hd->conf.circbuff == NULL)
        {
            free(hd);
            return STM_ERR_BAD_CONF;
        }
        hd->circbuff = hd->conf.circbuff;
    }
    else
    {
        buff = (uint8_t *)malloc(hd->circbuff_size);
        if (buff == NULL)
        {
            free(hd);
            return STM_ERR_NO_MEM;
        }
        hd->circbuff = buff;
    }
    *handle = hd;

    return STM_OK;
}

int stream_delete(stream_handle_t *handle)
{
    if (handle == NULL)
    {
        return STM_ERR_BAD_ARG;
    }

    free(handle->circbuff);
    free(handle);

    return STM_OK;
}

int stream_write(stream_handle_t *handle, const void *buff, size_t size)
{
    uint8_t *first_copy_ptr;

    if (handle == NULL || buff == NULL)
    {
        return STM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return STM_OK;
    }

    if (STM_GET_FREE(handle) < size)
    {
        return STM_ERR_INSUF_SPACE;
    }

    first_copy_ptr = handle->circbuff + handle->circbuff_tail;

    /* check if we can write it all at once */
    if (handle->circbuff_cap + 1 - handle->circbuff_tail >= size)
    {
        memcpy(first_copy_ptr, buff, size);
        handle->circbuff_tail = (handle->circbuff_tail + size) % (handle->circbuff_cap + 1);
    }
    else
    {
        size_t first_copy_size = handle->circbuff_cap + 1 - handle->circbuff_tail;
        size_t second_copy_size = size - first_copy_size;

        /* first copy */
        memcpy(first_copy_ptr, buff, first_copy_size);

        /* second copy */
        memcpy(handle->circbuff, (uint8_t *)buff + first_copy_size, second_copy_size);

        handle->circbuff_tail = second_copy_size;
    }

    handle->stat.used += size;
    handle->stat.free = handle->stat.cap - handle->stat.used;

    return STM_OK;
}

int stream_read(stream_handle_t *handle, void *buff, size_t size)
{
    uint8_t *first_copy_ptr;

    if (handle == NULL || buff == NULL)
    {
        return STM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return STM_OK;
    }

    if (STM_GET_USED(handle) < size)
    {
        return STM_ERR_INSUF_DATA;
    }

    first_copy_ptr = (uint8_t *)handle->circbuff + handle->circbuff_head;

    /* check if we can read it all at once */
    if (handle->circbuff_cap + 1 - handle->circbuff_head >= size)
    {
        memcpy(buff, first_copy_ptr, size);
        handle->circbuff_head = (handle->circbuff_head + size) % (handle->circbuff_cap + 1);
    }
    else
    {
        size_t first_copy_size = handle->circbuff_cap + 1 - handle->circbuff_head;
        size_t second_copy_size = size - first_copy_size;

        /* first copy */
        memcpy(buff, first_copy_ptr, first_copy_size);

        /* second copy */
        memcpy((uint8_t *)buff + first_copy_size, handle->circbuff, second_copy_size);

        handle->circbuff_head = second_copy_size;
    }

    handle->stat.used -= size;
    handle->stat.free = handle->stat.cap - handle->stat.used;

    return STM_OK;
}

int stream_peek(stream_handle_t *handle, void *buff, size_t size)
{
    uint8_t *first_copy_ptr;

    if (handle == NULL || buff == NULL)
    {
        return STM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return STM_OK;
    }

    if (STM_GET_USED(handle) < size)
    {
        return STM_ERR_INSUF_DATA;
    }

    first_copy_ptr = (uint8_t *)handle->circbuff + handle->circbuff_head;

    /* check if we can read it all at once */
    if (handle->circbuff_cap + 1 - handle->circbuff_head >= size)
    {
        memcpy(buff, first_copy_ptr, size);
    }
    else
    {
        size_t first_copy_size = handle->circbuff_cap + 1 - handle->circbuff_head;
        size_t second_copy_size = size - first_copy_size;

        /* first copy */
        memcpy(buff, first_copy_ptr, first_copy_size);

        /* second copy */
        memcpy((uint8_t *)buff + first_copy_size, handle->circbuff, second_copy_size);
    }

    return STM_OK;
}

int stream_drop(stream_handle_t *handle, size_t size)
{
    uint8_t *first_copy_ptr;

    if (handle == NULL)
    {
        return STM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return STM_OK;
    }

    if (STM_GET_USED(handle) < size)
    {
        return STM_ERR_INSUF_DATA;
    }

    first_copy_ptr = (uint8_t *)handle->circbuff + handle->circbuff_head;

    /* check if we can read it all at once */
    if (handle->circbuff_cap + 1 - handle->circbuff_head >= size)
    {
        handle->circbuff_head = (handle->circbuff_head + size) % (handle->circbuff_cap + 1);
    }
    else
    {
        handle->circbuff_head = size - (handle->circbuff_cap + 1 - handle->circbuff_head);
    }

    handle->stat.used -= size;
    handle->stat.free = handle->stat.cap - handle->stat.used;

    return STM_OK;
}

