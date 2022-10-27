#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stddef.h>
#include <stdint.h>

typedef struct queue_node {
    struct queue_node *next;    // next queue node in the queue
    void *data;                 // data pointer of this node
    size_t size;                // data size of this node
} queue_node_t;

typedef struct queue_config {
    size_t ndsize_max;          // limit the maximum of node data size
    size_t nodnum_max;          // limit the maximum of node number
} queue_config_t;

typedef struct queue_status {
    size_t nod_num;             // the number of nodes
    size_t nhdata_size;         // the data size of the head node
} queue_status_t;

typedef struct queue_context {
    queue_node_t *nod_head;     // head node
    queue_node_t *nod_tail;     // tail node
    queue_config_t conf;        // queue configuration
    queue_status_t stat;        // queue status
} queue_context_t;

typedef enum queue_error
{
    QUE_OK = 0,
    QUE_ERR = -1000,
    QUE_ERR_NO_MEM,
    QUE_ERR_BAD_ARG,
    QUE_ERR_BAD_CONF,
    QUE_ERR_FULL_QUE,
    QUE_ERR_EMPTY_QUE,
    QUE_ERR_OVERLONG_NDATA,
} queue_error_t;

#define QUE_DEF_NDSIZE_MAX      1024
#define QUE_DEF_NODNUM_MAX      1024

int queue_create(queue_context_t **context, queue_config_t *config);

int queue_delete(queue_context_t *context);

int queue_status(queue_context_t *context, queue_status_t *status);

int queue_enqueue(queue_context_t *context, const void *data, size_t size);

int queue_peek(queue_context_t *context, void *data, size_t *size);

int queue_dequeue(queue_context_t *context, void *data, size_t *size);

int queue_drop(queue_context_t *context);

#endif
