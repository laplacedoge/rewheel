#ifndef __KV_H__
#define __KV_H__

#include <stdint.h>

typedef uint32_t (* kv_hash_cb_t)(const char *);
typedef void (* kv_foreach_cb_t)(void *, const char *, const char *);

enum
{
    /* boolean types */
    KV_FALSE                = 0,
    KV_TRUE                 = 1,

    /* error types */
    KV_OK                   = 0,
    KV_ERR_BAD_ARG          = -1,
    KV_ERR_BAD_MEM          = -2,
    KV_ERR_BAD_CONF         = -3,
    KV_ERR_KEY_NOT_FOUND    = -4,
};

/* structure used to store one key-value pair */
typedef struct kv_bucket
{
    /* key string(null-terminated) */
    char *key;

    /* value string(null-terminated) */
    char *value;

    /* point to the next bucket on chain */
    struct kv_bucket *next;
} kv_bucket_t;

/* structure used to configure kv set */
typedef struct kv_conf
{
    size_t bucket_num;
    kv_hash_cb_t hash_cb;
} kv_conf_t;

/* key-value set structure */
typedef struct kv_set
{
    /* number of the key-value pairs in this set */
    size_t pair_num;

    /* number of buckets in member 'array' */
    size_t bucket_num;

    /* hash callback function pointer */
    kv_hash_cb_t hash;

    /* store all the bucket chains of this set */
    kv_bucket_t *array[];
} kv_set_t;

int kv_create(kv_set_t **set, const kv_conf_t *conf);

int kv_destroy(kv_set_t *set);

int kv_contain(kv_set_t *set, const char *key);

int kv_size(kv_set_t *set, size_t *size);

int kv_put(kv_set_t *set, const char *key, const char *value);

int kv_del(kv_set_t *set, const char *key);

int kv_get(kv_set_t *set, const char *key, const char ** value);

int kv_clear(kv_set_t *set);

int kv_foreach(kv_set_t *set, kv_foreach_cb_t foreach_cb, void *arg);

#endif
