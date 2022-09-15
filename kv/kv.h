#ifndef __KV_H__
#define __KV_H__

#include <stdint.h>

#define KV_MAP_ARRAY_SIZE   128

typedef uint32_t (* kv_hash_cb_t)(const char *);
typedef void (* kv_foreach_cb_t)(void *, const char *, const char *);

typedef enum kv_res
{
    /* boolean types */
    KV_FALSE                = 0,
    KV_TRUE                 = 1,

    /* error types */
    KV_OK                   = -1,
    KV_ERR_INVALID_PARAM    = -2,
    KV_ERR_BAD_MEM_ALLOC    = -3,
    KV_ERR_KEY_NOT_FOUND    = -4,
} kv_res_t;

typedef struct kv_bucket
{
    /* key string(null-terminated) */
    char *key;

    /* value string(null-terminated) */
    char *value;

    /* point to the next bucket on chain */
    struct kv_bucket *next;
} kv_bucket_t;

typedef struct kv_map
{
    /* number of the key-value pairs in this map */
    int size;

    /* hash callback function pointer */
    kv_hash_cb_t hash;

    /* store all the bucket chains of this map */
    kv_bucket_t *array[KV_MAP_ARRAY_SIZE];
} kv_map_t;

kv_res_t kv_create(kv_map_t **map, kv_hash_cb_t hash_cb);

kv_res_t kv_destroy(kv_map_t *map);

kv_res_t kv_contain(kv_map_t *map, const char *key);

kv_res_t kv_size(kv_map_t *map, int *size);

kv_res_t kv_put(kv_map_t *map, const char *key, const char *value);

kv_res_t kv_del(kv_map_t *map, const char *key);

kv_res_t kv_get(kv_map_t *map, const char *key, const char ** value);

kv_res_t kv_clear(kv_map_t *map);

kv_res_t kv_foreach(kv_map_t *map, kv_foreach_cb_t foreach_cb, void *arg);

#endif
