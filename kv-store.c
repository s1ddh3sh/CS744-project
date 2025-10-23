#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "kv-store.h"
#include "db.h"

#define MAX_PAIRS 1000

void init(kv_store *store)
{
    store->count = 0;
    store->start = 0;
    pthread_mutex_init(&store->mutex, NULL);
}

static inline int idx(const kv_store *store, int i)
{
    return (store->start + i) % MAX_PAIRS;
}

bool kv_create(kv_store *store, char *key, char *value, int len)
{
    pthread_mutex_lock(&store->mutex);
    for (int i = 0; i < store->count; i++)
    {
        if (strcmp(store->pairs[idx(store, i)].key, key) == 0)
            return 0;
    }
    // Cache full, do FCFS evict
    if (store->count == MAX_PAIRS)
    {
        kv_pair victim = store->pairs[store->start];
        db_insert(victim.key, victim.value);

        store->start = (store->start + 1) % MAX_PAIRS;
        store->count--;
    }
    int index = (store->start + store->count) % MAX_PAIRS;
    kv_pair *newp = &store->pairs[index];

    strncpy(newp->key, key, sizeof(newp->key) - 1);
    newp->key[sizeof(newp->key) - 1] = '\0';

    int len2 = (len < (int)sizeof(newp->value) - 1) ? len : (int)sizeof(newp->value) - 1;
    strncpy(newp->value, value, len2);
    newp->value[len2] = '\0';

    store->count++;
    pthread_mutex_unlock(&store->mutex);
    return 1;
}

char *kv_get(kv_store *store, char *key)
{
    pthread_mutex_lock(&store->mutex);
    for (int i = 0; i < store->count; i++)
    {
        kv_pair *p = &store->pairs[idx(store, i)];
        if (strcmp(p->key, key) == 0)
        {
            return p->value;
        }
    }
    pthread_mutex_unlock(&store->mutex);

    // Cache miss - check in DB
    char *val = db_get(key);
    if (val != NULL)
    {
        pthread_mutex_lock(&store->mutex);
        kv_create(store, key, val, strlen(val));
        pthread_mutex_unlock(&store->mutex);

        return val;
    }
    return NULL;
}

bool kv_update(kv_store *store, char *key, char *value, int len)
{
    pthread_mutex_lock(&store->mutex);

    for (int i = 0; i < store->count; i++)
    {
        kv_pair *p = &store->pairs[idx(store, i)];
        if (strcmp(p->key, key) == 0)
        {
            int len2 = (len < (int)sizeof(p->value) - 1) ? len : (int)sizeof(p->value) - 1;
            strncpy(p->value, value, len2);
            p->value[len2] = '\0';
            return 1;
        }
    }
    pthread_mutex_unlock(&store->mutex);

    return 0;
}

bool kv_delete(kv_store *store, char *key)
{
    pthread_mutex_lock(&store->mutex);

    for (int i = 0; i < store->count; i++)
    {
        int curr_index = idx(store, i);
        if (strcmp(store->pairs[curr_index].key, key) == 0)
        {
            for (int j = i + 1; j < store->count; j++)
            {
                int from = idx(store, j);
                int to = idx(store, j - 1);
                store->pairs[to] = store->pairs[from];
            }
            store->count--;
            return 1;
        }
    }
    pthread_mutex_unlock(&store->mutex);

    return 0;
}