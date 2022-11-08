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

#include <stdbool.h>
#include <string.h>

#include "base64.h"

#define BASE64_PADDING_DIGIT        ('=')

/* check if it's a base64 character. */
#define IS_BASE64_DIGIT(ch)         ('A' <= (ch) && (ch) <= 'Z' || \
                                     'a' <= (ch) && (ch) <= 'z' || \
                                     '0' <= (ch) && (ch) <= '9' || \
                                     (ch) == '+' || (ch) == '/')

/* check if it's a url-safe base64 character. */
#define IS_BASE64_URLSAFE_DIGIT(ch) ('A' <= (ch) && (ch) <= 'Z' || \
                                     'a' <= (ch) && (ch) <= 'z' || \
                                     '0' <= (ch) && (ch) <= '9' || \
                                     (ch) == '-' || (ch) == '_')

#define IS_BASE64_PADDING_DIGIT(ch) ((ch) == BASE64_PADDING_DIGIT)

/* covert the size of the raw data to the size of the encoded data */
#define CONV_SIZE_RAW2ENC(size)     ((size + 2) / 3 * 4)

/* covert the size of the encoded data to the size of the raw data */
#define CONV_SIZE_ENC2RAW(size, np) ((size / 4 - 1) * 3 + ((np == 0) ? \
                                     3 : (3 - np)))

typedef enum base64_type
{
    BASE64_STANDARD = 1,
    BASE64_URLSAFE  = 2,
} base64_type_t;

/* a table for converting the index value to the ASCII value of
   the specified base64 digit character. */
static const char base64_digit_tab[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* same table as above but used for url-safe base64. */
static const char base64_urlsafe_digit_tab[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/* the offset of the 'base64_rev_tab'. */
#define BASE64_REV_TAB_OFFS         43

/* a table for converting the ASCII value to the index value of
   the specified base64 digit character, note that this table
   is offset from the original ASCII table. */
static const uint8_t base64_rev_tab[] =
{
    /* digit '+' */
    0x3E,

    0x00,

    /* digit '-', used for url-safe base64 */
    0x3E, 

    0x00,

    /* digit '/' */
    0x3F,

    /* digit '0'-'9' */
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* digit 'A'-'Z' */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19,

    0x00, 0x00, 0x00, 0x00,

    /* digit '_', used for url-safe base64 */
    0x3F,

    0x00,

    /* digit 'a'-'z' */
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x32, 0x33,
};

/* a macro function for converting the ASCII value to the index value of
   the specified base64 digit character. */
#define base64_reverse_digit(ch)    (base64_rev_tab[(ch) - BASE64_REV_TAB_OFFS])

/* store the parameter of current error, the meaning of this value varies
   depending on the type of the error. */
static uint32_t error_param;

/* get the parameter of current error. */
uint32_t base64_error_param(void)
{
    return error_param;
}

/**
 * @brief encode raw data by using specified digit table.
 * 
 * @param buff      encoded data buffer pointer.
 * @param data      raw data buffer pointer.
 * @param size      size of raw data(in bytes).
 * @param digit_tab digit table pointer.
 * @return      the size of the encoded data.
 */
static int base64_encode_by_digit_tab(void *buff, const void *data, int size,
                                      const char *digit_tab)
{
    /* raw data offset pointer. */
    const uint8_t *data_offs;

    /* encoded data offset pointer. */
    uint32_t *buff_offs;

    /* size of reset of the unchecked raw data. */
    int rest_size;

    /* buffer used to temporarily store 4 base64 digits. */
    uint8_t digit_buff[4];

    data_offs = (const uint8_t *)data;
    buff_offs = (uint32_t *)buff;
    rest_size = size;

    while (rest_size > 0)
    {
        if (rest_size >= 3)
        {
            digit_buff[0] = digit_tab[data_offs[0] >> 2];
            digit_buff[1] = digit_tab[((data_offs[0] & 0x03) << 4) |
                                      ((data_offs[1] & 0xF0) >> 4)];
            digit_buff[2] = digit_tab[((data_offs[1] & 0x0F) << 2) |
                                      ((data_offs[2] & 0xC0) >> 6)];
            digit_buff[3] = digit_tab[data_offs[2] & 0x3F];
            rest_size -= 3;
            data_offs += 3;
        }
        else
        {
            switch (rest_size % 3)
            {
            case 2:
                digit_buff[0] = digit_tab[data_offs[0] >> 2];
                digit_buff[1] = digit_tab[((data_offs[0] & 0x03) << 4) |
                                          ((data_offs[1] & 0xF0) >> 4)];
                digit_buff[2] = digit_tab[((data_offs[1] & 0x0F) << 2) |
                                          ((data_offs[2] & 0xC0) >> 6)];
                digit_buff[3] = BASE64_PADDING_DIGIT;
                rest_size -= 2;
                data_offs += 2;
                break;
            case 1:
                digit_buff[0] = digit_tab[data_offs[0] >> 2];
                digit_buff[1] = digit_tab[((data_offs[0] & 0x03) << 4) |
                                          ((data_offs[1] & 0xF0) >> 4)];
                digit_buff[2] = BASE64_PADDING_DIGIT;
                digit_buff[3] = BASE64_PADDING_DIGIT;
                rest_size -= 1;
                data_offs += 1;
                break;
            }
        }

        memcpy(buff_offs, &digit_buff, 4);
        buff_offs += 1;
    }

    return CONV_SIZE_RAW2ENC(size);
}

/**
 * @brief encode raw data by using base64.
 * 
 * @param buff  encoded data buffer pointer.
 * @param data  raw data buffer pointer.
 * @param size  size of raw data(in bytes).
 * @return      the size of the encoded data.
 */
int base64_encode(void *buff, const void *data, int size)
{
    if (buff == NULL || data == NULL)
    {
        return BASE64_ERR_BAD_ARG;
    }

    if (size == 0)
    {
        return 0;
    }

    return base64_encode_by_digit_tab(buff, data, size, base64_digit_tab);
}

/**
 * @brief encode raw data by using url-safe base64.
 * 
 * @param buff  encoded data buffer pointer.
 * @param data  raw data buffer pointer.
 * @param size  size of raw data(in bytes).
 * @return      the size of the encoded data.
 */
int base64_urlsafe_encode(void *buff, const void *data, int size)
{
    if (buff == NULL || data == NULL)
    {
        return BASE64_ERR_BAD_ARG;
    }

    if (size == 0)
    {
        return 0;
    }

    return base64_encode_by_digit_tab(buff, data, size, base64_urlsafe_digit_tab);
}

/**
 * @brief validate the specified encoded data.
 * 
 * @param data        encoded data buffer pointer.
 * @param size        size of the encoded data.
 * @param base64_type base64 type.
 * @return      see the definition of base64_err_t.
 */
static int validate_encoded_data(const void *data, int size, int base64_type)
{
    bool result;

    /* is previous character a data character(non-padding character). */
    bool is_prevchar_data;

    /* number of continuous padding characters. */
    int num_padding;

    /* index of the first padding character encountered in encoded data. */
    int first_padding_idx;

    /* number of the flipping times between data character sequence
       and padding character sequence. */
    int num_flipping;

    /* encoded data offset pointer. */
    const char *data_offs;

    /* size of reset of the unchecked encoded data. */
    int rest_size;

    if (size == 0)
    {
        return BASE64_OK;
    }

    if (size % 4 != 0)
    {
        return BASE64_ERR_BAD_ENC_SIZE;
    }

    num_flipping = 0;
    num_padding = 0;
    first_padding_idx = 0;
    rest_size = size;
    data_offs = (const char *)data;

    if (base64_type == BASE64_STANDARD)
    {
        result = !IS_BASE64_DIGIT(*data_offs);
    }
    else if (base64_type == BASE64_URLSAFE)
    {
        result = !IS_BASE64_URLSAFE_DIGIT(*data_offs);
    }
    if (result)
    {
        error_param = 0;
        if (IS_BASE64_PADDING_DIGIT(*data_offs))
        {
            return BASE64_ERR_BAD_ENC_PADDING;
        }
        else
        {
            return BASE64_ERR_BAD_ENC_CHAR;
        }
    }

    is_prevchar_data = true;
    data_offs++;
    rest_size--;

    if (base64_type == BASE64_STANDARD)
    {
        while (rest_size > 0)
        {
            if (IS_BASE64_DIGIT(*data_offs))
            {
                num_padding = 0;
                if (!is_prevchar_data)
                {
                    is_prevchar_data = true;
                    num_flipping++;
                }
            }
            else if (IS_BASE64_PADDING_DIGIT(*data_offs))
            {
                if (first_padding_idx == 0)
                {
                    first_padding_idx = data_offs - (char *)data;
                }
                num_padding++;
                if (is_prevchar_data)
                {
                    is_prevchar_data = false;
                    num_flipping++;
                }
            }
            else
            {
                error_param = data_offs - (char *)data;
                return BASE64_ERR_BAD_ENC_CHAR;
            }

            data_offs++;
            rest_size--;
        }
    }
    else if (base64_type == BASE64_URLSAFE)
    {
        while (rest_size > 0)
        {
            if (IS_BASE64_URLSAFE_DIGIT(*data_offs))
            {
                num_padding = 0;
                if (!is_prevchar_data)
                {
                    is_prevchar_data = true;
                    num_flipping++;
                }
            }
            else if (IS_BASE64_PADDING_DIGIT(*data_offs))
            {
                if (first_padding_idx == 0)
                {
                    first_padding_idx = data_offs - (char *)data;
                }
                num_padding++;
                if (is_prevchar_data)
                {
                    is_prevchar_data = false;
                    num_flipping++;
                }
            }
            else
            {
                error_param = data_offs - (char *)data;
                return BASE64_ERR_BAD_ENC_CHAR;
            }

            data_offs++;
            rest_size--;
        }
    }

    if (num_flipping > 1)
    {
        error_param = first_padding_idx;
        return BASE64_ERR_BAD_ENC_PADDING;
    }

    if (num_padding > 2)
    {
        error_param = data_offs - (char *)data - num_padding;
        return BASE64_ERR_BAD_ENC_PADDING;
    }

    error_param = num_padding;

    return BASE64_OK;
}

/**
 * @brief decode base64-encoded data by specified base64 type.
 * 
 * @param buff        decoded data buffer pointer.
 * @param data        encoded data buffer pointer.
 * @param size        size of encoded data(in bytes).
 * @param base64_type base64 type.
 * @return      return negative number if encountering error when decoding,
 *              otherwise the returned value represents the size of the
 *              decoded data.
 */
static int base64_decode_by_base64_type(void *buff, const void *data, int size,
                                        int base64_type)
{
    int ret;

    /* number of continuous padding characters. */
    int num_padding;

    /* decoded data offset pointer. */
    uint8_t *buff_offs;

    /* encoded data offset pointer. */
    const char *data_offs;

    /* size of reset of the unchecked encoded data. */
    int rest_size;

    /* buffer used to temporarily store 4 index values of base64 digits. */
    uint8_t idx_buff[4];

    /* buffer used to temporarily store 3bytes of the decoded data. */
    uint8_t raw_buff[3];

    if (size == 0)
    {
        return 0;
    }

    ret = validate_encoded_data(data, size, base64_type);
    if (ret != BASE64_OK)
    {
        return ret;
    }

    buff_offs =  (uint8_t *)buff;
    data_offs = (const char *)data;

    num_padding = error_param;
    if (num_padding == 0)
    {
        rest_size = size;
    }
    else
    {
        rest_size = size - 4;
    }

    while(rest_size > 0)
    {
        idx_buff[0] = base64_reverse_digit(data_offs[0]);
        idx_buff[1] = base64_reverse_digit(data_offs[1]);
        idx_buff[2] = base64_reverse_digit(data_offs[2]);
        idx_buff[3] = base64_reverse_digit(data_offs[3]);
        raw_buff[0] = (idx_buff[0] << 2) | ((idx_buff[1] & 0x30) >> 4);
        raw_buff[1] = ((idx_buff[1] & 0x0F) << 4) | ((idx_buff[2] & 0x3C) >> 2);
        raw_buff[2] = ((idx_buff[2] & 0x03) << 6) | idx_buff[3];
        memcpy(buff_offs, &raw_buff, 3);
        data_offs += 4;
        buff_offs += 3;
        rest_size -= 4;
    }

    if (num_padding == 0)
    {
        return CONV_SIZE_ENC2RAW(size, 0);
    }

    switch (num_padding)
    {
    case 2:
        idx_buff[0] = base64_reverse_digit(data_offs[0]);
        idx_buff[1] = base64_reverse_digit(data_offs[1]);
        raw_buff[0] = (idx_buff[0] << 2) | ((idx_buff[1] & 0x30) >> 4);
        memcpy(buff_offs, &raw_buff, 1);
        break;
    case 1:
        idx_buff[0] = base64_reverse_digit(data_offs[0]);
        idx_buff[1] = base64_reverse_digit(data_offs[1]);
        idx_buff[2] = base64_reverse_digit(data_offs[2]);
        raw_buff[0] = (idx_buff[0] << 2) | ((idx_buff[1] & 0x30) >> 4);
        raw_buff[1] = ((idx_buff[1] & 0x0F) << 4) | ((idx_buff[2] & 0x3C) >> 2);
        memcpy(buff_offs, &raw_buff, 2);
        break;
    }

    return CONV_SIZE_ENC2RAW(size, num_padding);
}

/**
 * @brief decode base64 encoded data.
 * 
 * @param buff  decoded data buffer pointer.
 * @param data  encoded data buffer pointer.
 * @param size  size of encoded data(in bytes).
 * @return      return negative number if encountering error when decoding,
 *              otherwise the returned value represents the size of the
 *              decoded data.
 */
int base64_decode(void *buff, const void *data, int size)
{
    if (buff == NULL || data == NULL)
    {
        return BASE64_ERR_BAD_ARG;
    }

    if (size == 0)
    {
        return 0;
    }

    return base64_decode_by_base64_type(buff, data, size, BASE64_STANDARD);
}

/**
 * @brief decode url-safe base64 encoded data.
 * 
 * @param buff  decoded data buffer pointer.
 * @param data  encoded data buffer pointer.
 * @param size  size of encoded data(in bytes).
 * @return      return negative number if encountering error when decoding,
 *              otherwise the returned value represents the size of the
 *              decoded data.
 */
int base64_urlsafe_decode(void *buff, const void *data, int size)
{
    if (buff == NULL || data == NULL)
    {
        return BASE64_ERR_BAD_ARG;
    }

    if (size == 0)
    {
        return 0;
    }

    return base64_decode_by_base64_type(buff, data, size, BASE64_URLSAFE);
}
