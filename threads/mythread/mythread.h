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

typedef void *(*start_routine_t)(void *);

typedef struct _mythread {
	int				mythread_id;
	start_routine_t	start_routine;
	void*			arg;
	void*			retval;
	volatile int	joined;
	volatile int 	detached;
	volatile int	finished;
	volatile int	cancelled;
	ucontext_t 		uctx;

} mythread_struct_t;

// __thread mythread_t current_thread;

typedef mythread_struct_t* mythread_t;

int mythread_create(mythread_t *tid, void *(*start_routine)(void *), void *arg);
int mythread_join(mythread_t tid, void **retval);
int mythread_detach(mythread_t tid);
int mythread_cancel(mythread_t tid);
void mythread_testcancel();


#endif // MYTHREAD_H