#include <stdio.h>
#include "mythread.h"

void *mythread(void *arg) {
    char *str = (char *) arg;

    for (int i = 0; i < 5; i++)  {
        printf("Hello %s\n", str);
        sleep(1);
		// mythread_testcancel();
    }
	// mythread_detach(gtid);
	return "gbye";
}


int main() {
    mythread_t tids[3];
	// void *retval;

	printf("main [%d %d]\n", getpid(), getppid());

	sleep(3);
	for (int i = 0; i < 3; i++) {
    	mythread_create(&tids[i], mythread, "hello from main");
	}
	
	// for (int i = 0; i < 3; i++) {
	// 	mythread_cancel(tids[i]);
	// }
	// mythread_detach(tid);
	for (int i = 0; i < 3; i++) {
		int err = mythread_join(tids[i], NULL);
		if (err != 0) {
			printf("Error. Error code: %d\n", err);
		}
	}

	// printf("main [%d %d %d] thread returned '%s'\n", getpid(), getppid(), gettid(), (char *) retval);

	return 0;
}
