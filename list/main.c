#include <stdio.h>
#include "list.h"

int main() {
    Storage* storage = init_storage();
    if (storage == NULL) {
        return 1;
    }
    fill_storage(storage);

    Node* cur = storage->first;
    for (size_t i = 0; i < STORAGE_SIZE; i++)
    {
        printf("%s\n", cur->value);
        cur = cur->next;
    }

    destroy_storage(storage);

    return 0;
}
