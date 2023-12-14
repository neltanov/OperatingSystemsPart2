#define _GNU_SOURCE
#include <pthread.h>
#include <assert.h>

#include "queue.h"

pthread_cond_t queue_is_empty = PTHREAD_COND_INITIALIZER;

pthread_cond_t queue_is_full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *qmonitor(void *arg) {
	queue_t *q = (queue_t *)arg;

	printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

	while (1) {
		queue_print_stats(q);
		sleep(1);
	}

	return NULL;
}

queue_t* queue_init(int max_count) {
	int err;

	if (pthread_cond_init(&queue_is_empty, NULL)) {
		printf("queue_init: pthread_cond_init() failed\n");
		abort();
	}

	if (pthread_cond_init(&queue_is_full, NULL)) {
		printf("queue_init: pthread_cond_init() failed\n");
		abort();
	}

	if (pthread_mutex_init(&mutex, NULL)) {
		printf("queue_init: pthread_mutex_init() failed\n");
		abort();
	}

	queue_t *q = malloc(sizeof(queue_t));
	if (!q) {
		printf("Cannot allocate memory for a queue\n");
		abort();
	}

	q->first = NULL;
	q->last = NULL;
	q->max_count = max_count;
	q->count = 0;

	q->add_attempts = q->get_attempts = 0;
	q->add_count = q->get_count = 0;

	err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
	if (err) {
		printf("queue_init: pthread_create() failed: %s\n", strerror(err));
		abort();
	}

	return q;
}

void queue_destroy(queue_t *q) {
	if (pthread_mutex_lock(&mutex)) {
		printf("queue_destroy: pthread_mutex_lock failed\n");
	}

	if (!q) {
		return;
	}
	int err;

	qnode_t *tmp;
	while (q->first) {
		tmp = q->first;
		q->first = q->first->next;
		free(tmp);
	}

	err = pthread_cancel(q->qmonitor_tid);
	if (err) {
		printf("queue_destroy: pthread_cancel() failed: %s\n", strerror(err));
		abort();
	}

	err = pthread_join(q->qmonitor_tid, NULL);
	if (err) {
		printf("queue_destroy: pthread_join() failed: %s\n", strerror(err));
		abort();
	}

	free(q);

	if (pthread_mutex_unlock(&mutex)) {
		printf("queue_destroy: pthread_mutex_unlock failed\n");
	}
	if (pthread_cond_destroy(&queue_is_empty)) {
		printf("queue_destroy: pthread_cond_destroy() failed\n");
	}
	if (pthread_mutex_destroy(&mutex)) {
		printf("queue_destroy: pthread_mutex_destroy() failed\n");
	}
}

int queue_add(queue_t *q, int val) {
	if (pthread_mutex_lock(&mutex)) {
		printf("queue_add: pthread_mutex_lock failed\n");
	}

	q->add_attempts++;

	assert(q->count <= q->max_count);

	while (q->count == q->max_count) {
		pthread_cond_wait(&queue_is_full, &mutex);
	}

	qnode_t *new = malloc(sizeof(qnode_t));
	if (!new) {
		printf("Cannot allocate memory for new node\n");
		abort();
	}

	new->val = val;
	new->next = NULL;

	if (!q->first)
		q->first = q->last = new;
	else {
		q->last->next = new;
		q->last = q->last->next;
	}

	q->count++;
	q->add_count++;

	if (pthread_cond_signal(&queue_is_empty)) {
		printf("queue_add: pthread_cond_singnal() failed\n");
	}
	if (pthread_mutex_unlock(&mutex)) {
		printf("queue_add: pthread_mutex_unlock failed\n");
	}

	return 1;
}

int queue_get(queue_t *q, int *val) {
	if (pthread_mutex_lock(&mutex)) {
		printf("queue_get: pthread_mutex_lock failed\n");
	}

	q->get_attempts++;

	assert(q->count >= 0);

	while (!(q->count > 0)) {
		pthread_cond_wait(&queue_is_empty, &mutex);
	}

	qnode_t *tmp = q->first;

	*val = tmp->val;
	q->first = q->first->next;

	free(tmp);
	q->count--;
	q->get_count++;

	if (pthread_cond_signal(&queue_is_full)) {
		printf("queue_get: pthread_cond_singnal() failed\n");
	}

	if (pthread_mutex_unlock(&mutex)) {
		printf("queue_get: pthread_mutex_unlock failed\n");
	}

	return 1;
}

void queue_print_stats(queue_t *q) {
	printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
		q->count,
		q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
		q->add_count, q->get_count, q->add_count -q->get_count);
}

