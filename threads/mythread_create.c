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

#define PAGE 4096
#define STACK_SIZE 3*PAGE

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

typedef mythread_struct_t* mythread_t;

mythread_t gtid;

int mythread_startup(void *arg) {
	mythread_struct_t *mythread = (mythread_struct_t *) arg;

	getcontext(&(mythread->uctx));
	if (!mythread->cancelled) {
		mythread->retval = mythread->start_routine(mythread->arg);
	}
	mythread->finished = 1;

	if (!mythread->detached) {
		while (!mythread->joined) {
			sleep(1);
		}
	}
	
	return 0;
}


void *create_stack(off_t size, int tid) {
	char stack_file[128];
	int stack_fd;
	void *stack;

	snprintf(stack_file, sizeof(stack_file), "stack-%d", tid);

	stack_fd = open(stack_file, O_RDWR | O_CREAT, 0660);
	ftruncate(stack_fd, 0);
	ftruncate(stack_fd, size);

	stack = mmap(NULL, size, PROT_NONE, MAP_SHARED, stack_fd, 0);
	close(stack_fd);

	return stack;
}

int mythread_create(mythread_t *tid, void *(*start_routine)(void *), void *arg) {
    static int mythread_id = 0;
	mythread_struct_t *mythread;
	int child_pid;
	void *child_stack;

	mythread_id++;

	printf("mythread_create: creating thread %d\n", mythread_id);

	child_stack = create_stack(STACK_SIZE, mythread_id);

	mprotect(child_stack + PAGE, STACK_SIZE - PAGE, PROT_READ | PROT_WRITE);

	memset(child_stack + PAGE, 0x7f, STACK_SIZE - PAGE);

	mythread = (mythread_struct_t *) (child_stack + STACK_SIZE - sizeof(mythread_struct_t));
	mythread->mythread_id = mythread_id;
	mythread->start_routine = start_routine;
	mythread->arg = arg;
	mythread->retval = NULL;
	mythread->joined = 0;
	mythread->detached = 0;
	mythread->finished = 0;
	mythread->cancelled = 0;

	gtid = mythread;

	child_stack = (void *) mythread;

	printf("child stack %p; mythread_struct %p; \n", child_stack, mythread);

	child_pid = clone(mythread_startup, child_stack, 
				CLONE_VM | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | SIGCHLD, 
				mythread);
	if (child_pid == -1) {
		printf("clone failed: %s\n", strerror(errno));
		exit(-1);
	}

	*tid = mythread;

	return 0;
}

int mythread_join(mythread_t tid, void **retval) {
	mythread_struct_t *mythread = tid;
	
	if (mythread->detached) {
		*retval = NULL;
		return 22;
	}
	printf("thread_join: waiting for the thread %d to finish\n", mythread->mythread_id);

	while (!mythread->finished)
		usleep(1);

	printf("thread_join: the thread %d finished\n", mythread->mythread_id);

	if (retval) {
		*retval = mythread->retval;
	}
	mythread->joined = 1;

	return 0;
}

int mythread_detach(mythread_t tid) {
	mythread_struct_t *mythread = tid;

	mythread->detached = 1;

	return 0;
}

int mythread_cancel(mythread_t tid) {
	mythread_struct_t *mythread = tid;

	mythread->retval = "MYTHREAD_CANCELLED";
	mythread->cancelled = 1;
	
	return 0;
}

void mythread_testcancel() {
	mythread_struct_t *mythread;
	mythread = gtid;
	if (mythread->cancelled) {
		setcontext(&(mythread->uctx));
	}
}

void *mythread(void *arg) {
    char *str = (char *) arg;

    for (int i = 0; i < 5; i++)  {
        printf("Hello %s\n", str);
        sleep(1);
		mythread_testcancel();
    }
	// mythread_detach(gtid);
	return "gbye";
}


int main() {
    mythread_t tid;
	void *retval;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

    mythread_create(&tid, mythread, "hello from main");
	
	// sleep(3);
	// mythread_cancel(tid);
	mythread_detach(tid);

    int err = mythread_join(tid, &retval);
	if (err != 0) {
		printf("Error. Error code: %d\n", err);
	}

	printf("main [%d %d %d] thread returned '%s'\n", getpid(), getppid(), gettid(), (char *) retval);

	return 0;
}
