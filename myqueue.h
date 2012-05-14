/*
 * myqueue.h -- interface for queue ops
 */

#ifndef MYQUEUE_H
#define MYQUEUE_H

#include <malloc.h>
#include <stdio.h>

typedef struct mythread_queue {
  void *item;
  struct mythread_queue *prev, *next;
} *mythread_queue_t;

/* Initialize the queue */
void mythread_q_init(mythread_queue_t *headp, void *item);

/* Test if item in Q, return TRUE if so, FALSE o/w */
int mythread_inq(mythread_queue_t *headp, void *item);

/* Enqueue the new item */
void mythread_enq(mythread_queue_t *headp, void *item);

/* Remove element from the queue */
void mythread_deq(mythread_queue_t *headp, void *item);

/* Dequeue item by priority */
void *mythread_deq_prio(mythread_queue_t *headp);

#endif /* MYQUEUE_H */
