/**
 * @file array_list.h
 * @brief Header file for the array list data structure.
 * @details This file contains the definition of the array list data structure
 *         and function prototypes for creating, destroying, and manipulating the array list.
 *         The array list is a dynamic array that can grow in size as needed.
 *         The array never shrinks, but the size can be reset to 0. 
 */

#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <stddef.h> /* size_t */

#define ARRAY_LIST_INITIAL_CAPACITY 16
#define ARRAY_LIST_GROWTH_FACTOR 2

/* Free function type for freeing elements in the array list */
typedef void (*ArrayListFreeFunction)(void *ptr);

/* Array list structure */
typedef struct array_list {
    void **items;                                   /* Pointer to the array of pointers */
    size_t size;                                    /* Number of elements currently in the array */
    size_t capacity;                                /* Total capacity of the array */
    ArrayListFreeFunction free_func;                /* Function to free elements */
} ArrayList;


/**
 * @brief Creates a new array list.
 * @param free_func Function to free elements in the array list when destroyed.
 * @note If NULL, no free function is used.
 * @return Pointer to the newly created array list, or NULL on failure.
 * @note The caller is responsible for freeing the array list using array_list_destroy().
 */
ArrayList *array_list_create(ArrayListFreeFunction free_func);

/**
 * @brief Destroys an array list and frees its memory.
 *        Destroys the array list and all its elements using the free function if provided.
 * @param list Pointer to the array list to destroy.
 */
void array_list_destroy(ArrayList *list);

/**
 * @brief Clears the array list and frees its elements using the free function if provided.
 * @param list Pointer to the array list to clear.
 * @note Resets the size to 0, capacity remains unchanged.
 */
void array_list_clear(ArrayList *list);

/**
 * @brief Appends an item to the end of the array list.
 * @param list Pointer to the array list.
 * @param item Pointer to the item to append.
 * @note If the list is full, it will be resized to accommodate the new item.
 */
void array_list_append(ArrayList *list, void *item);

/**
 * @brief Gets an item from the array list at the specified index.
 * @param list Pointer to the array list.
 * @param index Index of the item to get.
 * @return Pointer to the item at the specified index, or NULL if the index is out of bounds.
 */
void *array_list_get(ArrayList *list, size_t index);

/**
 * @brief Removes an item from the array list at the specified index.
 * @param list Pointer to the array list.
 * @param index Index of the item to remove.
 * @return Pointer to the removed item, or NULL if the index is out of bounds.
 * @note The item is not freed, the caller is responsible for freeing it if necessary.
 */
void *array_list_remove(ArrayList *list, size_t index);

/**
 * @brief Gets the size of the array list.
 * @param list Pointer to the array list.
 * @return Size of the array list (number of elements).
 * @note Returns -1 if the list is NULL (since 0 is a valid size).
 */
int array_list_size(ArrayList *list);

/**
 * @brief Converts the array list to a string by concatenating all elements into a single string.
 * @param list Pointer to the array list.
 * @return Pointer to the string representation of the array list, or NULL on failure.
 * @note The caller is responsible for freeing the returned string.
 */
char *array_list_to_str(ArrayList *list);

/**
 * @brief Resizes the array list to accommodate more elements.
 * @param list Pointer to the array list.
 * @return 1 on success, 0 on failure.
 * @note This function is called automatically when the list is full.
 */
int array_list_resize(ArrayList *list);


#endif /* ARRAY_LIST_H */