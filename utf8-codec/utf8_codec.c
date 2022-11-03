#include <stdio.h>

#include "utf8_codec.h"

int utf8_decode(void *buff, const void *data, size_t size, size_t *decoded_num, size_t *last_pos, int flags)
{
    int ret;
    int ignore_dirty_data;
    size_t rest;
    uint16_t code_point;
    uint16_t *buff_offs;
    const uint8_t *data_offs;

    if (buff == NULL || data == NULL)
    {
        return UTF8DEC_ERR_BAD_ARG;
    }

    if (size == 0)
    {
        if (decoded_num != NULL)
        {
            *decoded_num = 0;
        }
        if (last_pos != NULL)
        {
            *last_pos = 0;
        }
        return UTF8DEC_OK;
    }

    ignore_dirty_data = flags & UTF8DEC_IGNORE_DIRTY_DATA;

    rest = size;
    buff_offs = (uint16_t *)buff;
    data_offs = (const uint8_t *)data;
    while (rest >= 1)
    {
        if ((data_offs[0] & 0x80) == 0)
        {
            *buff_offs = (uint16_t)data_offs[0];
            rest -= 1;
            buff_offs++;
            data_offs += 1;
        }
        else
        {
            if (rest >= 2)
            {
                if ((data_offs[0] & 0xE0) == 0xC0 && (data_offs[1] & 0xC0) == 0x80)
                {
                    code_point = ((data_offs[0] & 0x1F) << 6) | (data_offs[1] & 0x3F);
                    if (code_point < 0x0080)
                    {
                        ret = UTF8DEC_ERR_BAD_CP_VAL;
                        goto exit;
                    }

                    *buff_offs = code_point;
                    rest -= 2;
                    buff_offs++;
                    data_offs += 2;
                }
                else
                {
                    if (rest >= 3)
                    {
                        if ((data_offs[0] & 0xF0) == 0xE0 && (data_offs[1] & 0xC0) == 0x80 && (data_offs[2] & 0xC0) == 0x80)
                        {
                            code_point = ((data_offs[0] & 0x0F) << 12) | ((data_offs[1] & 0x3F) << 6) | (data_offs[2] & 0x3F);
                            if (code_point < 0x0800)
                            {
                                ret = UTF8DEC_ERR_BAD_CP_VAL;
                                goto exit;
                            }

                            *buff_offs = code_point;
                            rest -= 3;
                            buff_offs++;
                            data_offs += 3;
                        }
                        else
                        {
                            if (ignore_dirty_data)
                            {
                                rest -= 1;
                                data_offs += 1;
                            }
                            else
                            {
                                ret = UTF8DEC_ERR_DIRTY_DATA;
                                goto exit;
                            }
                        }
                    }
                    else
                    {
                        ret = UTF8DEC_ERR_DIRTY_DATA;
                        goto exit;
                    }
                }
            }
            else
            {
                ret = UTF8DEC_ERR_DIRTY_DATA;
                goto exit;
            }
        }
    }

    ret = UTF8DEC_OK;

exit:
    if (decoded_num != NULL)
    {
        *decoded_num = buff_offs - (uint16_t *)buff;
    }
    if (last_pos != NULL)
    {
        *last_pos = data_offs - (uint8_t *)data;
    }
    return ret;
}
