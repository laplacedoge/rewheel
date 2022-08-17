#include "base64.h"

#define IS_IN_ALPHABET(ch)  ('A' <= (ch) && (ch) <= 'Z' || 'a' <= (ch) && (ch) <= 'z' || '0' <= (ch) && (ch) <= '9' || (ch) == '+' || (ch) == '/')
#define IS_PADDING_CHAR(ch) ((ch) == '=')

/* covert the length of the raw data to the length of the encoded data */
#define CONV_LEN_RAW2ENC(len)       ((len + 2) / 3 * 4)

/* covert the length of the encoded data to the length of the raw data */
#define CONV_LEN_ENC2RAW(len, pad)  ((len / 4 - 1) * 3 + ((pad == 0) ? 3 : (3 - pad)))

/* a table for converting the index value to the ASCII value of the specified base64 digit character */
static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* the offset value of the 'reverse_tab' */
#define REVERSE_TAB_OFFSET  43

/* a table for converting the ASCII value to the index value of the specified base64 digit character,
   note that this table is offset from the original ASCII table */
static const uint8_t reverse_tab[] =
{
    /* digit '+' */
    0x3E,

    0x00, 0x00, 0x00,

    /* digit '/' */
    0x3F,

    /* digit '0'-'9' */
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* digit 'A'-'Z' */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* digit 'a'-'z' */
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
};

/* a macro function for converting the ASCII value to the index value of the specified base64 digit character */
#define reverse(ch) (reverse_tab[(ch) - REVERSE_TAB_OFFSET])

/* store the parameter of current error, the meaning of this value varies depending on the type of the error */
static uint32_t error_param;

/* get the parameter of current error */
uint32_t base64_error_param(void)
{
    return error_param;
}

/**
 * @brief encode raw data using base64
 * 
 * @param out   encoded data buffer pointer
 * @param in    raw data buffer pointer
 * @param len   length of raw data(in bytes)
 * @return      the length of the encoded data
 */
int base64_encode(void *out, const void *in, int len)
{
    uint8_t *__in;
    uint32_t *__out;
    int __len;
    uint8_t buff[4];

    __in = (uint8_t *)in;
    __out = (uint32_t *)out;
    __len = len;
    while (__len)
    {
        if (__len >= 3)
        {
            buff[0] = alphabet[__in[0] >> 2];
            buff[1] = alphabet[((__in[0] & 0x03) << 4) | ((__in[1] & 0xF0) >> 4)];
            buff[2] = alphabet[((__in[1] & 0x0F) << 2) | ((__in[2] & 0xC0) >> 6)];
            buff[3] = alphabet[__in[2] & 0x3F];
            __len -= 3;
            __in += 3;
        }
        else
        {
            switch (__len % 3)
            {
            case 2:
                buff[0] = alphabet[__in[0] >> 2];
                buff[1] = alphabet[((__in[0] & 0x03) << 4) | ((__in[1] & 0xF0) >> 4)];
                buff[2] = alphabet[((__in[1] & 0x0F) << 2) | ((__in[2] & 0xC0) >> 6)];
                buff[3] = '=';
                __len -= 2;
                __in += 2;
                break;
            case 1:
                buff[0] = alphabet[__in[0] >> 2];
                buff[1] = alphabet[((__in[0] & 0x03) << 4) | ((__in[1] & 0xF0) >> 4)];
                buff[2] = '=';
                buff[3] = '=';
                __len -= 1;
                __in += 1;
                break;
            }
        }
        memcpy(__out, &buff, 4);
        __out += 1;
    }
    return CONV_LEN_RAW2ENC(len);
}

/**
 * @brief validate the validation of the specified encoded data
 * 
 * @param data  encoded data buffer pointer
 * @param len   length of the encoded data
 * @return      see base64_err_t
 */
static int validate_encoded_data(const void *data, int len)
{
    bool prev;          // is previous character a data character(non-padding character)
    int paddings;       // number of continuous padding characters
    int first_padding;  // offset of the first padding character occurred in data
    int flips;          // flipping times between data character sequence and padding character sequence
    char *__data;       // a copy of the function parameter 'data'
    int __len;          // a copy of the function parameter 'len'

    if (len == 0)
    {
        return BASE64_OK;
    }
    if (len % 4 != 0)
    {
        return BASE64_ERR_INVALID_ENC_LEN;
    }
    flips = 0;
    paddings = 0;
    first_padding = 0;
    __len = len;
    __data = (char *)data;
    if (!IS_IN_ALPHABET(*__data))
    {
        error_param = 0;
        if (IS_PADDING_CHAR(*__data))
        {
            return BASE64_ERR_INVALID_ENC_PADDING;
        }
        else
        {
            return BASE64_ERR_INVALID_ENC_CHAR;
        }
    }
    prev = true;
    __data++;
    __len--;
    while (__len)
    {
        if (IS_IN_ALPHABET(*__data))
        {
            paddings = 0;
            if (!prev)
            {
                prev = true;
                flips++;
            }
        }
        else if (IS_PADDING_CHAR(*__data))
        {
            if (first_padding == 0)
            {
                first_padding = __data - (char *)data;
            }
            paddings++;
            if (prev)
            {
                prev = false;
                flips++;
            }
        }
        else
        {
            error_param = __data - (char *)data;
            return BASE64_ERR_INVALID_ENC_CHAR;
        }
        __data++;
        __len--;
    }
    if (flips > 1)
    {
        error_param = first_padding;
        return BASE64_ERR_INVALID_ENC_PADDING;
    }
    if (paddings > 2)
    {
        error_param = __data - (char *)data - paddings;
        return BASE64_ERR_INVALID_ENC_PADDING;
    }
    error_param = paddings;
    return BASE64_OK;
}

/**
 * @brief decode base64-encoded data
 * 
 * @param out   raw data buffer pointer
 * @param in    encoded data buffer pointer
 * @param len   length of encoded data(in bytes)
 * @return      return negative number if encountering error when decoding,
 *              otherwise the returned value represent the length of the
 *              decoded raw data
 */
int base64_decode(void *out, const void *in, int len)
{
    int ret;                // return value
    int paddings;           // the number of padding characters in encoded data
    uint8_t *__out;         // a copy of the function parameter 'out'
    char *__in;             // a copy of the function parameter 'in'
    int __len;              // a copy of the function parameter 'len'
    uint8_t enc_buff[4];    // buffer to store the index value of base64 digits in partial encoded data
    uint8_t raw_buff[3];    // buffer to store the decoded data of partial encoded data

    if (len == 0)
    {
        return 0;
    }
    ret = validate_encoded_data(in, len);
    if (ret != BASE64_OK)
    {
        return ret;
    }
    __out =  (uint8_t *)out;
    __in = (char *)in;
    paddings = error_param;
    if (paddings == 0)
    {
        __len = len;
    }
    else
    {
        __len = len - 4;
    }
    while(__len)
    {
        enc_buff[0] = reverse(__in[0]);
        enc_buff[1] = reverse(__in[1]);
        enc_buff[2] = reverse(__in[2]);
        enc_buff[3] = reverse(__in[3]);
        raw_buff[0] = (enc_buff[0] << 2) | ((enc_buff[1] & 0x30) >> 4);
        raw_buff[1] = ((enc_buff[1] & 0x0F) << 4) | ((enc_buff[2] & 0x3C) >> 2);
        raw_buff[2] = ((enc_buff[2] & 0x03) << 6) | enc_buff[3];
        memcpy(__out, &raw_buff, 3);
        __in += 4;
        __out += 3;
        __len -= 4;
    }
    if (paddings == 0)
    {
        return CONV_LEN_ENC2RAW(len, 0);
    }
    switch (paddings)
    {
    case 2:
        enc_buff[0] = reverse(__in[0]);
        enc_buff[1] = reverse(__in[1]);
        raw_buff[0] = (enc_buff[0] << 2) | ((enc_buff[1] & 0x30) >> 4);
        memcpy(__out, &raw_buff, 1);
        break;
    case 1:
        enc_buff[0] = reverse(__in[0]);
        enc_buff[1] = reverse(__in[1]);
        enc_buff[2] = reverse(__in[2]);
        raw_buff[0] = (enc_buff[0] << 2) | ((enc_buff[1] & 0x30) >> 4);
        raw_buff[1] = ((enc_buff[1] & 0x0F) << 4) | ((enc_buff[2] & 0x3C) >> 2);
        memcpy(__out, &raw_buff, 2);
        break;
    }
    return CONV_LEN_ENC2RAW(len, paddings);
}
