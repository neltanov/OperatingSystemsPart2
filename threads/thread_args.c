#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct my_struct_t {
    int a;
    char *str;
} my_struct_t;


void *mythread(void *arg) {
    printf("mythread [%d %d %d %lu]: Hello from mythread!\n", getpid(), getppid(), gettid(), pthread_self());
    sleep(2);
    printf("mythread [%d %d %d %lu]: Hello from mythread!\n", getpid(), getppid(), gettid(), pthread_self());
    
    my_struct_t *my_struct_p = (my_struct_t *) arg;
    printf("a: %d\nstr: %s\n", my_struct_p->a, my_struct_p->str);
    if (arg) {
        free(arg);
        arg = NULL;
    }
    return NULL;
}

int main() {
	pthread_t tid;
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    // my_struct_t my_struct_p;
    my_struct_t *my_struct_p = malloc(sizeof(my_struct_t));
    my_struct_p->a = 12;
    my_struct_p->str = "hi";

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    err = pthread_create(&tid, &attr, mythread, my_struct_p);
    if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        free(my_struct_p);
        my_struct_p = NULL;
        return -1;
    }

    // pthread_attr_destroy(&attr);
    // if (my_struct_p) {
    //     free(my_struct_p);
    //     my_struct_p = NULL;
    // }
    // err = pthread_join(tid, NULL);
    // if (err) {
    //     perror("error pthread join");
    //     return -1;
    // }

    sleep(1);

	return 0;
}

