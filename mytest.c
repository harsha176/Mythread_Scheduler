/*
 * This code plays with setiter API it generates signal at specific interval and is caught by signal handler.
 *
 */
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include "mythread.h"
#include "futex.h"
#include "mysched.h"

void queues();

#define NR_THREADS 10

void* thr_fn(void* arg) {
	int id = *(int*) arg;
	int i, j = 0;

	while (j < 10) {
		mythread_enter_kernel();
		fprintf(stderr, "%d", id);
		mythread_leave_kernel();
		i = 0;
		while (i < 10000000)
			i++;
		j++;
	}
	return NULL;
}

void* dummy_fn(void* arg) {
	return NULL;
}

int main(int argc, char* argv[]) {
	mythread_t tid[NR_THREADS], dummy;
	int id[NR_THREADS];
	int i = 0;
	mythread_attr_t attr[NR_THREADS];
	mythread_setconcurrency(2);

	mythread_create(&dummy, NULL, dummy_fn, NULL);
	mythread_join(dummy, NULL);

	struct sched_param priority;

	sigset_t sset;
	sigprocmask(SIG_SETMASK, NULL, &sset);
	sigdelset(&sset, SIGALRM);
	sigdelset(&sset, SIGUSR1);

	sigprocmask(SIG_SETMASK, &sset, NULL);

        fprintf(stderr, "Scheduling order of 10 threads with priorites ranging from 10(for thread 9) to 19(for thread 0)\n"); 
        fprintf(stderr, "Please wait as it takes some time...\n");

	for(i = 0; i < NR_THREADS; i++) {
		mythread_attr_init(&attr[i]);
		id[i] = i;
		priority.__sched_priority = NR_THREADS -i + 10;
		mythread_attr_setschedparam(&attr[i], &priority);
		mythread_create(&tid[i], &attr[i], thr_fn, &id[i]);
	}

	for (i = 0; i < NR_THREADS; i++) {
		mythread_join(tid[i], NULL);
	}

	printf("\nMain thread leaving \n");
	mythread_exit(NULL);

        return 0;
}

