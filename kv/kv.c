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

/* default configuration */
static const kv_conf_t def_conf =
{
    .hash_cb = djb2,
};

/**
 * @brief create kv set with specified hash callback function.
 * @note  if 'conf->hash_cb' is NULL, then this function will use default hash
 *        callback function(defined by marco DEF_HASH_CB) for this kv set.
 * 
 * @param set   address of kv set pointer.
 * @param conf  configuration pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
int kv_create(kv_set_t **set, const kv_conf_t *conf)
{
    kv_set_t *inner_set;
    const kv_conf_t *real_conf;

    if (set == NULL)
    {
        return KV_ERR_BAD_ARG;
    }

    /* allocate memory space for set */
    inner_set = (kv_set_t *)malloc(sizeof(kv_set_t));
    if (inner_set == NULL)
    {
        return KV_ERR_BAD_MEM;
    }

    /* initialize set and its hash callback function */
    memset(inner_set, 0, sizeof(kv_set_t));
    if (conf != NULL)
    {
        real_conf = conf;
    }
    else
    {
        real_conf = &def_conf;
    }
    if (real_conf->hash_cb == NULL)
    {
        inner_set->hash = DEF_HASH_CB;
    }
    else
    {
        inner_set->hash = real_conf->hash_cb;
    }

    *set = inner_set;

    return KV_OK;
}

/**
 * @brief destroy the kv set.
 * 
 * @param set kv set pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
int kv_destroy(kv_set_t *set)
{
    int res;

    if (set == NULL)
    {
        res = KV_ERR_BAD_ARG;
        goto exit;
    }

    res = kv_clear(set);
    if (res != KV_OK)
    {
        goto exit;
    }
    free(set);

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief get the number of key-value pairs in the kv set.
 * @note  the value pointed by 'size' won't update if this function doesn't
 *        return KV_OK.
 * 
 * @param set   kv set pointer.
 * @param size  pointer to variable for storing size.
 * @return  return KV_OK if success, otherwise return other value.
 */
int kv_size(kv_set_t *set, size_t *size)
{
    int res;

    if (set == NULL)
    {
        res = KV_ERR_BAD_ARG;
        goto exit;
    }

    *size = set->size;

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief check if key is in the kv set or not.
 * 
 * @param set kv set pointer.
 * @param key key string pointer.
 * @return  return KV_FALSE or KV_TRUE if success, otherwise return other value.
 */
int kv_contain(kv_set_t *set, const char *key)
{
    int res;
    int array_index;
    kv_bucket_t *curt_bucket;
    bool key_found;

    if (set == NULL || key == NULL)
    {
        res = KV_ERR_BAD_ARG;
        goto exit;
    }

    key_found = false;
    array_index = set->hash(key) % KV_MAP_ARRAY_SIZE;
    curt_bucket = set->array[array_index];

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
 * @brief put a key-value pair in the kv set.
 * @note  this function will replace the value corresponding to the specified
 *        key with the specified value if the specified key already exists in
 *        this kv set.
 * 
 * @param set   kv set pointer.
 * @param key   key string pointer.
 * @param value value string pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
int kv_put(kv_set_t *set, const char *key, const char *value)
{
    int res;
    int array_index;
    kv_bucket_t **bucket_next;
    kv_bucket_t *curt_bucket;
    kv_bucket_t *new_bucket;
    bool key_found;
    char *inner_key;
    char *inner_value;
    size_t malloc_size;

    if (set == NULL || key == NULL || value == NULL)
    {
        res = KV_ERR_BAD_ARG;
        goto exit;
    }

    key_found = false;
    array_index = set->hash(key) % KV_MAP_ARRAY_SIZE;
    curt_bucket = set->array[array_index];
    bucket_next = set->array + array_index;

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
            res = KV_ERR_BAD_MEM;
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

        set->size++;
    }

    return KV_OK;

err_malloc_value:
    free(inner_key);
err_malloc_key:
    free(new_bucket);
err_malloc_bucket:
    res = KV_ERR_BAD_MEM;
exit:
    return res;
}

/**
 * @brief delete a key-value pair in the kv set.
 * 
 * @param set kv set pointer.
 * @param key key string pointer.
 * @return  return KV_OK if success, or return KV_ERR_KEY_NOT_FOUND if the 
 *          specified key isn't found in kv set, otherwise return other value.
 */
int kv_del(kv_set_t *set, const char *key)
{
    int res;
    int array_index;
    kv_bucket_t **bucket_next;
    kv_bucket_t *curt_bucket;
    kv_bucket_t *next_bucket;
    bool key_found;

    if (set == NULL || key == NULL)
    {
        res = KV_ERR_BAD_ARG;
        goto exit;
    }

    key_found = false;
    array_index = set->hash(key) % KV_MAP_ARRAY_SIZE;
    curt_bucket = set->array[array_index];
    bucket_next = set->array + array_index;

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
        set->size--;
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
 * @brief get the value string corresponding to the specified key in the kv set.
 * @note  the value pointed by 'value' won't update if this function doesn't
 *        return KV_OK.
 * 
 * @param set   kv set pointer.
 * @param key   key string pointer.
 * @param value pointer to a variable for storing value string pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
int kv_get(kv_set_t *set, const char *key, const char **value)
{
    int res;
    int array_index;
    kv_bucket_t *curt_bucket;
    bool key_found;

    if (set == NULL || key == NULL || value == NULL)
    {
        res = KV_ERR_BAD_ARG;
        goto exit;
    }

    key_found = false;
    array_index = set->hash(key) % KV_MAP_ARRAY_SIZE;
    curt_bucket = set->array[array_index];

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
 * @brief clear all key-value pairs in the kv set.
 * 
 * @param set kv set pointer.
 * @return  return KV_OK if success, otherwise return other value.
 */
int kv_clear(kv_set_t *set)
{
    int res;
    kv_bucket_t *curt_bucket;
    kv_bucket_t *next_bucket;

    if (set == NULL)
    {
        res = KV_ERR_BAD_ARG;
        goto exit;
    }

    /* free every chain on the set array */
    for (int i = 0; i < KV_MAP_ARRAY_SIZE; i++)
    {
        if (set->array[i] != NULL)
        {
            next_bucket = set->array[i];
            do
            {
                curt_bucket = next_bucket;
                next_bucket = curt_bucket->next;

                /* free the property of current bucket */
                free(curt_bucket->key);
                free(curt_bucket->value);

                free(curt_bucket);
            } while (next_bucket != NULL);
            set->array[i] = NULL;
        }
    }

    set->size = 0;

    res = KV_OK;
exit:
    return res;
}

/**
 * @brief iterate all the key-value pairs in the kv set.
 * 
 * @param set         kv set pointer.
 * @param foreach_cb  pointer to iteration callback function.
 * @return  return KV_OK if success, otherwise return other value.
 */
int kv_foreach(kv_set_t *set, kv_foreach_cb_t foreach_cb, void *arg)
{
    int res;
    kv_bucket_t *curt_bucket;

    if (set == NULL)
    {
        res = KV_ERR_BAD_ARG;
        goto exit;
    }

    /* free every chain on the set array */
    for (int i = 0; i < KV_MAP_ARRAY_SIZE; i++)
    {
        if (set->array[i] != NULL)
        {
            curt_bucket = set->array[i];
            while (curt_bucket != NULL)
            {
                foreach_cb(arg, curt_bucket->key, curt_bucket->value);
                curt_bucket = curt_bucket->next;
            }
        }
    }

    res = KV_OK;
exit:
    return res;
}
