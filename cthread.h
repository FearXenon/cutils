#ifndef CTHREAD
#define CTHREAD

#include <pthread.h>

typedef void* (*thread_fn_t)(void*);

typedef struct {
  pthread_t thread;
  pthread_attr_t attr;
  thread_fn_t fn;
  void *arg;
  void* pool;
} thread_t;

typedef struct {
  pthread_attr_t attr;
  struct {
    size_t cpusetsize;
    cpu_set_t cpuset;
  } cpuset;
  int detachstate;
  size_t guardsize;
  int inherit_sched;
  struct sched_param sched_param;
  int sched_policy;
  int scope;
  void *stackaddr;
  size_t stacksize;
} thread_attr_t;

typedef struct {
  thread_t** ptr;
  unsigned int size;
  unsigned int capacity;
} thread_pool_t;

int thread_start(thread_t *thread);
int thread_start_attr(thread_t *thread, thread_attr_t attr);
int thread_cancel(thread_t *thread);

thread_pool_t *thread_pool_create(unsigned int size);
int thread_pool_append(thread_pool_t *pool, thread_t *thread);
int thread_pool_destroy(thread_pool_t *pool);

int thread_pool_resize(thread_pool_t *pool, int size);

#endif
