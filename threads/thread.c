#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define THREAD_COUNT 5

int global_var = 325159;

void *mythread(void *arg) {
	int local_var = 2121;

    static int local_static = 214124;
    const int local_const = 2324;
    printf("\nmythread [%d %d %d %lu]: Hello from mythread!\nlocal_var_addr: %p\nlocal_static: %p\nconst_local: %p\nglobal: %p\n", getpid(), getppid(), gettid(), pthread_self(), &local_var, &local_static, &local_const, &global_var);
    if (global_var == 325159) {
        global_var = gettid();
    }
    printf("new global: %d\n", global_var);
    return NULL;
}

int main() {
	pthread_t tids[THREAD_COUNT];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    for (int i = 0; i < THREAD_COUNT; i++) {
        err = pthread_create(&tids[i], NULL, mythread, NULL);
        printf("Value from first arg: %ld\n", tids[i]);
        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            return -1;
        }
    }
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        err = pthread_join(tids[i], NULL);
        if (err) {
            perror("error pthread join");
            return -1;
        }
    }
    
    sleep(1000);

	return 0;
}

