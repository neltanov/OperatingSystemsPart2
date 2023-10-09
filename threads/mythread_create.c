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

#define PAGE 4096
#define STACK_SIZE 3*PAGE

typedef void *(*start_routine_t)(void *);

typedef struct _mythread {
	int				mythread_id;
	start_routine_t	start_routine;
	void*			arg;
	void*			retval;
	volatile int	joined;
	volatile int	finished;
} mythread_struct_t;

typedef mythread_struct_t* mythread_t;

int mythread_startup(void *arg) {
	mythread_struct_t *mythread = (mythread_struct_t *) arg;

	mythread->retval = mythread->start_routine(mythread->arg);
	mythread->finished = 1;

	// TODO change to futex
	while (!mythread->joined) {
		sleep(1);
	}
	
	return 0;
}


void *create_stack(off_t size, int mytid) {
	char stack_file[128];
	int stack_fd;
	void *stack;

	snprintf(stack_file, sizeof(stack_file), "stack-%d", mytid);

	stack_fd = open(stack_file, O_RDWR | O_CREAT, 0660);
	ftruncate(stack_fd, 0);
	ftruncate(stack_fd, size);

	stack = mmap(NULL, size, PROT_NONE, MAP_SHARED, stack_fd, 0);
	close(stack_fd);

	return stack;
}

int mythread_create(mythread_t *mytid, void *(*start_routine)(void *), void *arg) {
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
	mythread->finished = 0;
	mythread->joined = 0;

	child_stack = (void *) mythread;

	printf("child stack %p; mythread_struct %p; \n", child_stack, mythread);

	child_pid = clone(mythread_startup, child_stack, 
				CLONE_VM | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | SIGCHLD, 
				mythread);
	if (child_pid == -1) {
		printf("clone failed: %s\n", strerror(errno));
		exit(-1);
	}

	*mytid = mythread;

	return 0;
}

int mythread_join(mythread_t mytid, void **retval) {
	mythread_struct_t *mythread = mytid;
	
	printf("thread_join: waiting for the thread %d to finish\n", mythread->mythread_id);

	while (!mythread->finished)
		usleep(1);

	printf("thread_join: the thread %d finished\n", mythread->mythread_id);

	*retval = mythread->retval;
	mythread->joined = 1;

	return 0;
}

void *mythread(void *arg) {
    char *str = (char *) arg;

    for (int i = 0; i < 5; i++)  {
        printf("Hello %s\n", str);
        sleep(1);
    }

	return "gbye";
}


int main() {
    mythread_t mytid;
	void *retval;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

    mythread_create(&mytid, mythread, "hello from main");

    mythread_join(mytid, &retval);

	printf("main [%d %d %d] thread returned '%s'\n", getpid(), getppid(), gettid(), (char *) retval);

	return 0;
}
