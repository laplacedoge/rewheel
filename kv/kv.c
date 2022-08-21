#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "kv.h"

/* default hash callback function */
#define DEF_HASH_CB djb2

static uint32_t djb2(const char *str)
{
    uint32_t hash = 5381;
    int c;

    while (c = *str++)
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

/**
 * @brief create kv map with specified hash callback function.
 * @note  if 'hash_cb' is NULL, then this function will use default hash
 *        callback function(defined by marco DEF_HASH_CB) for this kv map.
 * 
 * @param map     address of kv map pointer.
 * @param hash_cb hash callback function pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
kv_res_t kv_create(kv_map_t **map, kv_hash_cb_t hash_cb)
{
    kv_res_t res;
    kv_map_t *inner_map;

    if (map == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    /* allocate memory space for map */
    inner_map = (kv_map_t *)malloc(sizeof(kv_map_t));
    if (inner_map == NULL)
    {
        res = KV_ERR_BAD_MEM_ALLOC;
        goto exit;
    }

    /* initialize map and its hash callback function */
    memset(inner_map, 0, sizeof(kv_map_t));
    if (hash_cb == NULL)
    {
        inner_map->hash = DEF_HASH_CB;
    }
    else
    {
        inner_map->hash = hash_cb;
    }

    *map = inner_map;
    res = KV_OK;
exit:
    return res;
}

/**
 * @brief destroy the kv map.
 * 
 * @param map kv map pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
kv_res_t kv_destroy(kv_map_t *map)
{
    kv_res_t res;

    if (map == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    res = kv_clear(map);
    if (res != KV_OK)
    {
        goto exit;
    }
    free(map);

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief get the number of key-value pairs in the kv map.
 * @note  the value pointed by 'size' won't update if this function doesn't
 *        return KV_OK.
 * 
 * @param map   kv map pointer.
 * @param size  pointer to variable for storing size.
 * @return  return KV_OK if success, otherwise return other value.
 */
kv_res_t kv_size(kv_map_t *map, int *size)
{
    kv_res_t res;

    if (map == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    *size = map->size;

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief check if key is in the kv map or not.
 * 
 * @param map kv map pointer.
 * @param key key string pointer.
 * @return  return KV_FALSE or KV_TRUE if success, otherwise return other value.
 */
kv_res_t kv_contain(kv_map_t *map, const char *key)
{
    kv_res_t res;
    int array_index;
    kv_bucket_t *curt_bucket;
    bool key_found;

    if (map == NULL || key == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    key_found = false;
    array_index = map->hash(key) % KV_MAP_ARRAY_SIZE;
    curt_bucket = map->array[array_index];

    /* seach this key on chain to check if it exists */
    while (curt_bucket != NULL)
    {
        if (strcmp(key, curt_bucket->key) == 0)
        {
            key_found = true;
            break;
        }
        curt_bucket = curt_bucket->next;
    }

    if (key_found)
    {
        res = KV_TRUE;
        goto exit;
    }
    else
    {
        res = KV_FALSE;
        goto exit;
    }

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief put a key-value pair in the kv map.
 * @note  this function will replace the value corresponding to the specified
 *        key with the specified value if the specified key already exists in
 *        this kv map.
 * 
 * @param map   kv map pointer.
 * @param key   key string pointer.
 * @param value value string pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
kv_res_t kv_put(kv_map_t *map, const char *key, const char *value)
{
    kv_res_t res;
    int array_index;
    kv_bucket_t **bucket_next;
    kv_bucket_t *curt_bucket;
    kv_bucket_t *new_bucket;
    bool key_found;
    char *inner_key;
    char *inner_value;
    size_t malloc_size;

    if (map == NULL || key == NULL || value == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    key_found = false;
    array_index = map->hash(key) % KV_MAP_ARRAY_SIZE;
    curt_bucket = map->array[array_index];
    bucket_next = map->array + array_index;

    /* seach this key on chain to check if it exists */
    while (curt_bucket != NULL)
    {
        if (strcmp(key, curt_bucket->key) == 0)
        {
            key_found = true;
            break;
        }
        curt_bucket = curt_bucket->next;
        bucket_next = &((*bucket_next)->next);
    }

    /* modify bucket or create new bucket */
    if (key_found)
    {
        malloc_size = strlen(value) + 1;
        inner_value = (char *)malloc(malloc_size);
        if (inner_value == NULL)
        {
            res = KV_ERR_BAD_MEM_ALLOC;
            goto exit;
        }
        memcpy(inner_value, value, malloc_size);

        free(curt_bucket->value);
        curt_bucket->value = inner_value;
    }
    else
    {
        /* create new bucket and be ready to be linked on the chain */
        new_bucket = (kv_bucket_t *)malloc(sizeof(kv_bucket_t));
        if (new_bucket == NULL)
        {
            goto err_malloc_bucket;
        }

        /* create the copy of the key */
        malloc_size = strlen(key) + 1;
        inner_key = (char *)malloc(malloc_size);
        if (inner_key == NULL)
        {
            goto err_malloc_key;
        }
        memcpy(inner_key, key, malloc_size);

        /* create the copy of the value */
        malloc_size = strlen(value) + 1;
        inner_value = (char *)malloc(malloc_size);
        if (inner_value == NULL)
        {
            goto err_malloc_value;
        }
        memcpy(inner_value, value, malloc_size);

        /* link every thing we just create */
        new_bucket->key = inner_key;
        new_bucket->value = inner_value;
        new_bucket->next = NULL;
        *bucket_next = new_bucket;

        map->size++;
    }

    return KV_OK;

err_malloc_value:
    free(inner_key);
err_malloc_key:
    free(new_bucket);
err_malloc_bucket:
    res = KV_ERR_BAD_MEM_ALLOC;
exit:
    return res;
}

/**
 * @brief delete a key-value pair in the kv map.
 * 
 * @param map kv map pointer.
 * @param key key string pointer.
 * @return  return KV_OK if success, or return KV_ERR_KEY_NOT_FOUND if the 
 *          specified key isn't found in kv map, otherwise return other value.
 */
kv_res_t kv_del(kv_map_t *map, const char *key)
{
    kv_res_t res;
    int array_index;
    kv_bucket_t **bucket_next;
    kv_bucket_t *curt_bucket;
    kv_bucket_t *next_bucket;
    bool key_found;

    if (map == NULL || key == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    key_found = false;
    array_index = map->hash(key) % KV_MAP_ARRAY_SIZE;
    curt_bucket = map->array[array_index];
    bucket_next = map->array + array_index;

    /* seach this key on chain to check if it exists */
    while (curt_bucket != NULL)
    {
        if (strcmp(key, curt_bucket->key) == 0)
        {
            key_found = true;
            break;
        }
        curt_bucket = curt_bucket->next;
        bucket_next = &((*bucket_next)->next);
    }

    if (key_found)
    {
        next_bucket = curt_bucket->next;
        free(curt_bucket->key);
        free(curt_bucket->value);
        free(curt_bucket);
        *bucket_next = next_bucket;
        map->size--;
    }
    else
    {
        res = KV_ERR_KEY_NOT_FOUND;
        goto exit;
    }

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief get the value string corresponding to the specified key in the kv map.
 * @note  the value pointed by 'value' won't update if this function doesn't
 *        return KV_OK.
 * 
 * @param map   kv map pointer.
 * @param key   key string pointer.
 * @param value pointer to a variable for storing value string pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
kv_res_t kv_get(kv_map_t *map, const char *key, const char **value)
{
    kv_res_t res;
    int array_index;
    kv_bucket_t *curt_bucket;
    bool key_found;

    if (map == NULL || key == NULL || value == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    key_found = false;
    array_index = map->hash(key) % KV_MAP_ARRAY_SIZE;
    curt_bucket = map->array[array_index];

    /* seach this key on chain to check if it exists */
    while (curt_bucket != NULL)
    {
        if (strcmp(key, curt_bucket->key) == 0)
        {
            key_found = true;
            break;
        }
        curt_bucket = curt_bucket->next;
    }

    if (!key_found)
    {
        res = KV_ERR_KEY_NOT_FOUND;
        goto exit;
    }

    *value = curt_bucket->value;

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief clear all key-value pairs in the kv map.
 * 
 * @param map kv map pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
kv_res_t kv_clear(kv_map_t *map)
{
    kv_res_t res;
    kv_bucket_t *curt_bucket;
    kv_bucket_t *next_bucket;

    if (map == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    /* free every chain on the map array */
    for (int i = 0; i < KV_MAP_ARRAY_SIZE; i++)
    {
        if (map->array[i] != NULL)
        {
            next_bucket = map->array[i];
            do
            {
                curt_bucket = next_bucket;
                next_bucket = curt_bucket->next;

                /* free the property of current bucket */
                free(curt_bucket->key);
                free(curt_bucket->value);

                free(curt_bucket);
            } while (next_bucket != NULL);
            map->array[i] = NULL;
        }
    }

    map->size = 0;

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief iterate all the key-value pairs in the kv map.
 * 
 * @param map         kv map pointer.
 * @param foreach_cb  pointer to iteration callback function.
 * @return  return KV_OK if success, otherwise return other value.
 */
kv_res_t kv_foreach(kv_map_t *map, kv_foreach_cb_t foreach_cb)
{
    kv_res_t res;
    kv_bucket_t *curt_bucket;

    if (map == NULL)
    {
        res = KV_ERR_INVALID_PARAM;
        goto exit;
    }

    /* free every chain on the map array */
    for (int i; i < KV_MAP_ARRAY_SIZE; i++)
    {
        if (map->array[i] != NULL)
        {
            curt_bucket = map->array[i];
            while (curt_bucket != NULL)
            {
                foreach_cb(curt_bucket->key, curt_bucket->value);
                curt_bucket = curt_bucket->next;
            }
        }
    }

    res = KV_OK;
exit:
    return res;
}
