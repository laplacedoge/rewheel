#ifndef __UTF8_CODEC_H__
#define __UTF8_CODEC_H__

#include <stddef.h>
#include <stdint.h>

typedef enum utf8dec_err
{
    UTF8DEC_OK  = 0,
    UTF8DEC_ERR = -1000,
    UTF8DEC_ERR_BAD_ARG,
    UTF8DEC_ERR_BAD_CP_VAL,     // Bad code point value
    UTF8DEC_ERR_DIRTY_DATA,     // Dirty data was encountered
} utf8dec_err_t;

typedef enum utf8dec_flag
{
    UTF8DEC_IGNORE_DIRTY_DATA  = 0x00000001,
} utf8dec_flag_t;

int utf8_decode(void *buff, const void *data, size_t size, size_t *decoded_num, size_t *last_pos, int flags);

#endif
