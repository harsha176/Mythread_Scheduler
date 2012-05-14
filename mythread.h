/*
 * mythread.h -- interface of user threads library
 */

#ifndef MYTHREAD_H
#define MYTHREAD_H

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include <pthread.h>
#include <sys/times.h>
#include "myqueue.h"
#include "futex.h"

#define DEFAULT_ATTR 10	/* Default value of sched_priority */

typedef struct mythread_attr { /* thread attributes */
  int attr;                    /* Use for sched_priority */
} mythread_attr_t;

typedef struct mythread {      /* thread control block */
  pid_t tid;
  struct futex block;
  struct mythread *locknext;   /* next pointer for MCS lock */
  int locked;                  /* MCS status for spin lock/backoff */
  void *stack;
  int state;                   /* state of execution */
  void * (*start_func)(void*); /* thread_func to be called */
  void *arg;                   /* thread_arg arguments of thread_func */
  mythread_queue_t joinq;      /* Q of threads waiting for my return */
  void *returnValue;           /* Pointer to detached function's return value */
  int reschedule;              /* if set to signo then reschedule ASAP */
  struct tms ts;               /* last timestamp recorded at timeslice */
  mythread_attr_t *attribute;        /* thread attribute */
} *mythread_t;


/*
 * gettid - syscall to SYS_gettid
 */
static pid_t gettid(void);

/*
 * tgkill - syscall to SYS_tgkill
 */
static long tgkill(int tgid, int pid, int sig);

/*
 * myswapcontext - swap context
 */
void myswapcontext(mythread_t oth, mythread_t nth);

/*
 * mythread_readyq - pointer to ready q
 */
mythread_queue_t *mythread_readyq(void);

/*
 * mythread_runq - pointer to run q
 */
mythread_queue_t *mythread_runq(void);

/*
 * mythread_setconcurrency - set the number of LWPs
 * (max. number of parallel threads)
 */
void mythread_setconcurrency(int new_level);

/*
 * mythread_getconcurrency - return the number of LWPs
 * (max. number of parallel threads)
 */
int mythread_getconcurrency(void);

/*
 * mythread_init_kernel - call futex_init
 */
static void mythread_init_kernel(void);

/*
 * mythread_enter_kernel - enter the monolithic threading kernel
 */
void mythread_enter_kernel(void);

/*
 * mythread_tryenter_kernel - enter the monolithic threading kernel IF not busy
 * return TRUE on success, FALSE o/w
 */
int mythread_tryenter_kernel(void);

/*
 * mythread_leave_kernel_nonpreemptive - leave the monolithic threading kernel
 */
void mythread_leave_kernel_nonpreemptive(void);

/*
 * mythread_self - thread id of running thread
 */
mythread_t mythread_self(void);

/*
 * mythread_continue - move item from q to run Q and resume execution of item
 */
static void mythread_continue(mythread_queue_t *q, void *item);

/*
 * mythread_suspend_and_leave_kernel - move item from run Q to q and suspend
 */
static void mythread_suspend_and_leave_kernel(mythread_queue_t *q, void *item);

/*
 * mythread_block_phase1_kernel - see mythread_block_phase1() below except
 * post: still in monolithic kernel
 */
void mythread_block_phase1_kernel(mythread_queue_t *q, int state);

/*
 * mythread_block_phase1 - prepare to block (see block op below):
 * remove currently running thread off run q,
 * if q is non-NULL, enq this thread on q but DO NOT suspend the thread,
 * add the state flags to the thread's state (via bit-wise OR | state)
 * pre: must be in monolithic kernel
 * post: return outside the monolithic kernel
 * NOTICE: use in conjunction with block as follows:
 *   mythread_enter_kernel();
 *   mythread_block_phase1(q, state);
 *   some_other_operation();
 *   mythread_enter_kernel();
 *   mythread_block_phase2();
 */
void mythread_block_phase1(mythread_queue_t *q, int state);

/*
 * mythread_block_phase2_kernel - see mythread_block_phase2() except
 * post: still in monolithic kernel
 */
void mythread_block_phase2_kernel(void);

/*
 * mythread_block_phase2 - commit to block (see block op below):
 * suspend the thread,
 * and activate ready threads if an LWP becomes available
 * pre: must be in monolithic kernel
 * post: return outside the monolithic kernel
 */
void mythread_block_phase2(void);

/*
 * mythread_block - remove currently running thread off run q (if present),
 * if q is non-NULL and, enq this thread on q,
 * always suspend the thread,
 * add the state flags to the thread's state (via bit-wise OR | state)
 * and activate ready threads if an LWP becomes available
 * pre: must be in monolithic kernel
 * post: return outside the monolithic kernel
 */
void mythread_block(mythread_queue_t *q, int state);

/*
 * mythread_unblock - resumes the thread at the head of q,
 * remove the state flags to the thread's state (via bit-wise AND & ~state)
 * and activate the thread if an LWP becomes available (o/w mark as ready)
 * pre: must be in monolithic kernel
 * post: return outside the monolithic kernel
 */
void mythread_unblock(mythread_queue_t *q, int state);

/*
 * mythread_unblock_thread - resumes the target thread from q,
 * remove the state flags to the thread's state (via bit-wise AND & ~state)
 * and activate the thread if an LWP becomes available (o/w mark as ready)
 * pre: must be in monolithic kernel
 * post: return outside the monolithic kernel
 */
void mythread_unblock_thread(mythread_queue_t *q, mythread_t target, int state);

/*
 * mythread_exit - exit thread, awakes joiners on return
 * from thread_func and dequeue itself from run Q before dispatching run->next
 */
void mythread_exit(void *retval);

/*
 * mythread_wrapper - calls thread_func(thread_arg), then exits thread
 */
static mythread_wrapper(void *arg);

/*
 * mythread_newthread - allocates a TCB that th will point to afterwards,
 * creates a context that will invoke start_func(arg) inside mythread_wrapper,
 * and allocates a stack of size stacksize for this new thread.
 */
static void mythread_newthread(mythread_t *th,
			       void * (*start_func)(void *),
			       void *arg,
			       const mythread_attr_t *attr,
			       int stacksize);

/*
 * mythread_create - prepares context of new_thread_ID as start_func(arg),
 * attr is ignored right now.
 * If mythread_init is FALSE, the ready and the run Q are initialized.
 * Threads are activated (run) according to the number of available LWPs
 * or are marked as ready.
 */
int mythread_create(mythread_t *new_thread_ID,
		    const mythread_attr_t *attr,
		    void * (*start_func)(void *),
		    void *arg);

/*
 * mythread_yield - switch from running thread to the next ready one
 */
int mythread_yield(void);

/*
 * mythread_join - suspend calling thread if target_thread has not finished,
 * enqueue on the join Q of the target thread, then dispatch ready thread;
 * once target_thread finishes, it activates the calling thread / marks it
 * as ready.
 */
int mythread_join(mythread_t target_thread, void **status);

#endif /* MYTHREAD_H */







