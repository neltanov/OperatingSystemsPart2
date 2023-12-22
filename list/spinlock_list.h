#ifndef SPINLOCK_LIST_H
#define SPINLOCK_LIST_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include "utils.h"

typedef struct _Node{
    char value[MAX_STRING_LENGTH];
    struct _Node* next;
    pthread_spinlock_t lock;
} Node;

typedef struct _Storage{
    Node *first;
    size_t size;

    atomic_ullong asc_order_count, 
                desc_order_count, 
                same_order_count, 
                swaps_count;
} Storage;

Storage* init_storage();
void destroy_storage(Storage* storage);
void print_storage(Storage* storage);
void add_node(Storage* storage, const char* newValue);
void fill_storage(Storage* storage);
int lock_node(Node* node);
int unlock_node(Node* node);
void *increment_asc_strings(void *arg);
void *increment_desc_strings(void *arg);
void *increment_equal_strings(void *arg);
void swap(Node* node_1, Node* node_2);
void *swap_nodes(void *arg);


#endif // LIST_H