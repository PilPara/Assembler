/**
 * @file array_list.c
 * @brief Implementation of the ArrayList data structure.
 * @details This file contains the implementation of a dynamic array list
 *          that can grow in size as needed. The array list allows for
 *          adding, removing, and accessing elements. It also provides
 *          functions for resizing the array and converting it to a string.
 */

#include "./array_list.h"
#include "../common/util.h"

#include <string.h>

ArrayList *array_list_create(ArrayListFreeFunction free_func) 
{
    size_t i;

    ArrayList *list = (ArrayList *)MALLOC(sizeof(ArrayList));
    if (!list)
        return NULL;

    list->items = (void **)MALLOC(ARRAY_LIST_INITIAL_CAPACITY * sizeof(void *));

    if (!list->items) 
    {
        FREE(list);
        return NULL;
    }

    list->size = 0;
    list->capacity = ARRAY_LIST_INITIAL_CAPACITY;
    list->free_func = free_func;
    
    /* Initialize all items to NULL - basically a memset */
    for (i = 0; i < list->capacity; i++) 
        list->items[i] = NULL;

    return list;
}

void array_list_destroy(ArrayList *list) 
{
    size_t i;

    if (!list)
        return;

    if (list->free_func)
        for (i = 0; i < list->size; i++)
            if (list->items[i])
                list->free_func(list->items[i]);

    FREE(list->items);
    FREE(list);
}

void array_list_clear(ArrayList *list) 
{
    size_t i;

    if (!list)
        return;

    if (list->free_func)
        for (i = 0; i < list->size; i++)
            if (list->items[i])
                list->free_func(list->items[i]);

    for (i = 0; i < list->size; i++)
        list->items[i] = NULL;

    list->size = 0;
}

void array_list_add(ArrayList *list, size_t index, void *item) 
{
    size_t i;

    if (!list || index > list->size)
        return;

    /* Check if need to resize */
    if (list->size == list->capacity) 
        if (!array_list_resize(list))
            return;

    /* Shift items to the right */
    for (i = list->size; i > index; i--)
        list->items[i] = list->items[i - 1];

    /* Insert the new item */
    list->items[index] = item;
    list->size++;
}

void array_list_append(ArrayList *list, void *item)
{
    if (!list)
        return;

    if (list->size == list->capacity) 
        if (!array_list_resize(list))
            return;

    list->items[list->size++] = item;
}

void *array_list_get(ArrayList *list, size_t index) 
{
    if (!list || index >= list->size)
        return NULL;

    return list->items[index];
}

void *array_list_remove(ArrayList *list, size_t index) 
{
    void *item = NULL;
    size_t i;

    item = array_list_get(list, index);

    if (!item)
        return NULL;

    for (i = index; i < list->size - 1; i++)
        list->items[i] = list->items[i + 1];

    list->items[--list->size] = NULL;

    return item;
}

int array_list_size(ArrayList *list) 
{
    return list ? (int)list->size : -1;
}

int array_list_resize(ArrayList *list) 
{
    void **new_items = NULL;
    size_t i;

    if (!list)
        return 0;

    new_items = (void **)REALLOC(list->items, list->capacity * ARRAY_LIST_GROWTH_FACTOR * sizeof(void *));
    if (!new_items)
        return 0;

    list->items = new_items;
    list->capacity *= ARRAY_LIST_GROWTH_FACTOR;

    for (i = list->size; i < list->capacity; i++)
        list->items[i] = NULL;

    return 1;
}

char *array_list_to_str(ArrayList *list) 
{
    size_t i;
    size_t total_length = 0;
    char *result = NULL;

    if (!list)
        return NULL;

    for (i = 0; i < list->size; i++) 
        total_length += strlen((char *)list->items[i]) + 1; 

    result = (char *)MALLOC(total_length + 1);
    if (!result)
        return NULL;

    result[0] = '\0';

    for (i = 0; i < list->size; i++) 
    {
        strcat(result, (char *)list->items[i]);
        if (i < list->size - 1)
            strcat(result, "\n");
    }

    return result;
}
