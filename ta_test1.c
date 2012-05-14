/**
 * Test Case for Homework 3 p5
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "mythread.h"
#include "mymutex.h"

#define THREAD_COUNT 7
#define CONCURRENCY_COUNT 1

#define OUTPUT_LENGTH 1024
static char actualOutput[OUTPUT_LENGTH] = "";
static const char *EXPECTED_OUTPUT1 = "6543201";
static const char *EXPECTED_OUTPUT2 = "6543210";

int score = 5;	// initial score

mythread_mutex_t mutex;
int temp;

static void checkResults(char *string, int rc) {
	if (rc) {
		printf("Error on : %s, rc=%d", string, rc);
		exit(EXIT_FAILURE);
	}
	return;
}

void show_sched_param(pthread_attr_t *a) {
	int rc = 0;
	struct sched_param param;

	rc = mythread_attr_getschedparam(a, &param);
	checkResults("pthread_attr_getschedparam()\n", rc);

	printf("param.sched_priority = %d\n",
			param.__sched_priority);
	return;
}

void* thread_func(void *args) {
	int t = 0;
	char buf[10];

	mythread_t self = mythread_self();

	sprintf(buf, "%ld", (long) args);
	mythread_mutex_lock(&mutex);
	strcat(actualOutput, buf);
	mythread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
	int rc = 0;
	mythread_t th[THREAD_COUNT];
	mythread_attr_t *attr[THREAD_COUNT];
	struct sched_param sparam1;
	long i;

	mythread_setconcurrency(CONCURRENCY_COUNT);

	//printf("concurrency is %d\n", mythread_getconcurrency());

	mythread_mutex_init(&mutex, NULL);

	for (i = 0; i < THREAD_COUNT; i++) {
		attr[i] = malloc(sizeof(mythread_attr_t));
		mythread_attr_init(attr[i]);
		switch (i) {
		case 2:
			sparam1.__sched_priority = 9;
			mythread_attr_setschedparam(attr[i], &sparam1);
			break;
		case 3:
			sparam1.__sched_priority = 8;
			mythread_attr_setschedparam(attr[i], &sparam1);
			break;
		case 4:
			sparam1.__sched_priority = 7;
			mythread_attr_setschedparam(attr[i], &sparam1);
			break;
		case 5:
			sparam1.__sched_priority = 6;
			mythread_attr_setschedparam(attr[i], &sparam1);
			break;
		case 6:
			sparam1.__sched_priority = 5;
			mythread_attr_setschedparam(attr[i], &sparam1);
			break;
		default:
			break;
		}

		mythread_create(&th[i], attr[i], thread_func, (void*) i);
	}

	for (i = 0; i < THREAD_COUNT; i++) {
		mythread_join(th[i], NULL);
	}

	/* Test: mythread_attr_getschedparam()*/
	for (i = 0; i < THREAD_COUNT; i++) {
		int rc = 0;
		struct sched_param sparam2;
		rc = mythread_attr_getschedparam(attr[i], &sparam2);
		if(!rc){
			switch (i) {
			case 2:
				sparam2.__sched_priority = 9;
				score = score + 0.7;
				break;
			case 3:
				sparam2.__sched_priority = 8;
				score = score + 0.7;
				break;
			case 4:
				sparam2.__sched_priority = 7;
				score = score + 0.7;
				break;
			case 5:
				sparam2.__sched_priority = 6;
				score = score + 0.7;
				break;
			case 6:
				sparam2.__sched_priority = 5;
				score = score + 0.7;
				break;
			default:
				break;
			}
		}
//		printf("sparam2.sched_priority = %d\n",
//				sparam2.__sched_priority);
		//show_sched_param(attr[i]);

	}

	/* Test 1*/
	printf("# Test Case 1:");
	//printf("Expected Output: %s\n", EXPECTED_OUTPUT1);
	//printf("Actual Output: %s\n", actualOutput);
	if (!strcmp(EXPECTED_OUTPUT1, actualOutput) ||
			!strcmp(EXPECTED_OUTPUT2, actualOutput)) {
		printf("PASS\n");
		score = score + 25;
	} else {
		printf("FAIL\n");
	}


	/*Test: mythread_attr_destroy() */
	int deduction = 0;
	for (i = 0; i < THREAD_COUNT; i++) {
		rc = mythread_attr_destroy(attr[i]);
		if(rc){
			deduction = deduction + 0.5;
		}
		free(attr[i]);
	}
	if(rc){
		score = score - deduction;
		printf("Deduction occurred.\n");
	}

	mythread_mutex_destroy(&mutex);

	printf("## Score: %d\n", score);

	return 0;
}
