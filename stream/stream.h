#ifndef __STREAM_H__
#define __STREAM_H__

#include <stddef.h>
#include <stdint.h>

typedef struct stream_config {
    size_t circbuff_cap;
} stream_config_t;

typedef struct stream_status {
    size_t cap;
    size_t free;
    size_t used;
} stream_status_t;

typedef struct stream_handle {
    uint8_t *circ_buff;
    size_t circbuff_size;
    size_t circbuff_cap;
    size_t circbuff_head;
    size_t circbuff_tail;
    stream_config_t conf;
    stream_status_t stat;
} stream_handle_t;

typedef enum stream_error
{
    STM_OK = 0,
    STM_ERR = -1000,
    STM_ERR_NO_MEM,
    STM_ERR_BAD_ARG,
    STM_ERR_BAD_CONF,
    STM_ERR_INSUF_SPACE,
    STM_ERR_INSUF_DATA,
} stream_error_t;

#define STM_DEF_CIRCBUFF_CAP    1024

#define STM_GET_USED(hd)        (hd->stat.used)

#define STM_GET_FREE(hd)        (hd->circbuff_cap - hd->stat.used)

int stream_create(stream_handle_t **handle, stream_config_t *config);

int stream_delete(stream_handle_t *handle);

int stream_write(stream_handle_t *handle, const void *buff, size_t size);

int stream_read(stream_handle_t *handle, void *buff, size_t size);

int stream_peek(stream_handle_t *handle, void *buff, size_t size);

int stream_drop(stream_handle_t *handle, size_t size);

#endif
