#ifndef MYTHREAD_H
#define MYTHREAD_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>

#define MAX_THREADS 10

typedef void *(*start_routine_t)(void *);

typedef struct _uthread {
	int				uthread_id;
	start_routine_t	start_routine;
	void*			arg;
	void*			retval;
	volatile int	joined;
	volatile int 	detached;
	volatile int	finished;
	volatile int	cancelled;
	ucontext_t 		uctx;
} uthread_struct_t;

typedef uthread_struct_t* uthread_t;

uthread_t uthreads[MAX_THREADS];
int uthread_count = 0;
int uthread_cur = 0;

// __thread mythread_t current_thread;

int uthread_create(uthread_t *tid, void *(*start_routine)(void *), void *arg);
int uthread_join(uthread_t tid, void **retval);
int uthread_detach(uthread_t tid);
int uthread_cancel(uthread_t tid);
void uthread_testcancel();
void schedule();


#endif // MYTHREAD_H