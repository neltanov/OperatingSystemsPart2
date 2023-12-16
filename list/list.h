#ifndef LIST_H
#define LIST_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

#define STORAGE_SIZE 10
#define MAX_STRING_LENGTH 100

typedef struct _Node{
    char value[MAX_STRING_LENGTH];
    struct _Node* next;
    pthread_spinlock_t lock;
} Node;

typedef struct _Storage{
    Node *first;
    size_t size;

    atomic_ullong increasing_iter_counter, 
                decreasing_iter_counter, 
                equal_iter_counter, 
                swap_iter_counter;
} Storage;

Storage* init_storage();
void destroy_storage(Storage* storage);
void print_storage(Storage* storage);
void add_node(Storage* storage, const char* newValue);
void fill_storage(Storage* storage);
void *increment_asc_strings(void *arg);
void *increment_desc_strings(void *arg);
void *increment_equal_strings(void *arg);
void *swap_nodes(void *arg);


#endif // LIST_H