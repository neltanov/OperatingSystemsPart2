#include "list.h"

Storage* init_storage() {
    Storage* storage = malloc(sizeof(Storage));
    if (!storage) {
        return NULL;
    }
    storage->size = 0;
    storage->first = NULL;

    return storage;
}

void destroy_storage(Storage* storage) {
    Node* current = storage->first;
    Node* temp;

    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }

    free(storage);
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
