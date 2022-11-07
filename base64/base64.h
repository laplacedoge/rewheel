#ifndef __BASE64_H__
#define __BASE64_H__

#include <stdint.h>

typedef enum base64_err
{
    /* all is well. */
    BASE64_OK                       = 0,

    /* encounter invalid encoding character. */
    BASE64_ERR_INVALID_ENC_CHAR     = -1,

    /* the length of the encoded data is invalid. */
    BASE64_ERR_INVALID_ENC_LEN      = -2,

    /* the padding of the encoded data is invalid. */
    BASE64_ERR_INVALID_ENC_PADDING  = -3,
} base64_err_t;

uint32_t base64_error_param(void);

int base64_encode(void *buff, const void *data, int size);

int base64_decode(void *buff, const void *data, int size);

#endif
