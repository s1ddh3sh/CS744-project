#ifndef KV_STORE_H
#define KV_STORE_H

#include <stdbool.h>
#include <pthread.h>

typedef struct
{
    char key[256];
    char value[256];
} kv_pair;

typedef struct
{
    kv_pair pairs[1000];
    int count;
    int start;
    pthread_mutex_t mutex;
} kv_store;

void init(kv_store *store);
bool kv_create(kv_store *store, char *key, char *value, int len);
char *kv_get(kv_store *store, char *key);
bool kv_update(kv_store *store, char *key, char *value, int len);
bool kv_delete(kv_store *store, char *key);

#endif
