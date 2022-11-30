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

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "SeekableStream.h"

static const char data_numeric[] = "0123456789";
static const char data_alphabet_lower[] = "abcdefghijklmnopqrstuvwxyz";
static const char data_alphabet_upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const size_t size_numeric = sizeof(data_numeric) - 1;
static const size_t size_alphabet_lower = sizeof(data_alphabet_lower) - 1;
static const size_t size_alphabet_upper = sizeof(data_alphabet_upper) - 1;

void test_reading_and_writing(void)
{
    SeekableStream *stream = NULL;
    SeekableStreamConfig config;
    uint8_t buff[128];
    size_t capacity;
    int ret;

    printf("\n>>> TEST: try to read and write some data\n");

    capacity = 64;

    config.cap = capacity;

    ret = SeekableStream_Create(&stream, &config);
    assert(ret == SSTM_OK);
    assert(stream->head == 0);
    assert(stream->tail == 0);
    assert(stream->offset == 0);
    assert(stream->stat.cap == capacity);
    assert(stream->stat.used == 0);
    assert(stream->stat.free == capacity);
    assert(stream->stat.stale == 0);
    assert(stream->stat.fresh == 0);

    ret = SeekableStream_Write(stream, data_numeric, size_numeric);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric);
    assert(stream->stat.free == capacity - size_numeric);
    assert(stream->offset == 0);

    ret = SeekableStream_Read(stream, buff, size_numeric);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric);
    assert(stream->stat.free == capacity - size_numeric);
    assert(stream->offset == size_numeric);
    assert(memcmp(buff, data_numeric, size_numeric) == 0);

    ret = SeekableStream_Read(stream, buff, size_numeric);
    assert(ret == SSTM_ERR_INSUF_DATA);

    ret = SeekableStream_Seek(stream, -size_numeric - 99, SSTM_SEEK_CUR);
    assert(ret == SSTM_OK);

    ret = SeekableStream_Read(stream, buff, size_numeric);
    assert(ret == SSTM_OK);
    assert(memcmp(buff, data_numeric, size_numeric) == 0);

    ret = SeekableStream_Write(stream, data_alphabet_lower, size_alphabet_lower);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric + size_alphabet_lower);
    assert(stream->stat.free == capacity - size_numeric - size_alphabet_lower);
    assert(stream->offset == size_numeric);

    ret = SeekableStream_Write(stream, data_alphabet_lower, size_alphabet_lower);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric + size_alphabet_lower * 2);
    assert(stream->stat.free == capacity - size_numeric - size_alphabet_lower * 2);
    assert(stream->offset == size_numeric);

    ret = SeekableStream_Write(stream, data_alphabet_lower, size_alphabet_lower);
    assert(ret == SSTM_ERR_INSUF_SPACE);

    ret = SeekableStream_Delete(stream);
    assert(ret == SSTM_OK);

    printf("<<< PASS\n");
}

void test_peeking_and_seeking(void)
{
    SeekableStream *stream = NULL;
    SeekableStreamConfig config;
    uint8_t buff[128];
    size_t capacity;
    int ret;

    printf("\n>>> TEST: try to peek and seek some data\n");

    capacity = 64;

    config.cap = capacity;

    ret = SeekableStream_Create(&stream, &config);
    assert(ret == SSTM_OK);
    assert(stream->head == 0);
    assert(stream->tail == 0);
    assert(stream->offset == 0);
    assert(stream->stat.cap == capacity);
    assert(stream->stat.used == 0);
    assert(stream->stat.free == capacity);
    assert(stream->stat.stale == 0);
    assert(stream->stat.fresh == 0);

    ret = SeekableStream_Write(stream, data_numeric, size_numeric);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric);
    assert(stream->stat.free == capacity - size_numeric);

    ret = SeekableStream_Peek(stream, buff, size_numeric);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric);
    assert(stream->stat.free == capacity - size_numeric);
    assert(memcmp(buff, data_numeric, size_numeric) == 0);

    ret = SeekableStream_Write(stream, data_alphabet_lower, size_alphabet_lower);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric + size_alphabet_lower);
    assert(stream->stat.free == capacity - size_numeric - size_alphabet_lower);

    ret = SeekableStream_Peek(stream, buff, size_numeric);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric + size_alphabet_lower);
    assert(stream->stat.free == capacity - size_numeric - size_alphabet_lower);
    assert(memcmp(buff, data_numeric, size_numeric) == 0);

    ret = SeekableStream_Seek(stream, size_numeric, SSTM_SEEK_CUR);
    assert(ret == SSTM_OK);
    assert(stream->offset == size_numeric);
    assert(stream->stat.stale == size_numeric);
    assert(stream->stat.fresh == size_alphabet_lower);

    ret = SeekableStream_Peek(stream, buff, size_alphabet_lower);
    assert(ret == SSTM_OK);
    assert(stream->offset == size_numeric);
    assert(stream->stat.stale == size_numeric);
    assert(stream->stat.fresh == size_alphabet_lower);
    assert(memcmp(buff, data_alphabet_lower, size_alphabet_lower) == 0);

    ret = SeekableStream_Write(stream, data_alphabet_upper, size_alphabet_upper);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric + size_alphabet_lower + size_alphabet_upper);
    assert(stream->stat.free == capacity - size_numeric - size_alphabet_lower - size_alphabet_upper);

    ret = SeekableStream_Peek(stream, buff, size_alphabet_lower);
    assert(ret == SSTM_OK);
    assert(stream->offset == size_numeric);
    assert(stream->stat.stale == size_numeric);
    assert(stream->stat.fresh == size_alphabet_lower + size_alphabet_upper);

    ret = SeekableStream_Seek(stream, -size_alphabet_upper, SSTM_SEEK_END);
    assert(ret == SSTM_OK);
    assert(stream->offset == size_numeric + size_alphabet_lower);
    assert(stream->stat.stale == size_numeric + size_alphabet_lower);
    assert(stream->stat.fresh == size_alphabet_upper);

    ret = SeekableStream_Peek(stream, buff, size_alphabet_upper);
    assert(ret == SSTM_OK);
    assert(memcmp(buff, data_alphabet_upper, size_alphabet_upper) == 0);

    ret = SeekableStream_Seek(stream, capacity - size_numeric - size_alphabet_lower - size_alphabet_upper, SSTM_SEEK_END);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == capacity);
    assert(stream->offset == capacity);

    ret = SeekableStream_Seek(stream, 10, SSTM_SEEK_END);
    assert(ret == SSTM_ERR_INSUF_SPACE);

    ret = SeekableStream_Delete(stream);
    assert(ret == SSTM_OK);

    printf("<<< PASS\n");
}

void test_dropping_and_dumping(void)
{
    SeekableStream *stream = NULL;
    SeekableStreamConfig config;
    uint8_t buff[128];
    size_t capacity;
    int ret;

    printf("\n>>> TEST: try to drop and dump some data\n");

    capacity = 64;

    config.cap = capacity;

    ret = SeekableStream_Create(&stream, &config);
    assert(ret == SSTM_OK);

    ret = SeekableStream_Write(stream, data_numeric, size_numeric);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric);
    assert(stream->stat.stale == 0);
    assert(stream->stat.fresh == size_numeric);

    ret = SeekableStream_Drop(stream, 3);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric);
    assert(stream->stat.stale == 3);
    assert(stream->stat.fresh == size_numeric - 3);

    ret = SeekableStream_Drop(stream, size_numeric);
    assert(ret == SSTM_ERR_INSUF_DATA);
    assert(stream->stat.used == size_numeric);
    assert(stream->stat.stale == 3);
    assert(stream->stat.fresh == size_numeric - 3);

    ret = SeekableStream_Dump(stream, NULL, 2);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == size_numeric - 2);
    assert(stream->stat.stale == 1);
    assert(stream->stat.fresh == size_numeric - 3);

    ret = SeekableStream_Dump(stream, buff, size_numeric - 2);
    assert(ret == SSTM_OK);
    assert(stream->stat.used == 0);
    assert(stream->stat.stale == 0);
    assert(stream->stat.fresh == 0);

    ret = SeekableStream_Dump(stream, buff, size_numeric);
    assert(ret == SSTM_ERR_INSUF_DATA);

    ret = SeekableStream_Delete(stream);
    assert(ret == SSTM_OK);

    printf("<<< PASS\n");
}

int main(int argc, char *argv[])
{
    test_reading_and_writing();

    test_peeking_and_seeking();

    test_dropping_and_dumping();

    return 0;
}
