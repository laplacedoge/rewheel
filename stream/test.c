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
    stream_handle_t *handle = NULL;
    stream_config_t config;
    uint8_t buff[128];
    size_t capacity;
    int ret;

    printf("\n>>> TEST: try to peek some data\n");

    capacity = 64;

    config.is_circbuff_static = 0;
    config.circbuff_cap = capacity;

    ret = stream_create(&handle, &config);
    assert(ret == STM_OK);

    ret = stream_write(handle, data_numeric, size_numeric);
    assert(ret == STM_OK);
    assert(handle->stat.used == size_numeric);
    assert(handle->stat.free == capacity - size_numeric);

    ret = stream_peek(handle, buff, 0, size_numeric);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_numeric, size_numeric) == 0);

    ret = stream_peek(handle, buff, 1, size_numeric - 1);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_numeric + 1, size_numeric - 1) == 0);

    ret = stream_write(handle, data_alphabet_lower, size_alphabet_lower);
    assert(ret == STM_OK);
    assert(handle->stat.used == size_numeric + size_alphabet_lower);
    assert(handle->stat.free == capacity - size_numeric - size_alphabet_lower);

    ret = stream_peek(handle, buff, 0, size_numeric);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_numeric, size_numeric) == 0);

    ret = stream_peek(handle, buff, size_numeric, size_alphabet_lower);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_alphabet_lower, size_alphabet_lower) == 0);

    ret = stream_drop(handle, size_numeric);
    assert(ret == STM_OK);

    ret = stream_write(handle, data_alphabet_upper, size_alphabet_upper);
    assert(ret == STM_OK);
    assert(handle->stat.used == size_alphabet_lower + size_alphabet_lower);
    assert(handle->stat.free == capacity - size_alphabet_lower - size_alphabet_upper);

    ret = stream_peek(handle, buff, size_alphabet_lower, size_alphabet_upper);
    assert(ret == STM_OK);
    assert(memcmp(buff, data_alphabet_upper, size_alphabet_upper) == 0);

    ret = stream_discard(handle);
    assert(ret == STM_OK);
    assert(handle->stat.used == 0);
    assert(handle->stat.free == capacity);

    ret = stream_delete(handle);
    assert(ret == STM_OK);

    printf("<<< PASS\n");
}

int main(int argc, char *argv[])
{
    test_peeking();

    return 0;
}
