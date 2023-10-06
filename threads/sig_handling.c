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
    printf("mythread1 | Pid: %d | Tid: %d | pthread_self: %ld\n", getpid(), gettid(), pthread_self());
    int err;
    sigset_t set;
    sigfillset(&set);

    err = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (err) {
        perror("mythread1(): pthread_sigmask() failed");
        return NULL;
    }

    for (;;) {}

    return NULL;
}

void signal_handler(int sig) {
    printf("mythread2 caught SIGINT signal | Tid: %ld\n", pthread_self());
}

/* This thread catches SIGINT in signal handler */
void *mythread2(void *arg) {
    printf("mythread2 | Pid: %d | Tid: %d | pthread_self: %ld\n", getpid(), gettid(), pthread_self());
    int err;
    
    struct sigaction sa;
    sa.sa_flags = 0;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    err = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    if (err) {
        perror("mythread2(): pthread_sigmask() failed");
        return NULL;
    }

    sigfillset(&sa.sa_mask);
    sigdelset(&sa.sa_mask, SIGINT);

    sa.sa_handler = signal_handler;

    err = sigaction(SIGINT, &sa, NULL);
    if (err) {
        perror("mythread2(): sigaction() failed");
        return NULL;
    }

    for (;;) {}
    return NULL;
}

/* This thread catches SIGQUIT via sigwait() */
void *mythread3(void *arg) {
    printf("mythread3 | Pid: %d | Tid: %d | pthread_self: %ld\n", getpid(), gettid(), pthread_self());
    int err, signal;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);

    while (1) {
        err = sigwait(&set, &signal);
        if (err) {
            perror("mythread3(): sigwait() failed");
            return NULL;
        }
        printf("mythread3 caught signal: %d | Tid: %ld\n", signal, pthread_self());
    }

    return NULL;
}

int main() {
    printf("main | Pid: %d | Tid: %d | pthread_self: %ld\n", getpid(), gettid(), pthread_self());

    pthread_t tids[THREAD_COUNT];
    int err;

    sigset_t set;
    sigfillset(&set);
    err = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (err) {
        perror("main(): pthread_sigmask() failed");
        return 1;
    }

    err = pthread_create(&tids[0], NULL, mythread1, NULL);
    if (err) {
        perror("main(): pthread_create failed");
        return 1;
    }

    err = pthread_create(&tids[1], NULL, mythread2, NULL);
    if (err) {
        perror("main(): pthread_create failed");
        return 1;
    }

    err = pthread_create(&tids[2], NULL, mythread3, NULL);
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