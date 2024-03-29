/**
 * @file hashtable.c
 * @author Tamir Attias
 * @brief Hash table implementation.
 */

#include "hashtable.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Node in a bucket's linked list.
 */
typedef struct bucketnode {
    /** Key. */
    char *key;
    /** Item pointer. */
    void *item;
    /** Next item in linked list. */
    struct bucketnode *next;
} bucketnode_t;

struct hashtable {
    /** Free item callback. */
    hashtable_free_item_func_t free_item;
    /** Number of slots. */
    int capacity;
    /** Slots. Each is a linked list of items. */
    bucketnode_t **buckets;
};

/**
 * Clone a string.
 *
 * @param str String to clone.
 * @return Cloned string object or null if out of memory.
 */
static char *dupstr(const char *str)
{
    int len = strlen(str); /* Find length of string. */
    char *dup = (char*)malloc(len + 1); /* Allocate buffer with room for null pointer. */
    
    /* Check if out of memory. */
    if (!dup)
        return 0;

    /* Copy entire string including null terminator. */
    memcpy(dup, str, len + 1);

    return dup;
}

/**
 * Hashes a key.
 *
 * @param key Key to hash.
 * @return Hash value.
 */
static unsigned hash(const char *key)
{
    /* Don't know how good this algorithm is but probably good enough. */

    char c;
    unsigned hash = 7;

    while ((c = *key++) != '\0')
        hash = hash * 31 + c;

    return hash;
}

hashtable_t *hashtable_alloc(int capacity, hashtable_free_item_func_t free_item)
{
    /* Allocate hash table object. */
    hashtable_t *ht = (hashtable_t*)malloc(sizeof(hashtable_t));

    /* Check if out of memory. */
    if (!ht)
        return 0;

    ht->free_item = free_item;
    ht->capacity = capacity;
    
    /* Allocate array of buckets. */
    ht->buckets = calloc(capacity, sizeof(ht->buckets[0]));

    /* Check if out of memory when trying to allocate buckets. */
    if (!ht->buckets) {
        free(ht); /* Free hash table object. */
        return 0;
    }

    return ht;
}

void hashtable_free(hashtable_t *ht)
{
    int i; /* Loop counter. */
    bucketnode_t *cur, *next; /* Current node, next node. */

    /* Go through every slot. */
    for (i = 0; i < ht->capacity; ++i) {
        /* Traverse linked list of nodes. */
        for (cur = ht->buckets[i]; cur; cur = next) {
            /* Set next node to current's next before freeing current. */
            next = cur->next;

            /* Free current node's key. */
            free(cur->key);

            /* Free item in current node. */
            ht->free_item(cur->item);

            /* Free current node. */
            free(cur);
        }
    }

    /* Free bucket list. */
    free(ht->buckets);

    /* Free the hash table object. */
    free(ht);
}

int hashtable_insert(hashtable_t *ht, const char *key, void *item)
{
    unsigned index; /* Bucket index. */
    bucketnode_t *node; /* New node. */

    /* Calculate hash and then bucket index from it. */
    index = hash(key) % (unsigned)ht->capacity;

    /* Try to allocate a new node. */
    if ((node = (bucketnode_t*)malloc(sizeof(bucketnode_t))) == 0) {
        /* Out of memory. */
        return 1;
    }

    /* Duplicate key and store in new node. */
    if ((node->key = dupstr(key)) == 0) {
        /* Out of memory. */
        free(node); /* Free the new node. */
        return 1;
    }

    /* Store item in new node. */
    node->item = item;

    /* Make current head the next pointer of the new node. */
    node->next = ht->buckets[index];

    /* Make the new node the head of the list. */
    ht->buckets[index] = node;
    
    return 0;
}

void *hashtable_find(hashtable_t *ht, const char *key)
{
    bucketnode_t *bucket;

    /* Calculate hash and index then get bucket. */
    bucket = ht->buckets[hash(key) % (unsigned)ht->capacity];

    /* Find node in bucket with matching key. */
    for (; bucket; bucket = bucket->next) {
        if (strcmp(bucket->key, key) == 0) {
            /* Found matching node, return value. */
            return bucket->item;
        }
    }

    return 0;
}