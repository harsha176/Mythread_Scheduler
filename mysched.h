/*
 * mysched.h -- a header for implementation of thread scheduler
 *
 */

#ifndef MYSCHED_H
#define MYSCHED_H

#include <signal.h>
#include "mythread.h"

/*
 * mythread_scheduler
 */
static int mythread_scheduler(void);

/*
 * mythread_sighandler
 */
static void mythread_sighandler(int sig, siginfo_t *siginfo, void *ucp);

/*
 * mythread_leave_kernel - leave the monolithic threading kernel
 */
void mythread_leave_kernel(void);

/*
 * mythread_init_sched - called once at the start of each thread
 * enable virtual timer signals, install signal handler for it and
 * set time slice of the timer (real/virtual)
 */
void mythread_init_sched(void);

/*
 * mythread_exit_sched - called on thread terminiation (exit) by each thread
 * disable all the features activated in mythread_init_sched()
 */
void mythread_exit_sched(void);

/*
 * pthread_attr_init - initialize a thread attributes object attr
 * with the default value for all of the individual attributes used
 * by a given implementation.
 */
int mythread_attr_init(mythread_attr_t *attr);

/*
 * pthread_attr_destroy - destroy a  thread  attributes object
 */
int mythread_attr_destroy(mythread_attr_t *attr);

/*
 * pthread_attr_getschedparam - get the scheduling parameter
 * attributes in the attr argument
 */
int mythread_attr_getschedparam(const mythread_attr_t *attr,
		struct sched_param *param);

/*
 * pthread_attr_setschedparam - set the scheduling parameter
 * attributes in the attr argument
 */
int mythread_attr_setschedparam(mythread_attr_t *attr,
		const struct sched_param *param);

#endif // MYSCHED_H
