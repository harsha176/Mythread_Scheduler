
#include "mythread.h"
#include "mymutex.h"
static int count = 0;
#define THREAD_COUNT 100
#define CONCURRENCY_NUM 5

void my_dummy(void) {
  return;
}

int score = 10;

mythread_mutex_t mutex;

void *thread_func(void* arg)
{
	unsigned long long i = 0,j=0;
	double d = 0.0;
	while(i<10000)
	{
		while(j < 10000)
		{
			d = 1000000.5343/44343.674;
			j++;
		}
		i++;
	}

	mythread_mutex_lock(&mutex);
	count++;
	mythread_mutex_unlock(&mutex);

	//printf("\n%d Finished",count);
	//fflush(stdout);
	//printf("\nFinally %u is exiting",mythread_self());

	mythread_exit(NULL);
	
}

int main(int argc, char *argv[])
{
  mythread_t th[200], dummy;
	int i = 0;	
//	if (argc == 1) {
//	  printf("use: %s <n>\n", argv[0]);
//	  exit(1);
//	}
//	printf("argv[1]=%s\n", argv[1]);
	mythread_setconcurrency(CONCURRENCY_NUM);
	mythread_create(&dummy, NULL, (void *) my_dummy, NULL);
	for(i = 0;i<THREAD_COUNT;i++)
	{
		mythread_create(&th[i],NULL,(void*)thread_func,NULL);
	}
	
	for(i = 0;i<THREAD_COUNT;i++)
	{
		mythread_join(th[i],NULL);
	}

	/* Test Case 2*/
	printf("# Test Case 2:");
	score = score + count / 5;
	printf("## Score = %d\n", score);

	return 0;
}
