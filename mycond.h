/*
 * mycond.h -- interface of condition variables
 */

typedef struct mythread_condattr {
  int attr;                           /* not yet used */
} mythread_condattr_t;

typedef struct mythread_cond {
  mythread_queue_t q;
} mythread_cond_t;
