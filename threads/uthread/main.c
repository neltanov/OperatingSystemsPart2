#include <stdio.h>
#include "uthread.h"

void *uthread1(void *arg) {
    char *str = (char *) arg;

    for (int i = 0; i < 3; i++)  {
        printf("uthread1 | %s. Thread function address: %p\n", str, uthread1);
        sleep(1);
		schedule();
    }
	return "gbye1";
}

void *uthread2(void *arg) {
    char *str = (char *) arg;

    for (int i = 0; i < 3; i++)  {
        printf("uthread2 | %s. Thread function address: %p\n", str, uthread2);
        sleep(1);
		schedule();
    }
	return "gbye2";
}

void *uthread3(void *arg) {
    char *str = (char *) arg;

    for (int i = 0; i < 3; i++)  {
        printf("uthread3 | %s. Thread function address: %p\n", str, uthread3);
        sleep(1);
		schedule();
    }
	return "gbye3";
}

int main() {
	uthread_struct_t main_thread;
	uthreads[0] = &main_thread;
	uthread_count = 1;

    uthread_t tids[3];

	printf("main [%d %d]\n", getpid(), getppid());

	uthread_create(&tids[0], uthread1, "hello from main to thread1");
	uthread_create(&tids[1], uthread2, "hello from main to thread2");
	uthread_create(&tids[2], uthread3, "hello from main to thread3");

	while (1) {
		schedule();
	}
	
	return 0;
}
