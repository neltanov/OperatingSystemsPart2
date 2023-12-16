#include "list.h"

Storage* init_storage() {
    Storage* storage = malloc(sizeof(Storage));
    if (!storage) {
        return NULL;
    }
    storage->size = 0;
    storage->first = NULL;

    atomic_store(&storage->increasing_iter_counter, 0);
    atomic_store(&storage->decreasing_iter_counter, 0);
    atomic_store(&storage->equal_iter_counter, 0);
    atomic_store(&storage->swap_iter_counter, 0);

    return storage;
}

void destroy_storage(Storage* storage) {
    Node* current = storage->first;
    Node* temp;

    while (current != NULL) {
        temp = current;
        pthread_spin_destroy(&current->lock);
        current = current->next;
        free(temp);
    }

    free(storage);
}

void print_storage(Storage* storage) {
    Node* cur = storage->first;
    for (size_t i = 0; i < STORAGE_SIZE; i++)
    {
        printf("%ld) %s\n", i, cur->value);
        cur = cur->next;
    }
}

void add_node(Storage* storage, const char* new_value) {
    if (!storage || storage->size >= STORAGE_SIZE) {
        printf("Storage is full\n");
        return;
    }
    
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        return;
    }

    pthread_spin_init(&new_node->lock, PTHREAD_PROCESS_PRIVATE);

    strncpy(new_node->value, new_value, MAX_STRING_LENGTH - 1);
    new_node->value[MAX_STRING_LENGTH - 1] = '\0';
    new_node->next = NULL;

    if (storage->first == NULL) {
        storage->first = new_node;
    } else {
        Node* current = storage->first;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    storage->size++;
}

void fill_storage(Storage* storage) {
    printf("Start random filling storage...\n");
    srand(time(NULL));
    for (size_t i = 0; i < STORAGE_SIZE; i++)
    {   
        int rand_length = rand() % (MAX_STRING_LENGTH - 1) + 1;
        char new_random_string[rand_length + 1];
        new_random_string[rand_length] = '\0';
        for (size_t i = 0; i < rand_length; i++)
        {   
            char random_char;
            int random_choice = rand() % 3;
            
            switch (random_choice)
            {
            case 0:
                random_char = 'A' + rand() % 26;
                break;
            case 1:
                random_char = 'a' + rand() % 26;
                break;
            case 2:
                random_char = '0' + rand() % 10;
                break;
            default:
                break;
            }

            new_random_string[i] = random_char;
        }

        add_node(storage, new_random_string);
    }
    printf("Random storage filling is done!\n");
}

void *increment_asc_strings(void *arg) {
    printf("increment_asc_string started\n");
    Storage *storage = (Storage *) arg;
    atomic_fetch_add(&storage->increasing_iter_counter, 1);
    printf("asc: %llu\n", atomic_load(&storage->increasing_iter_counter));

    
    return NULL;
}

void *increment_desc_strings(void *arg) {
    printf("increment_desc_string started\n");
    Storage *storage = (Storage *) arg;
    atomic_fetch_add(&storage->decreasing_iter_counter, 1);
    printf("desc: %llu\n", atomic_load(&storage->decreasing_iter_counter));
    
    return NULL;
}

void *increment_equal_strings(void *arg) {
    printf("increment_equal_string started\n");
    Storage *storage = (Storage *) arg;
    atomic_fetch_add(&storage->equal_iter_counter, 1);
    printf("equal: %llu\n", atomic_load(&storage->equal_iter_counter));

    return NULL;
}

void *swap_nodes(void *arg) {
    printf("swapper started\n");
    Storage *storage = (Storage *) arg;
    atomic_fetch_add(&storage->swap_iter_counter, 1);
    printf("swapper: %llu\n", atomic_load(&storage->swap_iter_counter));

    return NULL;
}