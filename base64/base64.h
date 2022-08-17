#ifndef __BASE64_H__
#define __BASE64_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum
{
    BASE64_OK                       = 0,    // all is well
    BASE64_ERR_INVALID_ENC_CHAR     = -1,   // encounter invalid encoding character
    BASE64_ERR_INVALID_ENC_LEN      = -2,   // the length of the encoded data is invalid
    BASE64_ERR_INVALID_ENC_PADDING  = -3,   // the padding of the encoded data is invalid
} base64_err_t;

uint32_t base64_error_param(void);

int base64_encode(void *out, const void *in, int len);

int base64_decode(void *out, const void *in, int len);

#endif
