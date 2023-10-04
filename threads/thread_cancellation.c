#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

static void cleanup_handler(void *arg) {
    if (arg) { 
        free(arg);
        arg = NULL;
    }
}

static void *mythread(void *arg) {
    char *hello_string = (char *) malloc(100 * sizeof(char));
    strcpy(hello_string, "hello world");
    
    pthread_cleanup_push(cleanup_handler, hello_string);

    int count = 0;
    while (1) {
        // printf("Hello from mythread!\n");
        pthread_testcancel();
        count++;
        printf("%s\n", hello_string);
    }

    pthread_cleanup_pop(0);
    
    return NULL;
}

int main() {
	pthread_t tid;
	int err;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

    err = pthread_create(&tid, NULL, mythread, NULL);
    if (err) {
        printf("main(): pthread_create() failed: %s\n", strerror(err));
        return -1;
    }
    sleep(1);

    err = pthread_cancel(tid);
    if (err) {
        perror("error pthread cancel");
        return -1;
    }

    void *res;
    err = pthread_join(tid, &res);
    if (err) {
        perror("error pthread join");
        return -1;
    }

    if (res == PTHREAD_CANCELED) {
        printf("main(): thread was canceled\n");
    }
    else {
        printf("main(): thread was not canceled\n");
    }

	return 0;
}
