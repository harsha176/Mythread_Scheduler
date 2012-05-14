#include <assert.h>
#include <stdlib.h>
#include "myqueue.h"
#include "mythread.h"
#include "mysched.h"

int get_priority_by_item(mythread_t tid);

/* Initialize the queue */
void mythread_q_init(mythread_queue_t *headp, void *item) {
	//headp = (mythread_queue_t*)malloc(sizeof(mythread_queue_t));
	//assert(headp);
	*headp = (mythread_queue_t) malloc(sizeof(struct mythread_queue));
	if (!*headp) {
		perror("Failed to initialize queue");
		return;
	}
	(*headp)->item = item;
	(*headp)->next = NULL;
	(*headp)->prev = NULL;

	return;
}

/* Test if item in Q, return TRUE if so, FALSE o/w */
int mythread_inq(mythread_queue_t *headp, void *item) {
	mythread_queue_t curr_node = *headp;
	while (curr_node != NULL && curr_node->item != item) {
		curr_node = curr_node->next;
	}
	if (curr_node == NULL) {
		return 0;
	}
	return 1;
}

/* Enqueue the new item */
void mythread_enq(mythread_queue_t *headp, void *item) {
	mythread_queue_t curr_node = *headp;
	// if queue is not initialized
	if (curr_node == NULL) {
		mythread_q_init(headp, item);
		return;
	}
	// create new node
	mythread_queue_t new_node = (mythread_queue_t) malloc(
			sizeof(struct mythread_queue));

	if (!new_node) {
		perror("Failed to enqueue item");
		return;
	}

	new_node->item = item;
	new_node->next = NULL;
	new_node->prev = NULL;

	// insert at the end
	while (curr_node != NULL && curr_node->next != NULL) {
		curr_node = curr_node->next;
	}

	curr_node->next = new_node;
	new_node->prev = curr_node;
}

/* Remove elements from the queue */
void mythread_deq(mythread_queue_t *headp, void *item) {
	if (headp == NULL || *headp == NULL) {
		return NULL;
	}

	mythread_queue_t max_node_ptr = *headp;
	mythread_queue_t curr_node = *headp;

	while (curr_node != NULL && curr_node->item != item) {
		curr_node = curr_node->next;
	}

	// delete highest priority node
	if (curr_node != NULL) {
		if (curr_node == *headp) {
			*headp = (*headp)->next;
			if (*headp != NULL) {
				(*headp)->prev = NULL;
			}
			item = curr_node->item;
			free(curr_node);
		} else {
			curr_node->prev->next = curr_node->next;
			if (curr_node->next != NULL) {
				curr_node->next->prev = curr_node->prev;
			}
			item = curr_node->item;
			free(curr_node);
		}
	}
}

int get_priority_by_node(mythread_queue_t node) {
	if (node == NULL || node->item == NULL) {
		perror("Invalid node");
		return -1;
	}
	get_priority_by_item(node->item);
}

int get_priority_by_item(mythread_t tid) {
	//tid = ((mythread_t)(node->item));
	if (tid->attribute == NULL) {
		return DEFAULT_ATTR;
	} else {
		return tid->attribute->attr;
	}
}

/* Dequeue item by priority */
void *mythread_deq_prio(mythread_queue_t *headp) {
	// find the item with highest priority
	if (headp == NULL || *headp == NULL) {
		return NULL;
	}

	mythread_queue_t max_node_ptr = *headp;
	mythread_queue_t curr_node = *headp;

	while (curr_node != NULL) {
		if (get_priority_by_node(max_node_ptr) > get_priority_by_node(curr_node)) {
			max_node_ptr = curr_node;
		}
		curr_node = curr_node->next;
	}
	return max_node_ptr->item;
}
