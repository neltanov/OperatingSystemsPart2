#include "uthread.h"

#define PAGE 4096
#define STACK_SIZE 3*PAGE


void schedule() {
	int err;
	ucontext_t *current_ctx, *next_ctx;

	current_ctx = &(uthreads[uthread_cur]->uctx);
	uthread_cur = (uthread_cur + 1) % uthread_count;
	next_ctx = &(uthreads[uthread_cur]->uctx);

	err = swapcontext(current_ctx, next_ctx);
	if (err == -1) {
		printf("schedule(): swapcontext() failed: %s\n", strerror(errno));
		return;
	}
}

void uthread_startup(/* void *arg */) {
	int i;
	ucontext_t *ctx;
	
	for (int i = 1; i < uthread_count; i++ ) {
		ctx = &uthreads[i]->uctx;
		char *stack_begin = ctx->uc_stack.ss_sp;
		char *stack_end = ctx->uc_stack.ss_sp + ctx->uc_stack.ss_size;

		if (stack_begin <= (char *)&i && (char *)&i <= stack_end) {
			printf("Starting thread %d; thread function address: %p\n", uthreads[i]->uthread_id, uthreads[i]->start_routine);
			uthreads[i]->retval = uthreads[i]->start_routine(uthreads[i]->arg);
		}
	}
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

int uthread_create(uthread_t *tid, void *(*start_routine)(void *), void *arg) {
    static int uthread_id = 0;
	uthread_struct_t *uthread;
	int err;
	void *stack;

	uthread_id++;

	printf("uthread_create: creating thread %d\n", uthread_id);

	stack = create_stack(STACK_SIZE, uthread_id);

	if (!stack) {
		printf("uthread_create(): stack create error");
		return 1;
	}

	mprotect(stack + PAGE, STACK_SIZE - PAGE, PROT_READ | PROT_WRITE);

	memset(stack + PAGE, 0x7f, STACK_SIZE - PAGE);

	uthread = (uthread_struct_t *) (stack + STACK_SIZE - sizeof(uthread_struct_t));
	uthread->uthread_id = uthread_id;
	uthread->start_routine = start_routine;
	uthread->arg = arg;
	uthread->retval = NULL;
	uthread->joined = 0;
	uthread->finished = 0;

	err = getcontext(&uthread->uctx);
	if (err == -1) {
		printf("uthread_create(): getcontext() error: %s\n", strerror(errno));
		return 1;
	}
	uthread->uctx.uc_stack.ss_sp = stack;
	uthread->uctx.uc_stack.ss_size = STACK_SIZE - sizeof(uthread_struct_t);
	uthread->uctx.uc_link = NULL;

	makecontext(&uthread->uctx, uthread_startup, 0);

	uthreads[uthread_count] = uthread;
	uthread_count++;

	*tid = uthread;

	return 0;
}

int uthread_join(uthread_t tid, void **retval) {
	uthread_struct_t *mythread = tid;
	
	printf("uthread_join(): waiting for the thread %d to finish\n", mythread->uthread_id);

	while (!mythread->finished)
		usleep(1);

	printf("uthread_join(): the thread %d finished\n", mythread->uthread_id);

	if (retval) {
		*retval = mythread->retval;
	}
	mythread->joined = 1;

	return 0;
}

