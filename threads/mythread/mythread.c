#include "mythread.h"

#define PAGE 4096
#define STACK_SIZE 3*PAGE

static int mythread_startup(void *arg) {
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


static void *create_stack(off_t size, int tid) {
	char stack_file[128];
	int stack_fd;
	void *stack;
	int err_code;

	err_code = snprintf(stack_file, sizeof(stack_file), "stack-%d", tid);
	if (err_code < 0) {
		printf("create_stack(): output error in snprintf()");
	}

	stack_fd = open(stack_file, O_RDWR | O_CREAT, 0660);
	if (stack_fd == -1) {
		perror("create_stack(): stack file opening error");
		return NULL;
	}

	ftruncate(stack_fd, 0);
	ftruncate(stack_fd, size);

	stack = mmap(NULL, size, PROT_NONE, MAP_SHARED, stack_fd, 0);
	if (stack == MAP_FAILED) {
		close(stack_fd);
		perror("create_stack(): mmap failed");
		return NULL;
	}

	err_code = close(stack_fd);
	if (err_code == -1) {
		perror("create_stack(): close stack file error");
	}

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
	if (!child_stack) {
		printf("errror");
		return 1;
	}

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

	// current_thread = mythread;

	child_stack = (void *) mythread;

	// printf("child stack %p; mythread_struct %p; \n", child_stack, mythread);

	child_pid = clone(mythread_startup, child_stack, 
				CLONE_VM | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | SIGCHLD /* | CLONE_SETTLS */, 
				mythread);
	if (child_pid == -1) {
		printf("clone failed: %s\n", strerror(errno));
		return 1;
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
	printf("Cancellation request to thread %d is sent\n", mythread->mythread_id);
	return 0;
}

void mythread_testcancel() {
	// mythread_struct_t *mythread;
	// mythread = current_thread;
	
	// printf("thread %d: mythread->cancelled = %d\n", mythread->mythread_id, mythread->cancelled);
	// if (mythread->cancelled) {
	// 	setcontext(&(mythread->uctx));
	// }
}

