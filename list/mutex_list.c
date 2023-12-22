#include "mutex_list.h"

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
        pthread_mutex_destroy(&current->lock);
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

    if (pthread_mutex_init(&new_node->lock, 0) != 0) {
        printf("add_node: pthread_mutex_init failed\n");
        free(new_node);
        return;
    }

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

int lock_node(Node* node) {
    if (pthread_mutex_lock(&node->lock)) {
        printf("lock_node: pthread_mutex_lock failed\n");
        return 1;
    }
    return 0;
}

int unlock_node(Node* node) {
    if (pthread_mutex_unlock(&node->lock)) {
        printf("unlock_node: pthread_mutex_unlock failed\n");
        return 1;
    }
    return 0;
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

    while (1) {
        int asc_order_pairs = 0;
        if (lock_node(storage->first)) 
            return NULL;
        Node* tmp = storage->first;
        while (1) {
            usleep(10);
            Node* next = tmp->next;
            if (!next) {
                if (unlock_node(tmp)) 
                    return NULL;
                break;
            }

            if (lock_node(next)) return NULL;

            if (strlen(tmp->value) < strlen(next->value)) asc_order_pairs++;

            if (unlock_node(tmp)) return NULL;

            tmp = next;
        }
        atomic_fetch_add(&storage->increasing_iter_counter, asc_order_pairs);
    }
    return NULL;
}

void *increment_desc_strings(void *arg) {
    printf("increment_desc_string started\n");
    Storage *storage = (Storage *) arg;
    
    while (1) {
        int desc_order_pairs = 0;
        if (lock_node(storage->first)) return NULL;
        Node* tmp = storage->first;
        while (1) {
            usleep(10);
            Node* next = tmp->next;
            if (!next) {
                if (unlock_node(tmp)) return NULL;
                break;
            }

            if (lock_node(next)) return NULL;

            if (strlen(tmp->value) > strlen(next->value)) desc_order_pairs++;

            if (unlock_node(tmp)) return NULL;

            tmp = next;
        }
        atomic_fetch_add(&storage->decreasing_iter_counter, desc_order_pairs);
    }

    return NULL;
}

void *increment_equal_strings(void *arg) {
    printf("increment_equal_string started\n");
    Storage *storage = (Storage *) arg;

    while (1) {
        int same_order_pairs = 0;
        if (lock_node(storage->first)) return NULL;
        Node* tmp = storage->first;

        while (1) {
            usleep(10);
            Node* next = tmp->next;
            if (!next) {
                if (unlock_node(tmp)) return NULL;
                break;
            }

            if (lock_node(next)) return NULL;

            if (strlen(tmp->value) == strlen(next->value)) same_order_pairs++;

            if (unlock_node(tmp)) return NULL;

            tmp = next;
        }
        atomic_fetch_add(&storage->equal_iter_counter, same_order_pairs);
    }

    return NULL;
}

void swap(Node* node_1, Node* node_2) {
    Node* tmp = node_2->next;
    node_1->next = node_2->next;
    node_2->next = tmp->next;
    tmp->next = node_2;
}

void *swap_nodes(void *arg) {
    printf("swapper started\n");
    Storage *storage = (Storage *) arg;

    Node* s1, *s2;
    while (1) {
        int swap_pairs = 0;
        
        if (lock_node(storage->first)) return NULL;
        else s1 = storage->first;

        if (lock_node(storage->first->next)) return NULL;
        else s2 = storage->first->next;

        while (1) {
            Node* s3 = s2->next;
            if (lock_node(s3)) return NULL;

            if (s3->next == NULL) {
                if (unlock_node(s1)) return NULL; 
                if (unlock_node(s2)) return NULL; 
                if (unlock_node(s3)) return NULL;
                break;
            }
            else {
                // now list is s1 -> s2 -> s3
                if (rand() % 9 == 0) {
                    swap(s1, s2);
                    // now list is s1 -> s3 -> s2
                    if (unlock_node(s1)) return NULL;
                    s1 = s3;
                    swap_pairs += 1;
                }
                else {
                    if (unlock_node(s1)) return NULL;
                    s1 = s2;
                    s2 = s3;
                }
            }
        }
        atomic_fetch_add(&storage->swap_iter_counter, swap_pairs);
    }

    return NULL;
}