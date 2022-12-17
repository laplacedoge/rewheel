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

void add_key_value_pairs(kv_set_t *set)
{
    int res;

    for (int i = 0; i < KV_PAIR_NUM; i++)
    {
        res = kv_put(set, keys[i], values[i]);
        assert(res == KV_OK);
    }
}

void foreach_cb(void *arg, const char *key, const char *value)
{
    printf("{\"%s\": \"%s\"}\n", key, value);
}

int main(int argc, char *argv[])
{
    int res;
    kv_set_t *set;
    size_t size;
    const char *value;

    res = kv_create(&set, NULL);
    assert(res == KV_OK);

    res = kv_size(set, &size);
    assert(res == KV_OK);
    assert(size == 0);

    add_key_value_pairs(set);

    res = kv_size(set, &size);
    assert(res == KV_OK);
    assert(size == KV_PAIR_NUM);

    res = kv_foreach(set, foreach_cb, NULL);
    assert(res == KV_OK);

    res = kv_contain(set, keys[1]);
    assert(res == KV_TRUE);
    res = kv_contain(set, keys[3]);
    assert(res == KV_TRUE);
    res = kv_contain(set, keys[7]);
    assert(res == KV_TRUE);
    res = kv_contain(set, "df457bc7");
    assert(res == KV_FALSE);

    res = kv_get(set, keys[0], &value);
    assert(res == KV_OK);
    assert(strcmp(values[0], value) == 0);

    res = kv_del(set, keys[0]);
    assert(res == KV_OK);
    res = kv_del(set, keys[1]);
    assert(res == KV_OK);
    res = kv_del(set, keys[2]);
    assert(res == KV_OK);

    res = kv_contain(set, keys[0]);
    assert(res == KV_FALSE);
    res = kv_contain(set, keys[1]);
    assert(res == KV_FALSE);
    res = kv_contain(set, keys[2]);
    assert(res == KV_FALSE);

    res = kv_get(set, keys[0], &value);
    assert(res == KV_ERR_KEY_NOT_FOUND);
    res = kv_get(set, keys[1], &value);
    assert(res == KV_ERR_KEY_NOT_FOUND);
    res = kv_get(set, keys[2], &value);
    assert(res == KV_ERR_KEY_NOT_FOUND);

    res = kv_put(set, keys[9], value_9_sub);
    assert(res == KV_OK);
    res = kv_get(set, keys[9], &value);
    assert(res == KV_OK);
    assert(strcmp(value_9_sub, value) == 0);

    res = kv_size(set, &size);
    assert(res == KV_OK);
    assert(size == KV_PAIR_NUM - 3);

    res = kv_clear(set);
    assert(res == KV_OK);

    res = kv_size(set, &size);
    assert(res == KV_OK);
    assert(size == 0);

    res = kv_destroy(set);
    assert(res == KV_OK);

    return 0;
}
