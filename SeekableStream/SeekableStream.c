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

/**
 * stream buffer structure:
 * 
 * ┌──────────── allocated memory for stream buffer ────────────┐
 * │                                                            │
 * ┌───────────────── stream buffer ─────────────────┐          │
 * │                                                 │          │
 * ┌──────┐ ╔═══════════════╦═══════════════╗ ┌──────┐ ┌────────┐
 * │ free │ ║     stale     ║     fresh     ║ │ free │ │ unused │
 * └──────┘ ╚═══════════════╩═══════════════╝ └──────┘ └────────┘
 *          └───────────── used ────────────┘
 * └────────────────────── cap ──────────────────────┘
 * └─────────────────────────── size ───────────────────────────┘
 * 
 * the actual memory size allocated for the stream buffer must be a multiple of
 * 8. stream buffer is a FIFO, we call the unused part "free", and the used part
 * "used", "used" can be further divided in to "stale" and "fresh", "stale"
 * indicate the data which has been read, and "fresh" indicate the data which
 * hasn't been read.
 * 
 * SeekableStream_Write() will extend the size the "fresh" part (obviously the
 * "used" part will be extended as well) and shrink the "free" part (the "stale"
 * part won't be shrunk).
 * 
 * SeekableStream_Read() read data from the "fresh" part and then turns it into
 * the "stale" part, so the size of the "used" part won't be affected.
 * 
 * SeekableStream_Peek() read data from the "fresh" part but won't affect the
 * "stale" and "fresh" part.
 * 
 * SeekableStream_Drop() turns data in the "fresh" part into the "stale" part.
 * 
 * SeekableStream_Dump() read data from the "used" part and it will shrink the
 * "used" part.
 * 
 * SeekableStream_Seek() sets the separation position between the "stale" and
 * the "fresh" part.
 * 
*/

#include "SeekableStream.h"

#include <stdlib.h>
#include <string.h>

#ifdef SSTM_PTHREAD_MUTEX_ENABLE
    #define SSTM_MUTEX_LOCK(stream)     pthread_mutex_lock(&stream->mutex)
    #define SSTM_MUTEX_UNLOCK(stream)   pthread_mutex_unlock(&stream->mutex)
#else
    #define SSTM_MUTEX_LOCK(stream)
    #define SSTM_MUTEX_UNLOCK(stream)
#endif

#define SSTM_DEF_CAP    1024    // default capacity of stream buffer.

/**
 * @brief load default configuration.
*/
static int loadDefaultConfig(SeekableStreamConfig *config)
{
    if (config == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }

    config->cap = SSTM_DEF_CAP;

    return SSTM_OK;
}

/**
 * @brief create a seekable stream and initialize it.
 * @param stream  the pointer pointing to a stream pointer.
 * @param config  configuration pointer.
*/
int SeekableStream_Create(SeekableStream **stream, SeekableStreamConfig *config)
{
    SeekableStream *allocStream;
    uint8_t *allocBuff;

    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }

    /* allocate memory for stream. */
    allocStream = (SeekableStream *)malloc(sizeof(SeekableStream));
    if (allocStream == NULL)
    {
        return SSTM_ERR_NO_MEM;
    }

    /* configure stream. */
    if (config == NULL)
    {
        loadDefaultConfig(&allocStream->conf);
    }
    else
    {
        if (config->cap == 0)
        {
            free(allocStream);
            return SSTM_ERR_BAD_CONF;
        }
        memcpy(&allocStream->conf, config, sizeof(SeekableStreamConfig));
    }

    /* initialize stream. */
    allocStream->size = ((allocStream->conf.cap >> 3) + 1) << 3;
    allocStream->cap = allocStream->conf.cap;

    allocStream->head = 0;
    allocStream->offset = 0;
    allocStream->tail = 0;

    allocStream->stat.size = allocStream->size;
    allocStream->stat.cap = allocStream->cap;
    allocStream->stat.used = 0;
    allocStream->stat.free = allocStream->cap;
    allocStream->stat.stale = 0;
    allocStream->stat.fresh = 0;

    /* allocate memory for stream buffer. */
    allocBuff = (uint8_t *)malloc(allocStream->stat.size);
    if (allocBuff == NULL)
    {
        free(allocStream);
        return SSTM_ERR_NO_MEM;
    }
    allocStream->buff = allocBuff;

    /* initialize pthread mutex. */
#ifdef SSTM_PTHREAD_MUTEX_ENABLE
    if (pthread_mutex_init(&allocStream->mutex, NULL) != 0)
    {
        free(allocBuff);
        free(allocStream);
        return SSTM_ERR_BAD_MUTEX;
    }
#endif

    *stream = allocStream;

    return SSTM_OK;
}

/**
 * @brief delete a seekable stream.
 * @param stream  stream pointer.
*/
int SeekableStream_Delete(SeekableStream *stream)
{
    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }

    /* deinitialize pthread mutex. */
#ifdef SSTM_PTHREAD_MUTEX_ENABLE
    if (pthread_mutex_destroy(&stream->mutex) != 0)
    {
        return SSTM_ERR_BAD_MUTEX;
    }
#endif

    /* deallocate memory space. */
    free(stream->buff);
    free(stream);

    return SSTM_OK;
}

/**
 * @brief read data from the stream.
 * @param stream  stream pointer.
 * @param buff    buffer pointer.
 * @param size    read size.
*/
int SeekableStream_Read(SeekableStream *stream, void *buff, size_t size)
{
    uint8_t *firstCopyPtr;
    size_t headOffset;
    int ret;

    if (stream == NULL || buff == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    SSTM_MUTEX_LOCK(stream);

    if (stream->stat.fresh < size)
    {
        ret = SSTM_ERR_INSUF_DATA;
        goto exit;
    }

    headOffset = (stream->head + stream->offset) % (stream->cap + 1);
    firstCopyPtr = (uint8_t *)stream->buff + headOffset;

    /* check if we can read it all at once. */
    if (stream->cap + 1 - headOffset >= size)
    {
        memcpy(buff, firstCopyPtr, size);
    }
    else
    {
        size_t firstCopySize = stream->cap + 1 - headOffset;
        size_t secondCopySize = size - firstCopySize;

        /* first copy. */
        memcpy(buff, firstCopyPtr, firstCopySize);

        /* second copy. */
        memcpy((uint8_t *)buff + firstCopySize, stream->buff, secondCopySize);
    }
    stream->offset += size;

    stream->stat.stale += size;
    stream->stat.fresh -= size;

    ret = SSTM_OK;

exit:
    SSTM_MUTEX_UNLOCK(stream);
    return ret;
}

/**
 * @brief peek data from the stream.
 * @param stream  stream pointer.
 * @param buff    buffer pointer.
 * @param size    peek size.
*/
int SeekableStream_Peek(SeekableStream *stream, void *buff, size_t size)
{
    uint8_t *firstCopyPtr;
    size_t headOffset;
    int ret;

    if (stream == NULL || buff == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    SSTM_MUTEX_LOCK(stream);

    if (stream->stat.fresh < size)
    {
        ret = SSTM_ERR_INSUF_DATA;
        goto exit;
    }

    headOffset = (stream->head + stream->offset) % (stream->cap + 1);
    firstCopyPtr = (uint8_t *)stream->buff + headOffset;

    /* check if we can read it all at once. */
    if (stream->cap + 1 - headOffset >= size)
    {
        memcpy(buff, firstCopyPtr, size);
    }
    else
    {
        size_t firstCopySize = stream->cap + 1 - headOffset;
        size_t secondCopySize = size - firstCopySize;

        /* first copy. */
        memcpy(buff, firstCopyPtr, firstCopySize);

        /* second copy. */
        memcpy((uint8_t *)buff + firstCopySize, stream->buff, secondCopySize);
    }

    ret = SSTM_OK;

exit:
    SSTM_MUTEX_UNLOCK(stream);
    return ret;
}

/**
 * @brief drop data from stream.
 * @param stream  stream pointer.
 * @param size    peek size.
*/
int SeekableStream_Drop(SeekableStream *stream, size_t size)
{
    int ret;

    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    SSTM_MUTEX_LOCK(stream);

    if (stream->stat.fresh < size)
    {
        ret = SSTM_ERR_INSUF_DATA;
        goto exit;
    }

    stream->offset += size;

    stream->stat.stale += size;
    stream->stat.fresh -= size;

    ret = SSTM_OK;

exit:
    SSTM_MUTEX_UNLOCK(stream);
    return ret;
}

/**
 * @brief dump data from the stream.
 * @param stream  stream pointer.
 * @param buff    buffer pointer.
 * @param size    dump size.
*/
int SeekableStream_Dump(SeekableStream *stream, void *buff, size_t size)
{
    uint8_t *firstCopyPtr;
    int ret;

    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    SSTM_MUTEX_LOCK(stream);

    if (stream->stat.used < size)
    {
        ret = SSTM_ERR_INSUF_DATA;
        goto exit;
    }

    firstCopyPtr = (uint8_t *)stream->buff + stream->head;

    /* check if we can dump it all at once. */
    if (stream->cap + 1 - stream->head >= size)
    {
        if (buff)
        {
            memcpy(buff, firstCopyPtr, size);
        }
        stream->head = (stream->head + size) % (stream->cap + 1);
    }
    else
    {
        size_t firstCopySize = stream->cap + 1 - stream->head;
        size_t secondCopySize = size - firstCopySize;

        if (buff)
        {
            /* first copy. */
            memcpy(buff, firstCopyPtr, firstCopySize);

            /* second copy. */
            memcpy((uint8_t *)buff + firstCopySize, stream->buff, secondCopySize);
        }

        stream->head = secondCopySize;
    }

    stream->stat.used -= size;
    stream->stat.free += size;
    if (size <= stream->offset)
    {
        stream->offset -= size;
        stream->stat.stale = stream->offset;
    }
    else
    {
        stream->offset = 0;
        stream->stat.stale = 0;
    }
    stream->stat.fresh = stream->stat.used - stream->offset;

    ret = SSTM_OK;

exit:
    SSTM_MUTEX_UNLOCK(stream);
    return ret;
}

/**
 * @brief write data to the stream.
 * @param stream  stream pointer.
 * @param buff    buffer pointer.
 * @param size    dump size.
*/
int SeekableStream_Write(SeekableStream *stream, const void *buff, size_t size)
{
    uint8_t *firstCopyPtr;
    int ret;

    if (stream == NULL || buff == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    SSTM_MUTEX_LOCK(stream);

    if (stream->stat.free < size)
    {
        ret = SSTM_ERR_INSUF_SPACE;
        goto exit;
    }

    firstCopyPtr = stream->buff + stream->tail;

    /* check if we can write it all at once. */
    if (stream->cap + 1 - stream->tail >= size)
    {
        memcpy(firstCopyPtr, buff, size);
        stream->tail = (stream->tail + size) % (stream->cap + 1);
    }
    else
    {
        size_t firstCopySize = stream->cap + 1 - stream->tail;
        size_t secondCopySize = size - firstCopySize;

        /* first copy. */
        memcpy(firstCopyPtr, buff, firstCopySize);

        /* second copy. */
        memcpy(stream->buff, (uint8_t *)buff + firstCopySize, secondCopySize);

        stream->tail = secondCopySize;
    }

    stream->stat.used += size;
    stream->stat.free = stream->stat.cap - stream->stat.used;
    stream->stat.fresh += size;

    ret = SSTM_OK;

exit:
    SSTM_MUTEX_UNLOCK(stream);
    return ret;
}

/**
 * @brief reposition read offset within the stream.
 * @param stream  stream pointer.
 * @param offset  offset based on argument "whence".
 * @param whence  whence.
*/
int SeekableStream_Seek(SeekableStream *stream, int offset, int whence)
{
    int finalOffset;
    int ret;

    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (whence != SSTM_SEEK_SET && whence != SSTM_SEEK_CUR && whence != SSTM_SEEK_END)
    {
        return SSTM_OK;
    }

    SSTM_MUTEX_LOCK(stream);

    switch (whence)
    {
    case SSTM_SEEK_SET:
        finalOffset = offset;
        break;
    case SSTM_SEEK_CUR:
        finalOffset = stream->offset + offset;
        break;
    case SSTM_SEEK_END:
        finalOffset = stream->stat.used + offset;
        break;
    }

    if (finalOffset < 0)
    {
        finalOffset = 0;
    }
    if (finalOffset == stream->offset)
    {
        ret = SSTM_OK;
        goto exit;
    }
    if (finalOffset > stream->stat.used)
    {
        size_t size = finalOffset - stream->stat.used;

        if (size > stream->stat.free)
        {
            ret = SSTM_ERR_INSUF_SPACE;
            goto exit;
        }
        stream->tail = (stream->tail + size) % (stream->cap + 1);
        stream->stat.used += size;
        stream->stat.free -= size;
    }
    stream->offset = finalOffset;
    stream->stat.stale = stream->offset;
    stream->stat.fresh = stream->stat.used - stream->stat.stale;

    ret = SSTM_OK;

exit:
    SSTM_MUTEX_UNLOCK(stream);
    return ret;
}
