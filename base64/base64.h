#ifndef __BASE64_H__
#define __BASE64_H__

#include <stdint.h>

typedef enum base64_err
{
    /* all is well. */
    BASE64_OK                       = 0,

    /* bad arguments. */
    BASE64_ERR_BAD_ARG              = -1,

    /* encounter invalid encoding character. */
    BASE64_ERR_INVALID_ENC_CHAR     = -2,

    /* the length of the encoded data is invalid. */
    BASE64_ERR_INVALID_ENC_LEN      = -3,

    /* the padding of the encoded data is invalid. */
    BASE64_ERR_INVALID_ENC_PADDING  = -4,
} base64_err_t;

uint32_t base64_error_param(void);

int base64_encode(void *buff, const void *data, int size);

int base64_decode(void *buff, const void *data, int size);

int base64_urlsafe_encode(void *buff, const void *data, int size);

int base64_urlsafe_decode(void *buff, const void *data, int size);

#endif
