/**
 * @file hash_map.h
 * @brief Header file for a simple hash map implementation.
 * @details This file contains the definition of the hash map structure and function prototypes for creating,
 *          destroying, and manipulating the hash map.
 *          The hash map uses separate chaining for collision resolution and allows for dynamic resizing.
 *          The hash function is the djb2 algorithm.
 */

#ifndef HASH_MAP_H  
#define HASH_MAP_H

#include <stddef.h>

#define HASH_MAP_INIT_CAPACITY 16
#define HASH_MAP_LOAD_FACTOR 0.75
#define HASH_MAP_GROWTH_FACTOR 2
#define HASH_CONSTANT 5381
#define HASH_SHIFT 5

/* Free function type for freeing elements in the hash map */
typedef void (*HashMapFreeFunction)(void *ptr);

/* Hash map entry structure */
typedef struct Entry {
    char *key;                      /* Key for the entry */
    void *value;                    /* Value associated with the key */
    struct Entry *next;             /* Pointer to the next entry in the linked list */
} Entry;

/* Hash map structure */
typedef struct HashMap {
    Entry **entries;                /* Array of pointers to linked lists of entries */
    size_t capacity;                /* Total capacity of the hash map */
    size_t size;                    /* Number of entries in the hash map */
    HashMapFreeFunction free_func;  /* Function to free elements */
} HashMap;


/**
 * @brief Creates a new hash map.
 * @param free_func Function to free elements in the hash map when destroyed.
 * @note If NULL, no free function is used.
 * @return Pointer to the newly created hash map, or NULL on failure.
 * @note The caller is responsible for freeing the hash map using hash_map_destroy().
 */
HashMap *hash_map_create(HashMapFreeFunction free_func);

/**
 * @brief Destroys a hash map and frees its memory.
 * @param map Pointer to the hash map to destroy.
 * @note Frees all entries and their keys and values using the free function if provided. 
 */

void hash_map_destroy(HashMap *map);

/**
 * @brief Puts a key-value pair into the hash map.
 * @param map Pointer to the hash map.
 * @param key Key for the entry.
 * @param value Value associated with the key.
 * @note If the key already exists, the value is updated.
 * @note If the map is full, it will be resized to accommodate the new entry.
 */
void hash_map_put(HashMap *map, const char *key, void *value);

/**
 * @brief Gets a value from the hash map by key.
 * @param map Pointer to the hash map.
 * @param key Key for the entry.
 * @return Pointer to the value associated with the key, or NULL if not found.
 */
void *hash_map_get(HashMap *map, const char *key);

/**
 * @brief Gets the number of entries in the hash map.
 * @param map Pointer to the hash map.
 * @return Number of entries in the hash map.
 * @note Returns -1 if the map is NULL (0 is a valid size).
 */
size_t hash_map_size(HashMap *map);

/**
 * @brief Resizes the hash map to accommodate more entries.
 * @param map Pointer to the hash map.
 * @return 1 on success, 0 on failure.
 * @note The hash map is resized to the growth factor times the current capacity.
 */
int hash_map_resize(HashMap *map);

/**
 * @brief Checks if the hash map contains a key.
 * @param map Pointer to the hash map.
 * @param key Key to check for.
 * @return 1 if the key exists, 0 otherwise.
 */
int hash_map_contains(HashMap *map, const char *key);   

/**
 * @brief Hash function for strings.
 * @param key Pointer to the string key.
 * @return Hash value for the key.
 */size_t hash(const char *key);
#endif /* HASH_MAP_H */