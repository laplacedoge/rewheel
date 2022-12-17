/**
 * BSD 3-Clause License
 * 
 * Copyright (c) 2022, Alex Chen
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "stream.h"

#include <stdlib.h>
#include <string.h>

#ifdef STM_PTHREAD_LOCK_ENABLE
    #define STM_MUTEX_LOCK(stream)      pthread_mutex_lock(&stream->mutex)
    #define STM_MUTEX_UNLOCK(stream)    pthread_mutex_unlock(&stream->mutex)
#else
    #define STM_MUTEX_LOCK(stream)
    #define STM_MUTEX_UNLOCK(stream)
#endif

#define STM_DEF_CAP 1024

static const stream_config_t def_conf =
{
    .cap = STM_DEF_CAP,
};

int stream_create(stream_t **stream, const stream_config_t *config)
{
    stream_t *alloc_stream;
    uint8_t *buff;
    const stream_config_t *real_conf;
    int ret;

    if (stream == NULL)
    {
        return STM_ERR_BAD_ARG;
    }

    alloc_stream = (stream_t *)malloc(sizeof(stream_t));
    if (alloc_stream == NULL)
    {
        return STM_ERR_NO_MEM;
    }

    if (config != NULL)
    {
        real_conf = config;
    }
    else
    {
        real_conf = &def_conf;
    }

    alloc_stream->head = 0;
    alloc_stream->tail = 0;
    alloc_stream->size = ((real_conf->cap >> 3) + 1) << 3;
    alloc_stream->cap = real_conf->cap;

    alloc_stream->stat.cap = alloc_stream->cap;
    alloc_stream->stat.free = alloc_stream->stat.cap;
    alloc_stream->stat.used = 0;

#ifdef STM_PTHREAD_LOCK_ENABLE
    ret = pthread_mutex_init(&alloc_stream->mutex, NULL);
    if (ret != 0)
    {
        ret = STM_ERR_BAD_MUTEX;
        goto err_exit;
    }
#endif

    buff = (uint8_t *)malloc(alloc_stream->size);
    if (buff == NULL)
    {
        ret = STM_ERR_NO_MEM;
        goto err_exit;
    }
    alloc_stream->buff = buff;
    *stream = alloc_stream;

    return STM_OK;

err_exit:
    free(alloc_stream);
    return ret;
}

int stream_delete(stream_t *stream)
{
    int ret;

    if (stream == NULL)
    {
        return STM_ERR_BAD_ARG;
    }

#ifdef STM_PTHREAD_LOCK_ENABLE
    ret = pthread_mutex_destroy(&stream->mutex);
    if (ret != 0)
    {
        return STM_ERR_BAD_MUTEX;
    }
#endif

    free(stream->buff);
    free(stream);

    return STM_OK;
}

/**
 * @brief get stream status info.
 * 
 * @param stream  stream stream pointer.
 * @param status  stream status pointer.
*/
int stream_status(stream_t *stream, stream_status_t *status)
{
    if (stream == NULL || status == NULL)
    {
        return STM_ERR_BAD_ARG;
    }

    STM_MUTEX_LOCK(stream);

    memcpy(status, &stream->stat, sizeof(stream_status_t));

    STM_MUTEX_UNLOCK(stream);

    return STM_OK;
}

int stream_write(stream_t *stream, const void *buff, size_t size)
{
    uint8_t *first_copy_ptr;

    if (stream == NULL || buff == NULL)
    {
        return STM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return STM_OK;
    }

    if (stream->stat.free < size)
    {
        return STM_ERR_INSUF_SPACE;
    }

    STM_MUTEX_LOCK(stream);

    first_copy_ptr = stream->buff + stream->tail;

    /* check if we can write it all at once */
    if (stream->cap + 1 - stream->tail >= size)
    {
        memcpy(first_copy_ptr, buff, size);
        stream->tail = (stream->tail + size) % (stream->cap + 1);
    }
    else
    {
        size_t first_copy_size = stream->cap + 1 - stream->tail;
        size_t second_copy_size = size - first_copy_size;

        /* first copy */
        memcpy(first_copy_ptr, buff, first_copy_size);

        /* second copy */
        memcpy(stream->buff, (uint8_t *)buff + first_copy_size, second_copy_size);

        stream->tail = second_copy_size;
    }

    stream->stat.used += size;
    stream->stat.free = stream->stat.cap - stream->stat.used;

    STM_MUTEX_UNLOCK(stream);

    return STM_OK;
}

int stream_read(stream_t *stream, void *buff, size_t size)
{
    uint8_t *first_copy_ptr;

    if (stream == NULL || buff == NULL)
    {
        return STM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return STM_OK;
    }

    if (stream->stat.used < size)
    {
        return STM_ERR_INSUF_DATA;
    }

    STM_MUTEX_LOCK(stream);

    first_copy_ptr = (uint8_t *)stream->buff + stream->head;

    /* check if we can read it all at once */
    if (stream->cap + 1 - stream->head >= size)
    {
        memcpy(buff, first_copy_ptr, size);
        stream->head = (stream->head + size) % (stream->cap + 1);
    }
    else
    {
        size_t first_copy_size = stream->cap + 1 - stream->head;
        size_t second_copy_size = size - first_copy_size;

        /* first copy */
        memcpy(buff, first_copy_ptr, first_copy_size);

        /* second copy */
        memcpy((uint8_t *)buff + first_copy_size, stream->buff, second_copy_size);

        stream->head = second_copy_size;
    }

    stream->stat.used -= size;
    stream->stat.free = stream->stat.cap - stream->stat.used;

    STM_MUTEX_UNLOCK(stream);

    return STM_OK;
}

/**
 * @brief just take a peek of the data in the stream.
 * 
 * @param stream  stream stream pointer.
 * @param buff    output data pointer.
 * @param offs    offset of the peeked data.
 * @param size    size of the peeked data.
*/
int stream_peek(stream_t *stream, void *buff, size_t offs, size_t size)
{
    uint8_t *first_copy_ptr;
    size_t temp_head;

    if (stream == NULL || buff == NULL)
    {
        return STM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return STM_OK;
    }

    if (stream->stat.used < offs + size)
    {
        return STM_ERR_INSUF_DATA;
    }

    STM_MUTEX_LOCK(stream);

    temp_head = (stream->head + offs) % (stream->cap + 1);

    first_copy_ptr = (uint8_t *)stream->buff + temp_head;

    /* check if we can read it all at once */
    if (stream->cap + 1 - temp_head >= size)
    {
        memcpy(buff, first_copy_ptr, size);
    }
    else
    {
        size_t first_copy_size = stream->cap + 1 - temp_head;
        size_t second_copy_size = size - first_copy_size;

        /* first copy */
        memcpy(buff, first_copy_ptr, first_copy_size);

        /* second copy */
        memcpy((uint8_t *)buff + first_copy_size, stream->buff, second_copy_size);
    }

    STM_MUTEX_UNLOCK(stream);

    return STM_OK;
}

int stream_drop(stream_t *stream, size_t size)
{
    uint8_t *first_copy_ptr;

    if (stream == NULL)
    {
        return STM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return STM_OK;
    }

    if (stream->stat.used < size)
    {
        return STM_ERR_INSUF_DATA;
    }

    STM_MUTEX_LOCK(stream);

    first_copy_ptr = (uint8_t *)stream->buff + stream->head;

    /* check if we can read it all at once */
    if (stream->cap + 1 - stream->head >= size)
    {
        stream->head = (stream->head + size) % (stream->cap + 1);
    }
    else
    {
        stream->head = size - (stream->cap + 1 - stream->head);
    }

    stream->stat.used -= size;
    stream->stat.free = stream->stat.cap - stream->stat.used;

    STM_MUTEX_UNLOCK(stream);

    return STM_OK;
}

/**
 * @brief discard all the data in the stream.
 * 
 * @param stream  stream stream pointer.
*/
int stream_discard(stream_t *stream)
{
    if (stream == NULL)
    {
        return STM_ERR_BAD_ARG;
    }

    STM_MUTEX_LOCK(stream);

    stream->head = 0;
    stream->tail = 0;

    stream->stat.used = 0;
    stream->stat.free = stream->stat.cap;

    STM_MUTEX_UNLOCK(stream);

    return STM_OK;
}
