#ifndef LIST_H
#define LIST_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define STORAGE_SIZE 10
#define MAX_STRING_LENGTH 100

typedef struct _Node{
    char value[MAX_STRING_LENGTH];
    struct _Node* next;
    pthread_mutex_t sync;
} Node;

typedef struct _Storage{
    Node *first;
    size_t size;
} Storage;

Storage* init_storage();
void destroy_storage(Storage* storage);
void add_node(Storage* storage, const char* newValue);
void fill_storage(Storage* storage);

#endif // LIST_H