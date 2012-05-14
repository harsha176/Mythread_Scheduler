/*
 * This file implements preemptive scheduler functions.
 * It mainly implements following functions.
 * 		1. mythread_scheduler()
 * 		2. mythread_signal_handler()
 * 		3. mythread_leave_kernel
 * The below functions are used to configure priority of threads.
 * 		4. mythread_attr_init
 * 		5. mythread_attr_destroy
 * 		6. mythread_attr_getschedparam
 * 		7. mythread_attr_setschedparam
 */
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <syscall.h>
#include <string.h>

#include "mythread.h"
#include "futex.h"
#include "mysched.h"
#include "myqueue.h"

#define INTERVAL 10000           // Time slice of the scheduler which is 10ms
#define BLOCKED  64				 // State flag that is used to mark a thread blocked on mythread_block
#define STATE_SIGUSR1 4			 // State flag used to indicate that SIGUSR1 has been received by the thread
#define STATE_SIGALRM 8		     // State flag used to indicate that SIGALRM has been received by the thread
/*Initial signal mask that is restored every time mythread_exit is called*/
sigset_t initial_sigmask;
/*Initial signal handlers that is restored every time  mythread_exit is called*/
struct sigaction initial_timer_sigaction;
struct sigaction initial_usr_sigaction;

/*This function tests whether preemption is required for the current thread*/
static int mythread_scheduler();

/*This function is used to reschedule threads again that are needed to be scheduled*/
static void reschedule_threads();

int get_priority_by_item(mythread_t tid);

/*ONLY FOR DEBUGGING: These functions are used to print run and ready queues*/
#if 0
void print_queue(mythread_queue_t* q) {
	mythread_queue_t queue = *q;
	mythread_t tid = mythread_self();
	fprintf(stderr, "Thread[%d]\t", tid->tid);
	while (queue != NULL) {
		mythread_t tid = ((mythread_t)((queue)->item));
		fprintf(stderr, "%d\t", tid->tid);
		queue = queue->next;
	}
	fprintf(stderr, "\n");
}
void queues() {
	mythread_enter_kernel();
	fprintf(stderr, "ready queue contents\n");
	print_queue(mythread_readyq());

	fprintf(stderr, "run queue contents\n");
	print_queue(mythread_runq());
	mythread_leave_kernel();
}
#endif

/**
 * The main responsibility of signal handler is to preempt the current thread when all the following conditions are met
 * 	  1. Its inside the kernel
 * 	  2. mythread_scheduler() returns 1  to reschedule current thread
 * 	  3. Its reschedule flag is 1
 * 	  4. Its not in block state
 * 	Once the above conditions are met then block the current thread.
 *
 * 	If mythread enter kernel fails then come out of kernel non preemptively
 * 	Finally call reschedule other threads
 */
static void signal_handler(int sig) {

	mythread_t self = mythread_self();

	/*
	 * Need to store the type of signal inorder to determine sending to SIGUSR1 to other threads in run queue
	 */
	if (sig == SIGALRM)
		self->state |= STATE_SIGUSR1;
	else
		self->state |= STATE_SIGALRM;

	/*
	 * Set the flag for the current thread to be rescheduled.
	 */
	self->reschedule = 1;

	mythread_queue_t* run_q = mythread_runq();

	if (mythread_tryenter_kernel() == TRUE) {
		void* item = (mythread_deq_prio(mythread_readyq()));
		mythread_t next_tid = ((mythread_t) item);
		/*
		 * schedule if its reschedule flag is 1, mythread_scheduler says its need to be rescheduled
		 * , if it is not in BLOCKED state and the current thread is in run_q
		 */
		if (self->reschedule == 1 && mythread_scheduler()
				&& !(self->state & BLOCKED) && mythread_inq(run_q, self)) {
			/*
			 * reset reschedule flag as it is going to be blocked and there cannot be any interleaving here as it is inside
			 * kernel, finally preempt current thread
			 */

			if (get_priority_by_item(next_tid) <= get_priority_by_item(self)) {
				self->reschedule = 0;
				mythread_block(mythread_readyq(), BLOCKED);
				self->state &= ~(BLOCKED); // remove BLOCKED state flag once it resumes execution
			} else {
				self->reschedule = 0;
				mythread_leave_kernel_nonpreemptive();
			}
		} else {
			/*
			 * If unable to enter kernel then leave kernel without preempting current thread.
			 */
			mythread_leave_kernel_nonpreemptive();
		}
	}
	/*
	 * reschedule threads if they are to be rescheduled.
	 * This is required as there could be threads that executed signal handler but did not preempt the thread
	 */
	reschedule_threads();
}

/*
 * This function reschedules threads that were not able to preempt.
 */
static void reschedule_threads() {
	/*
	 * 	  First see if there are any threads in the run queue that needs to be preempted
	 * 	  If so reschedule them by sending SIGUSR1 to threads with resechedule flag is set
	 * 	  and not in blocked state.
	 */
	mythread_queue_t head = *mythread_runq();
	mythread_t self_tid = mythread_self();

	while (head != NULL) {
		mythread_t curr_tid = ((mythread_t) head->item);
		if (curr_tid != self_tid && curr_tid->reschedule == 1
				&& !(curr_tid->state & BLOCKED)) {
			syscall(SYS_tkill, curr_tid->tid, SIGUSR1);
		}
		head = head->next;
	}
}

/*
 * First leave from non preemptive kernel and then call
 * call reschedule_threads
 */
void mythread_leave_kernel() {
	mythread_leave_kernel_nonpreemptive();
	//reschedule if there are any other threads.
	reschedule_threads();
}

/* 1. Send SIGUSR1 to all threads if the current thread received SIGALRM
 * 2. Check if the current thread can be rescheduled
 */
static int mythread_scheduler() {
	mythread_queue_t ptr, head;
	mythread_t self = mythread_self();

	/* If we got here via a SIGALRM, then, send SIGUSR1 to everyone */
	if (self->state & STATE_SIGUSR1) {
		head = *mythread_runq();
		ptr = head;
		do {
			if (ptr == NULL) {
				break;
			}
			if (self != ptr->item) {
				syscall(SYS_tkill, ((mythread_t) ptr->item)->tid, SIGUSR1);
			}
			ptr = ptr->next;
		} while (ptr != head);

	}
	/* Reset all signal states */
	self->state &= ~STATE_SIGUSR1;
	self->state &= ~STATE_SIGALRM;

	//self->reschedule = 0;
	if (*mythread_readyq() != NULL)
		return 1;
	else
		return 0;
}

/* Global structures to save old data to restore them */
struct sigaction sig_act;
struct sigaction old_sig_act;

sigset_t newmask;
sigset_t oldmask;

static int timer_initialised = 0;

/* init_sched */
void mythread_init_sched() {
	struct itimerval timer;
	struct timeval timerval;

	memset(&sig_act, '\0', sizeof(sig_act));
	sig_act.sa_handler = signal_handler;
	sig_act.sa_flags = SA_RESTART;

	/* Install SIGALRM and save old sigaction */
	if (sigaction(SIGALRM, &sig_act, &old_sig_act) == -1) {
		printf("Error in registering the Signal Handler for SIGALRM!\n");
		printf("Exiting....");
		exit(-1);
	}

	/* Install SIGUSR1 and save old sigaction */
	if (sigaction(SIGUSR1, &sig_act, &old_sig_act) == -1) {
		printf("Error in registering the Signal Handler for SIGUSR1!\n");
		printf("Exiting....");
		exit(-1);
	}

	/* Unmask signals */
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);
	sigaddset(&newmask, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &newmask, &oldmask);

	/* Start the timer */
	if (timer_initialised == 0) {
		timer_initialised = 1;
		timerval.tv_sec = 0;
		timerval.tv_usec = 10000;
		timer.it_interval = timerval;
		timer.it_value = timerval;
		setitimer(ITIMER_REAL, &timer, NULL);
	}
}

void mythread_exit_sched() {
	/* Restore old SIGUSR1 handler */
	if (sigaction(SIGUSR1, &old_sig_act, &sig_act) == -1) {
		printf("Error in removing the signal handler for SIGUSR1!\n");
		printf("Exiting....\n");
		exit(-1);
	}

	/* Restore old SIGALRM handler */
	if (sigaction(SIGALRM, &old_sig_act, &sig_act) == -1) {
		printf("Error in removing the Signal Handler for SIGALRM!\n");
		printf("Exiting....\n");
		exit(-1);
	}

	/* Restore old signal mask */
	sigprocmask(SIG_SETMASK, &oldmask, &newmask);
}

/*
 * pthread_attr_init - initialize a thread attributes object attr
 * with the default value for all of the individual attributes used
 * by a given implementation.
 */
int mythread_attr_init(mythread_attr_t *attr) {
// set the attribute for current thread
	if (!attr) {
		return -1;
	}
	attr->attr = DEFAULT_ATTR;
	return 0;
}

/*
 * pthread_attr_destroy - destroy a  thread  attributes object
 */
int mythread_attr_destroy(mythread_attr_t *attr) {
	if (!attr) {
		return -1;
	}
	attr->attr = DEFAULT_ATTR;
	return 0;
}

/*
 * pthread_attr_getschedparam - get the scheduling parameter
 * attributes in the attr argument
 */
int mythread_attr_getschedparam(const mythread_attr_t *attr,
		struct sched_param *param) {
	if (!attr || !param) {
		return -1;
	}
	param->sched_priority = attr->attr;
	return 0;
}

/*
 * pthread_attr_setschedparam - set the scheduling parameter
 * attributes in the attr argument
 */
int mythread_attr_setschedparam(mythread_attr_t *attr,
		const struct sched_param *param) {
	if (!attr || !param) {
		return -1;
	}
	attr->attr = param->sched_priority;
	return 0;
}

