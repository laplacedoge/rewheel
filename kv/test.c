#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "kv.h"

const char *keys[] = 
{
    "charlieputh",
    "jomaoppa",
    "GreatScottLab",
    "adafruit",
    "Hacksterio",
    "__refla",
    "AlwaysRamCharan",
    "henryheffernan",
    "Blender",
    "KurtHSchneider",
};

const char *values[] = 
{
    "Charlie Puth",
    "joma",
    "GreatScott!",
    "adafruit industries",
    "Hackster.io",
    "Refla",
    "Ram Charan",
    "Henry Heffernan",
    "Blender",
    "Kurt Schneider",
};

const char *value_9_sub = "Kurt Hugo Schneider";

#define KV_PAIR_NUM (sizeof(keys) / sizeof(const char *))

void add_key_value_pairs(kv_map_t *map)
{
    kv_res_t res;

    for (int i = 0; i < KV_PAIR_NUM; i++)
    {
        res = kv_put(map, keys[i], values[i]);
        assert(res == KV_OK);
    }
}

void foreach_cb(void *arg, const char *key, const char *value)
{
    printf("{\"%s\": \"%s\"}\n", key, value);
}

int main(int argc, char *argv[])
{
    kv_res_t res;
    kv_map_t *map;
    int size;
    const char *value;

    res = kv_create(&map, NULL);
    assert(res == KV_OK);

    res = kv_size(map, &size);
    assert(res == KV_OK);
    assert(size == 0);

    add_key_value_pairs(map);

    res = kv_size(map, &size);
    assert(res == KV_OK);
    assert(size == KV_PAIR_NUM);

    res = kv_foreach(map, foreach_cb, NULL);
    assert(res == KV_OK);

    res = kv_contain(map, keys[1]);
    assert(res == KV_TRUE);
    res = kv_contain(map, keys[3]);
    assert(res == KV_TRUE);
    res = kv_contain(map, keys[7]);
    assert(res == KV_TRUE);
    res = kv_contain(map, "df457bc7");
    assert(res == KV_FALSE);

    res = kv_get(map, keys[0], &value);
    assert(res == KV_OK);
    assert(strcmp(values[0], value) == 0);

    res = kv_del(map, keys[0]);
    assert(res == KV_OK);
    res = kv_del(map, keys[1]);
    assert(res == KV_OK);
    res = kv_del(map, keys[2]);
    assert(res == KV_OK);

    res = kv_contain(map, keys[0]);
    assert(res == KV_FALSE);
    res = kv_contain(map, keys[1]);
    assert(res == KV_FALSE);
    res = kv_contain(map, keys[2]);
    assert(res == KV_FALSE);

    res = kv_get(map, keys[0], &value);
    assert(res == KV_ERR_KEY_NOT_FOUND);
    res = kv_get(map, keys[1], &value);
    assert(res == KV_ERR_KEY_NOT_FOUND);
    res = kv_get(map, keys[2], &value);
    assert(res == KV_ERR_KEY_NOT_FOUND);

    res = kv_put(map, keys[9], value_9_sub);
    assert(res == KV_OK);
    res = kv_get(map, keys[9], &value);
    assert(res == KV_OK);
    assert(strcmp(value_9_sub, value) == 0);

    res = kv_size(map, &size);
    assert(res == KV_OK);
    assert(size == KV_PAIR_NUM - 3);

    res = kv_clear(map);
    assert(res == KV_OK);

    res = kv_size(map, &size);
    assert(res == KV_OK);
    assert(size == 0);

    res = kv_destroy(map);
    assert(res == KV_OK);

    return 0;
}
