/**
 * @file hashtable.h
 * @author Tamir Attias
 * @brief Hash table declarations.
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef void(*hashtable_free_item_func_t)(void*);

typedef struct hashtable hashtable_t;

hashtable_t *hashtable_alloc(int capacity, hashtable_free_item_func_t free_item);

void hashtable_free(hashtable_t *ht);

void hashtable_insert(hashtable_t *ht, const char *key, void *item);

void *hashtable_find(hashtable_t *ht, const char *key);

#endif