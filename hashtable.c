/**
 * @file hashtable.c
 * @author Tamir Attias
 * @brief Hash table implementation.
 */

#include "hashtable.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct bucketnode {
    char *key;
    void *item;
    struct bucketnode *next;
} bucketnode_t;

struct hashtable {
    hashtable_free_item_func_t free_item;
    int capacity;
    bucketnode_t **buckets;
};

static char *dupstr(const char *str)
{
    int len;
    char *dup;
    len = strlen(str);
    dup = (char*)malloc(len + 1);
    memcpy(dup, str, len + 1);
    return dup;
}

static int hash(const char *key)
{
    char c;
    int hash = 7;
    while ((c = *key++) != '\0')
        hash = hash * 31 + c;
    return hash;
}

hashtable_t *hashtable_alloc(int capacity, hashtable_free_item_func_t free_item)
{
    hashtable_t *ht = (hashtable_t*)malloc(sizeof(hashtable_t));
    ht->free_item = free_item;
    ht->capacity = capacity;
    ht->buckets = calloc(capacity, sizeof(ht->buckets[0]));
    return ht;
}

void hashtable_free(hashtable_t *ht)
{
    int i;
    bucketnode_t *curbucket, *nextbucket;

    for (i = 0; i < ht->capacity; ++i) {
        for (curbucket = ht ->buckets[i]; curbucket; curbucket = nextbucket) {
            nextbucket = curbucket->next;
            free(curbucket->key);
            ht->free_item(curbucket->item);
            free(curbucket);
        }
    }

    free(ht);
}

void hashtable_insert(hashtable_t *ht, const char *key, void *item)
{
    int index;
    bucketnode_t *node;

    /* Calculate hash and then bucket index from it. */
    index = hash(key) % ht->capacity;

    /* Find tail of bucket. */
    for (node = ht->buckets[index]; node && node->next; node = node->next)
        ;

    if (node) {
        /* Bucket is not empty, allocate new node at tail. */
        node->next = (bucketnode_t*)malloc(sizeof(bucketnode_t));
        node = node->next;
    } else {
        /* Bucket is empty, create head node. */
        ht->buckets[index] = (bucketnode_t*)malloc(sizeof(bucketnode_t));
        node = ht->buckets[index];
    }

    /* Duplicate key and store in node. */
    node->key = dupstr(key);

    /* Tail of list. */
    node->next = 0;

    /* Store item in node */
    node->item = item;
}

void *hashtable_find(hashtable_t *ht, const char *key)
{
    bucketnode_t *bucket;

    /* Calculate hash and index then get bucket. */
    bucket = ht->buckets[(unsigned)hash(key) % (unsigned)ht->capacity];

    /* Find node in bucket with matching key. */
    for (; bucket; bucket = bucket->next) {
        if (strcmp(bucket->key, key) == 0) {
            /* Found matching node, return value. */
            return bucket->item;
        }
    }

    return 0;
}