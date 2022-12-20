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

#include "stream.h"

static const char data_numeric[] = "0123456789";
static const char data_alphabet_lower[] = "abcdefghijklmnopqrstuvwxyz";
static const char data_alphabet_upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const size_t size_numeric = sizeof(data_numeric) - 1;
static const size_t size_alphabet_lower = sizeof(data_alphabet_lower) - 1;
static const size_t size_alphabet_upper = sizeof(data_alphabet_upper) - 1;

void test_peeking(void)
{
    stream_t *stream = NULL;
    stream_config_t config;
    uint8_t buff[128];
    size_t capacity;
    int ret;

    printf("\n>>> TEST: try to peek some data\n");

    capacity = 64;

    config.cap = capacity;

    ret = stream_create(&stream, &config);
    assert(ret == STM_OK);

    ret = stream_write(stream, data_numeric, size_numeric);
    assert(ret == STM_OK);
    assert(stream->stat.used == size_numeric);
    assert(stream->stat.free == capacity - size_numeric);

    ret = stream_peek(stream, buff, 0, size_numeric);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_numeric, size_numeric) == 0);

    ret = stream_peek(stream, buff, 1, size_numeric - 1);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_numeric + 1, size_numeric - 1) == 0);

    ret = stream_write(stream, data_alphabet_lower, size_alphabet_lower);
    assert(ret == STM_OK);
    assert(stream->stat.used == size_numeric + size_alphabet_lower);
    assert(stream->stat.free == capacity - size_numeric - size_alphabet_lower);

    ret = stream_peek(stream, buff, 0, size_numeric);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_numeric, size_numeric) == 0);

    ret = stream_peek(stream, buff, size_numeric, size_alphabet_lower);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_alphabet_lower, size_alphabet_lower) == 0);

    ret = stream_drop(stream, size_numeric);
    assert(ret == STM_OK);

    ret = stream_write(stream, data_alphabet_upper, size_alphabet_upper);
    assert(ret == STM_OK);
    assert(stream->stat.used == size_alphabet_lower + size_alphabet_lower);
    assert(stream->stat.free == capacity - size_alphabet_lower - size_alphabet_upper);

    ret = stream_peek(stream, buff, size_alphabet_lower, size_alphabet_upper);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_alphabet_upper, size_alphabet_upper) == 0);

    ret = stream_discard(stream);
    assert(ret == STM_OK);
    assert(stream->stat.used == 0);
    assert(stream->stat.free == capacity);

    ret = stream_delete(stream);
    assert(ret == STM_OK);

    printf("<<< PASS\n");
}

void test_reading_lines(void)
{
    stream_t *stream = NULL;
    stream_config_t config;
    char buff[32];
    size_t size;
    int ret;

    printf("\n>>> TEST: try to read some lines\n");

    config.cap = 64;
    ret = stream_create(&stream, &config);
    assert(ret == STM_OK);

    ret = stream_write(stream, "I can't read ya\nMy sexy Mona Lisa\r\n", 35);
    assert(ret == STM_OK);

    size = 32;
    ret = stream_readline(stream, buff, &size);
    assert(ret == STM_OK);
    assert(size == 16);
    assert(memcmp(buff, "I can't read ya\n", 16) == 0);

    size = 32;
    ret = stream_readline(stream, buff, &size);
    assert(ret == STM_OK);
    assert(size == 19);
    assert(memcmp(buff, "My sexy Mona Lisa\r\n", 19) == 0);

    ret = stream_write(stream, "Can't tell if you gon' leave here\r\nOr if you wanna stay", 55);
    assert(ret == STM_OK);

    size = 32;
    ret = stream_readline(stream, buff, &size);
    assert(ret == STM_ERR_INSUF_SPACE);
    ret = stream_drop(stream, 35);
    assert(ret == STM_OK);

    size = 32;
    ret = stream_readline(stream, buff, &size);
    assert(ret == STM_ERR_INSUF_DATA);

    ret = stream_write(stream, "\n", 1);
    assert(ret == STM_OK);

    size = 32;
    ret = stream_readline(stream, buff, &size);
    assert(ret == STM_OK);
    assert(size == 21);
    assert(memcmp(buff, "Or if you wanna stay\n", 21) == 0);

    ret = stream_delete(stream);
    assert(ret == STM_OK);

    printf("<<< PASS\n");
}

int main(int argc, char *argv[])
{
    test_peeking();

    test_reading_lines();

    return 0;
}
