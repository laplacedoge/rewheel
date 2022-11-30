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

#ifndef __SEEKABLE_STREAM_H__
#define __SEEKABLE_STREAM_H__

#include <stddef.h>
#include <stdint.h>

#ifdef SSTM_PTHREAD_MUTEX_ENABLE
#include <pthread.h>
#endif

typedef struct SeekableStreamStatus
{
    size_t size;    // the amount of memory space that buffer occupied.
    size_t cap;     // the size of memory that the buffer could actually use.
    size_t used;    // the size of used space in stream. 
    size_t free;    // the size of free(unused) space in the stream. 
    size_t stale;   // the size of data that has been read in used space within the stream.
    size_t fresh;   // the size of data that hasn't been read in used space within the stream.
} SeekableStreamStatus;

typedef struct SeekableStreamConfig
{
    size_t cap;     // stream capacity.
} SeekableStreamConfig;

typedef struct SeekableStream
{
    uint8_t *buff;                  // buffer pointer.
    size_t size;                    // the amount of memory space that buffer occupied.
    size_t cap;                     // the size of memory that the buffer could actually use.
    size_t head;                    // stream head index.
    size_t offset;                  // stream offset.
    size_t tail;                    // stream tail index.
    SeekableStreamConfig conf;      // stream config.
    SeekableStreamStatus stat;      // stream status.
#ifdef SSTM_PTHREAD_MUTEX_ENABLE
    pthread_mutex_t mutex;          // pthread mutex.
#endif
} SeekableStream;

typedef enum SeekableStreamError
{
    SSTM_OK                 = 0,    // all is well.
    SSTM_ERR                = -1,   // generic error type.
    SSTM_ERR_NO_MEM         = -2,   // failed to allocate memory.
    SSTM_ERR_BAD_ARG        = -3,   // invalid argument.
    SSTM_ERR_BAD_CONF       = -4,   // invalid stream configuration.
    SSTM_ERR_INSUF_SPACE    = -5,   // insufficient stream space.
    SSTM_ERR_INSUF_DATA     = -6,   // insufficient stream data.
    SSTM_ERR_BAD_MUTEX      = -7,   // problem about pthread mutex.
} SeekableStreamError;

typedef enum SeekableStreamSeek
{
    SSTM_SEEK_SET   = 0,    // from the start of the stream.
    SSTM_SEEK_CUR,          // from current position of the stream.
    SSTM_SEEK_END,          // from the end of the stream.
} SeekableStreamSeek;

int SeekableStream_Create(SeekableStream **stream, SeekableStreamConfig *config);

int SeekableStream_Delete(SeekableStream *stream);

int SeekableStream_Read(SeekableStream *stream, void *buff, size_t size);

int SeekableStream_Peek(SeekableStream *stream, void *buff, size_t size);

int SeekableStream_Drop(SeekableStream *stream, size_t size);

int SeekableStream_Dump(SeekableStream *stream, void *buff, size_t size);

int SeekableStream_Write(SeekableStream *stream, const void *buff, size_t size);

int SeekableStream_Seek(SeekableStream *stream, int offset, int whence);

#endif
