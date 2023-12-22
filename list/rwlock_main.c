#include <stdio.h>
#include "rwlock_list.h"

int main() {
    Storage* list_storage = init_storage();
    if (list_storage == NULL) {
        return 1;
    }
    fill_storage(list_storage);
    // print_storage(list_storage);    

    pthread_t   asc_incrementer, 
                desc_incrementer, 
                equal_incrementer,
                swapper1, 
                swapper2, 
                swapper3;

    pthread_create(&asc_incrementer, NULL, increment_asc_strings, list_storage);
    pthread_create(&desc_incrementer, NULL, increment_desc_strings, list_storage);
    pthread_create(&equal_incrementer, NULL, increment_equal_strings, list_storage);

    pthread_create(&swapper1, NULL, swap_nodes, list_storage);
    pthread_create(&swapper2, NULL, swap_nodes, list_storage);
    pthread_create(&swapper3, NULL, swap_nodes, list_storage);

    while (1) {
        sleep(1);
        printf("asc string %llu\n", atomic_load(&list_storage->increasing_iter_counter));
        printf("desc strings %llu\n", atomic_load(&list_storage->increasing_iter_counter));
        printf("equal strings: %llu\n", atomic_load(&list_storage->increasing_iter_counter));
        printf("swaps: %llu\n", atomic_load(&list_storage->swap_iter_counter));
        printf("\n");
    }

    pthread_join(asc_incrementer, NULL);
    pthread_join(desc_incrementer, NULL);
    pthread_join(equal_incrementer, NULL);

    pthread_join(swapper1, NULL);
    pthread_join(swapper2, NULL);
    pthread_join(swapper3, NULL);

    destroy_storage(list_storage);

    return 0;
}
