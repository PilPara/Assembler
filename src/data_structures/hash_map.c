/**
 * @file hash_map.c
 * @brief Implementation of a simple hash map.
 * @details This file contains the implementation of a hash map data structure
 *          using separate chaining for collision resolution. The hash map allows
 *          for dynamic resizing and uses the djb2 hash function for hashing keys.
 */

#include "./hash_map.h"
#include "../common/util.h"

#include <string.h>

HashMap *hash_map_create(HashMapFreeFunction free_func) 
{
    HashMap *map = NULL;
    
    map = (HashMap *)MALLOC(sizeof(HashMap));
    if (!map) 
        return NULL;

    map->capacity = HASH_MAP_INIT_CAPACITY;
    map->size = 0;

    map->entries = (Entry **)MALLOC(map->capacity * sizeof(Entry *));
    if (!map->entries) 
    {
        FREE(map);
        return NULL;
    }

    map->free_func = free_func;

    memset(map->entries, 0, map->capacity * sizeof(Entry *));   

    return map;
}

void hash_map_destroy(HashMap *map) 
{
    
    size_t i;
    Entry *entry = NULL, *next = NULL;

    if (!map) 
        return;


    for (i = 0; i < map->capacity; i++) 
    {
        /* Free the linked list at each index */
        entry = map->entries[i];
        while (entry) 
        {
            /* Free the entry and its key */
            next = entry;
            entry = entry->next;

            /* Free the value if a free function is provided */
            if (map->free_func)
                map->free_func(next->value);

            FREE(next->key);
            FREE(next);
        }
    }

    /* Free the entries array and the map itself */
    FREE(map->entries);
    FREE(map);
}


void hash_map_put(HashMap *map, const char *key, void *value) 
{    
    size_t index = 0;
    Entry *entry = NULL;

    if (!map || !key) 
        return;

    /* Check if needs resizing */
    if (map->size >= map->capacity * HASH_MAP_LOAD_FACTOR) 
        if (!hash_map_resize(map)) 
            return;

    /* Check if the key already exists */
    index = hash(key) % map->capacity;
    entry = map->entries[index];
    while (entry) 
    {
        if (strcmp(entry->key, key) == 0) 
        {
            /* Key already exists - update the value */
            if (map->free_func) 
                map->free_func(entry->value);

            entry->value = value;

            return;
        }

        entry = entry->next;
    }

    /* Key does not exist - create a new entry */
    entry = (Entry *)MALLOC(sizeof(Entry));
    entry->key = STRDUP(key);
    entry->value = value;
    
    /* Add at the head of the linked list at the hash index */
    entry->next = map->entries[index];
    map->entries[index] = entry;
    map->size++;
}

void *hash_map_get(HashMap *map, const char *key)
{
    size_t index = 0;
    Entry *entry = NULL;

    if (!map || !key)
        return NULL;

    /* Get the index using the hash function */
    index = hash((char *)key) % map->capacity;
    entry = map->entries[index];

    /* Traverse the linked list at the index */
    while (entry)
    {
        if (strcmp(entry->key, key) == 0)
            return entry->value;
        entry = entry->next;
    }

    /* Key not found */

    return NULL;
}

size_t hash_map_size(HashMap *map) 
{
    return map ? map->size : 0;
}

int hash_map_contains(HashMap *map, const char *key) 
{
    return hash_map_get(map, key) != NULL;
}

size_t hash(const char *key) 
{
    size_t hash = HASH_CONSTANT;
    int c;

    while ((c = *key++)) 
        hash = ((hash << HASH_SHIFT) + hash) + c; 

    return hash;
}

int hash_map_resize(HashMap *map) 
{
    size_t new_capacity = 0, i;
    Entry **new_entries = NULL;

    if (!map)
        return 0;

    /* Allocate new entries array */
    new_capacity = map->capacity * HASH_MAP_GROWTH_FACTOR;
    new_entries = (Entry **)MALLOC(new_capacity * sizeof(Entry *));

    if (!new_entries)
        return 0;

    memset(new_entries, 0, new_capacity * sizeof(Entry *));

    /* Rehash all entries into the new array */
    for (i = 0; i < map->capacity; i++) 
    {
        Entry *entry = map->entries[i];
        
        while (entry) 
        {
            size_t index = hash(entry->key) % new_capacity;
            Entry *next = entry->next;

            entry->next = new_entries[index];
            new_entries[index] = entry;

            entry = next;
        }
    }

    FREE(map->entries);

    /* Update the map with the new entries */
    map->entries = new_entries;
    map->capacity = new_capacity;

    return 1;
}