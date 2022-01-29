/**
 * @file hashtable.h
 * @author Tamir Attias
 * @brief Hash table declarations.
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

/**
 * Free item callback type.
 */
typedef void(*hashtable_free_item_func_t)(void*);

/**
 * Hash table.
 */
typedef struct hashtable hashtable_t;

/**
 * Allocate a new hash table with a fixed number of slots.
 *
 * @param capacity Number of slots (buckets).
 * @param free_item Callback that is called when deallocating an item.
 * @return Pointer to the hash table object or null if out of memory.
 */
hashtable_t *hashtable_alloc(int capacity, hashtable_free_item_func_t free_item);

/**
 * Deallocates a previously allocated hash table including all keys and items.
 *
 * @param ht Pointer to hash table object to free.
 */
void hashtable_free(hashtable_t *ht);

/**
 * Insert an item into the hash table.
 *
 * @param ht Pointer to hash table object.
 * @param key Key of item to insert. A duplicate of the key will be made.
 * @param item Item to insert. Will be inserted as is without duplication.
 * @return Zero on success, non-zero if out of memory.
 */
int hashtable_insert(hashtable_t *ht, const char *key, void *item);

/**
 * Finds an item in the hash table with a given key.
 *
 * @param ht Pointer to hash table object.
 * @param key Key of item to search for.
 * @return Pointer to the item or null if it could not be found.
 */
void *hashtable_find(hashtable_t *ht, const char *key);

#endif