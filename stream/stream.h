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

#ifndef __STREAM_H__
#define __STREAM_H__

#include <stddef.h>
#include <stdint.h>

#ifdef STM_PTHREAD_LOCK_ENABLE
#include <pthread.h>
#endif

typedef struct stream_config
{
    size_t cap;
} stream_config_t;

typedef struct stream_status
{
    size_t cap;     // stream capacity
    size_t free;    // the amount of free space available in the stream
    size_t used;    // the amount of already used space in the stream
} stream_status_t;

typedef struct stream
{
    uint8_t *buff;
    size_t size;
    size_t cap;
    size_t head;
    size_t tail;
    stream_status_t stat;
#ifdef STM_PTHREAD_LOCK_ENABLE
    pthread_mutex_t mutex;
#endif
} stream_t;

typedef enum stream_error
{
    STM_OK                  = 0,    // all is well.
    STM_ERR                 = -1,   // generic error type.
    STM_ERR_NO_MEM          = -2,   // failed to allocate memory.
    STM_ERR_BAD_ARG         = -3,   // invalid argument.
    STM_ERR_BAD_CONF        = -4,   // invalid stream configuration.
    STM_ERR_INSUF_SPACE     = -5,   // insufficient stream space.
    STM_ERR_INSUF_DATA      = -6,   // insufficient stream data.
    STM_ERR_BAD_MUTEX       = -7,   // problem about pthread mutex.
} stream_error_t;

int stream_create(stream_t **stream, const stream_config_t *config);

int stream_delete(stream_t *stream);

int stream_status(stream_t *stream, stream_status_t *status);

int stream_write(stream_t *stream, const void *buff, size_t size);

int stream_read(stream_t *stream, void *buff, size_t size);

int stream_peek(stream_t *stream, void *buff, size_t offs, size_t size);

int stream_drop(stream_t *stream, size_t size);

int stream_discard(stream_t *stream);

#endif
