/* ----------------------------------------------------------------------------
// 
// pthread.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------
*/

#pragma once


#ifdef __cplusplus
	#define PTHREADS_EXTERN_C extern "C"
#else
	#define PTHREADS_EXTERN_C
#endif

#ifdef PTHREADS_EXPORT
	#define PTHREADS_API PTHREADS_EXTERN_C __declspec( dllexport )
#else
	#define PTHREADS_API PTHREADS_EXTERN_C __declspec( dllimport )
#endif


enum
{
	PTHREAD_CREATE_JOINABLE = 0,
	PTHREAD_CREATE_DETACHED = 1
};

enum
{
	PTHREAD_MUTEX_NORMAL = 0,
	PTHREAD_MUTEX_RECURSIVE = 1,
	PTHREAD_MUTEX_ERROR_CHECK = 2,
	PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
};

enum
{
	SCHED_OTHER = 0,
	SCHED_FIFO = 1,
	SCHED_RR = 2
};

#define PTHREAD_ONCE_INIT 0;

PTHREADS_EXTERN_C struct sched_param
{
	int sched_priority;
};

PTHREADS_EXTERN_C typedef void* pthread_t;
PTHREADS_EXTERN_C typedef void* pthread_attr_t;
PTHREADS_EXTERN_C typedef int pthread_key_t;
PTHREADS_EXTERN_C typedef void* pthread_mutex_t;
PTHREADS_EXTERN_C typedef int pthread_mutexattr_t;
PTHREADS_EXTERN_C typedef int pthread_once_t;


PTHREADS_API int pthread_attr_init(pthread_attr_t *attr);
PTHREADS_API int pthread_attr_destroy(pthread_attr_t *attr);
PTHREADS_API int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
PTHREADS_API int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
PTHREADS_API int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);
PTHREADS_API void pthread_exit(void *value_ptr);
PTHREADS_API int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param);
PTHREADS_API void* pthread_getspecific(pthread_key_t key);
PTHREADS_API int pthread_join(pthread_t thread, void **value_ptr);
PTHREADS_API int pthread_key_create(pthread_key_t *key, void(*destructor)(void*));
PTHREADS_API int pthread_key_delete(pthread_key_t key);
PTHREADS_API int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
PTHREADS_API int pthread_mutex_destroy(pthread_mutex_t *mutex);
PTHREADS_API int pthread_mutex_lock(pthread_mutex_t *mutex);
PTHREADS_API int pthread_mutex_unlock(pthread_mutex_t *mutex);
PTHREADS_API int pthread_mutexattr_init(pthread_mutexattr_t *attr);
PTHREADS_API int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
PTHREADS_API int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
PTHREADS_API int pthread_once(pthread_once_t *once_control, void(*init_routine)(void));
PTHREADS_API pthread_t pthread_self(void);
PTHREADS_API int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);
PTHREADS_API int pthread_setspecific(pthread_key_t key, const void *value);
PTHREADS_API void pthread_sleep(int milliseconds);
PTHREADS_API void pthread_yield(void);

