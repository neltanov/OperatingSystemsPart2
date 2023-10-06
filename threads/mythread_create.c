#define PAGE 4096
#define STACK_SIZE PAGE*8

typedef void *(*start_routine_t)(void *);

typedef struct _mythread {
	int				mythread_id;
	start_routine_t	start_routine;
	void*			arg;
	void*			retval;
	volatile int	joined;
	volatile int	exited;
} mythread_struct_t;


int mythread_startup(void *arg) {
	mythread_struct_t *mythread = (mythread_struct_t *) arg;

	mythread->retval = mythread->start_routine(mythread->arg);
	mythread->exited = 1;

	//wait until join;
	while (mythread->joined) {
		sleep(1);
	}
	
	return 0;
}


void * create_stack(off_t size, int thread_mum) {
	int stack_fd;
	void *stack;

	snprintf(stack_file, sizeof(stack_file), "stack-%d", thread_num);




}

int mythread_create(mythread_t *mytid, start_routine_t start_routine, void *arg) {
    static int thread_num = 0;
	mythread_struct_t *mythread;
	

	thread_num++;

	child_stack = create_stack(STACK_SIZE, thread_num);
	
	mythread->mythread_id = thread_num;
	mythread->start_routine = start_routine;
	mythread->arg = arg;

	int child_pid = clone(mythread_startup, child_stack + STACK_SIZE, CLONE_VM | CLONE_FILES | CLONE_THREAD | CLONE_SIGHAND | CLONE_SIGCHLD, (void *) mythread);
	
	// check
}

int mythread_join(mythread_t mytid, void **retval) {
	mythread_struct_t *mythread = mytid;
	// wait until thread ends
	
	while (!mythread->exited)
		sleep(1);

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
}


int main() {
    mythread_t tid;

    mythread_create(mythread_t &tid, mythread, "hello from main");
    mythread_join(tid, &retval);
}
