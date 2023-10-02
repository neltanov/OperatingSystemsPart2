#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void *mythread(void *arg) {
    printf("mythread [%d %d %d %lu]: Hello from mythread!\n", getpid(), getppid(), gettid(), pthread_self());
    
    int *thread_int_retval = malloc(sizeof(int));
    *thread_int_retval = 42;

    return (void *)thread_int_retval;
}

void *mythread1(void *arg) {
    printf("mythread [%d %d %d %lu]: Hello from mythread1!\n", getpid(), getppid(), gettid(), pthread_self());
    
    char *thread_int_retval = malloc(sizeof(char));
    thread_int_retval = "hello world";

    return (void *)thread_int_retval;
}


int main() {
	pthread_t tids[2];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    err = pthread_create(&tids[0], NULL, mythread, NULL);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return -1;
    }

    err = pthread_create(&tids[1], NULL, mythread1, NULL);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return -1;
    }

    void *thread_retval;
    err = pthread_join(tids[0], &thread_retval);
    if (err) {
        perror("error pthread join");
        return -1;
    }

    int *thread_int_retval = (int *)thread_retval;
    printf("Thread integer return value: %d\n", *thread_int_retval);

    err = pthread_join(tids[1], &thread_retval);
    if (err) {
        perror("error pthread join");
        return -1;
    }

    char *thread_char_retval = (char *)thread_retval;
    printf("Thread string return value: %s\n", thread_char_retval);
    
    sleep(2);

	return 0;
}

