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

#include "SeekableStream.h"

#include <stdlib.h>
#include <string.h>

static int loadDefaultConfig(SeekableStreamConfig *config)
{
    if (config == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }

    config->cap = STM_DEF_CAP;

    return SSTM_OK;
}

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

    *stream = allocStream;

    return SSTM_OK;
}

int SeekableStream_Delete(SeekableStream *stream)
{
    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }

    free(stream);

    return SSTM_OK;
}

int SeekableStream_Read(SeekableStream *stream, void *buff, size_t size)
{
    uint8_t *firstCopyPtr;
    size_t headOffset;

    if (stream == NULL || buff == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    if (stream->stat.fresh < size)
    {
        return SSTM_ERR_INSUF_DATA;
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

    return SSTM_OK;
}

int SeekableStream_Peek(SeekableStream *stream, void *buff, size_t size)
{
    uint8_t *firstCopyPtr;
    size_t headOffset;

    if (stream == NULL || buff == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    if (stream->stat.fresh < size)
    {
        return SSTM_ERR_INSUF_DATA;
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

    return SSTM_OK;
}

int SeekableStream_Drop(SeekableStream *stream, size_t size)
{
    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    if (stream->stat.fresh < size)
    {
        return SSTM_ERR_INSUF_DATA;
    }

    stream->offset += size;

    stream->stat.stale += size;
    stream->stat.fresh -= size;

    return SSTM_OK;
}

int SeekableStream_Dump(SeekableStream *stream, void *buff, size_t size)
{
    uint8_t *firstCopyPtr;

    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    if (stream->stat.used < size)
    {
        return SSTM_ERR_INSUF_DATA;
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
    stream->stat.free =+ size;
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

    return SSTM_OK;
}

int SeekableStream_Write(SeekableStream *stream, const void *buff, size_t size)
{
    uint8_t *firstCopyPtr;

    if (stream == NULL || buff == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (size == 0)
    {
        return SSTM_OK;
    }

    if (stream->stat.free < size)
    {
        return SSTM_ERR_INSUF_SPACE;
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

    return SSTM_OK;
}

int SeekableStream_Seek(SeekableStream *stream, int offset, int whence)
{
    int finalOffset;

    if (stream == NULL)
    {
        return SSTM_ERR_BAD_ARG;
    }
    if (whence != SSTM_SEEK_SET && whence != SSTM_SEEK_CUR && whence != SSTM_SEEK_END)
    {
        return SSTM_OK;
    }

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
        return SSTM_OK;
    }
    if (finalOffset > stream->stat.used)
    {
        size_t size = finalOffset - stream->stat.used;

        if (size > stream->stat.free)
        {
            return SSTM_ERR_INSUF_SPACE;
        }
        stream->tail = (stream->tail + size) % (stream->cap + 1);
        stream->stat.used += size;
        stream->stat.free -= size;
    }
    stream->offset = finalOffset;
    stream->stat.stale = stream->offset;
    stream->stat.fresh = stream->stat.used - stream->stat.stale;

    return SSTM_OK;
}
