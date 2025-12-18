#include "cthread.h"
#include <stdlib.h>
#include <pthread.h>

int thread_start(thread_t *thread) {

  if (thread->pool && !thread_pool_append(thread->pool, thread))
    return 0;

  return pthread_create(&thread->thread, &thread->attr, thread->fn, thread->arg);
}

int thread_start_attr(thread_t *thread, thread_attr_t attr) {

  if (pthread_attr_init(&attr.attr) != 0)
    return 0;

  if (attr.cpuset.cpusetsize > 0 && pthread_attr_setaffinity_np(&attr.attr, attr.cpuset.cpusetsize, &attr.cpuset.cpuset) != 0)
    return 0;

  if (attr.detachstate != PTHREAD_CREATE_JOINABLE && pthread_attr_setdetachstate(&attr.attr, attr.detachstate) != 0) 
    return 0;

  if (attr.guardsize > 0 && pthread_attr_setguardsize(&attr.attr, attr.guardsize) != 0)
    return 0;

  if (attr.inherit_sched != PTHREAD_INHERIT_SCHED && pthread_attr_setinheritsched(&attr.attr, attr.inherit_sched) != 0)
    return 0;

  if (attr.sched_param.sched_priority > 0 && 0)
    return 0;

  if (attr.sched_policy > 0 && pthread_attr_setschedpolicy(&attr.attr, attr.sched_policy) != 0)
    return 0;

  if (attr.scope > 0 && pthread_attr_setscope(&attr.attr, attr.scope) != 0)
    return 0;

  if (attr.stacksize > 0) {
    if ((!attr.stackaddr && pthread_attr_setstacksize(&attr.attr, attr.stacksize) != 0) || (attr.stackaddr && pthread_attr_setstack(&attr.attr, attr.stackaddr, attr.stacksize) != 0))
      return 0;
  }

  int t = pthread_create(&thread->thread, &attr.attr, thread->fn, thread->arg);

  pthread_attr_destroy(&attr.attr);

  return t;
}

int thread_cancel(thread_t *thread) {
  return pthread_cancel(thread->thread);
}

thread_pool_t *thread_pool_create(unsigned int size) {
  thread_pool_t *pool = malloc(sizeof(thread_pool_t)); 

  if (!pool)
    return (void*)0;

  pool->size = 0;
  thread_pool_resize(pool, size);

  if (!pool->ptr)
    return (void*)0;

  return pool;
}

int thread_pool_append(thread_pool_t *pool, thread_t *thread) {
  if (pool->size >= pool->capacity && !thread_pool_resize(pool, pool->capacity * 2))
    return 0;

  pool->ptr[pool->size++] = thread;

  return 1;
}

int thread_pool_destroy(thread_pool_t *pool) {
  if (!pool)
    return 0;

  for (int i = 0; i < pool->size; i++) {
    free(&pool->ptr[i]);
  }

  free(pool);

  return 1;
}

int thread_pool_resize(thread_pool_t *pool, int capacity) {
  if (!pool)
    return 0;

  pool->ptr = realloc(pool->ptr, sizeof(thread_t*) * capacity);
  pool->capacity = capacity;

  return 1;
}
