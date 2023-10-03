#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define THREAD_COUNT 3


/* This thread blocks all signals */
void *mythread1(void *arg) {
    printf("Pid: %d Tid: %d\n", getpid(), gettid());
    int err;
    sigset_t *set = arg;
    sigfillset(set);

    err = pthread_sigmask(SIG_BLOCK, set, NULL);
    if (err) {
        perror("mythread1(): pthread_sigmask() failed");
        return NULL;
    }

    return NULL;
}

void signal_handler(int sig) {
    printf("mythread2 caught SIGINT signal\n");
    exit(EXIT_FAILURE);
}

/* This thread catches SIGINT in signal handler */
void *mythread2(void *arg) {
    printf("Pid: %d Tid: %d\n", getpid(), gettid());
    int err;
    
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = signal_handler;

    err = sigaction(SIGINT, &sa, NULL);
    if (err) {
        perror("mythread2(): sigaction() failed");
        return NULL;
    }

    return NULL;
}

/* This thread catches SIGQUIT via sigwait() */
void *mythread3(void *arg) {
    printf("Pid: %d Tid: %d\n", getpid(), gettid());
    int err, signal;
    sigset_t *set = arg;
    sigemptyset(set);
    sigaddset(set, SIGQUIT);
    err = pthread_sigmask(SIG_UNBLOCK, set, NULL);
    if (err) {
        perror("mythread3(): pthread_sigmask() failed");
        return NULL;
    }

    while (1) {
        err = sigwait(set, &signal);
        if (err) {
            perror("mythread3(): sigwait() failed");
            return NULL;
        }
        printf("mythread3 caught signal: %d\n", signal);
    }

    return NULL;
}

int main() {
    printf("Pid: %d Tid: %d\n", getpid(), gettid());

    pthread_t tids[THREAD_COUNT];
    int err;
    sigset_t set;

    err = pthread_create(&tids[0], NULL, mythread1, &set);
    if (err) {
        perror("main(): pthread_create failed");
        return 1;
    }

    err = pthread_create(&tids[1], NULL, mythread2, &set);
    if (err) {
        perror("main(): pthread_create failed");
        return 1;
    }

    err = pthread_create(&tids[2], NULL, mythread3, &set);
    if (err) {
        perror("main(): pthread_create failed");
        return 1;
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        err = pthread_join(tids[i], NULL);
        if (err) {
            perror("main(): pthread_join failed");
            return 1;
        }
    }

    return 0;
}