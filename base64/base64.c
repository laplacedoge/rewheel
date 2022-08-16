#include "base64.h"

#define IS_IN_ALPHABET(ch)  ('A' <= (ch) && (ch) <= 'Z' || 'a' <= (ch) && (ch) <= 'z' || '0' <= (ch) && (ch) <= '9' || (ch) == '+' || (ch) == '/')
#define IS_PADDING_CHAR(ch) ((ch) == '=')

#define BASE64_LEN_CONV_RAW2ENC(len)    ((len + 2) / 3 * 4)
#define BASE64_LEN_CONV_ENC2RAW(len)    (len / 4 * 3)

static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_encode(void *out, void *in, int len)
{
    uint8_t *__in;
    uint32_t *__out;
    int __len;
    uint8_t buff[4];

    __in = (uint8_t *)in;
    __out = (uint32_t *)out;
    __len = len;
    while(__len)
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
    return BASE64_LEN_CONV_RAW2ENC(len);
}

int base64_decode(void *out, void *in, int len)
{
}
