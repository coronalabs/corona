#include "SimpleThread.h"
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#if defined(DEBUG)
#include <stdio.h>
#define THRDDBG(x) printf x
#else
#define THRDDBG(x)
#endif


struct SimpleThread
{
	size_t threadID;
	pthread_t nativeThread;
	int threadStatus;
//    void* userData;	
};

typedef struct SimpleThreadArguments
{
    int (*userFunction)(void*);
    void* userData;
    SimpleThread* simpleThread;
} SimpleThreadArguments;



static void* Internal_RunThread(void* user_data)
{
	int (*user_function)(void*);
	void* function_user_data;
	int* status_val;

#if 0
	/* disable signals */
	sigset_t disable_set;

	/*
	   in the main thread, set up the desired signal mask, common to most threads
	   any newly created threads will inherit this signal mask
	   */
	sigemptyset(&disable_set);
	sigaddset(&disable_set, SIGHUP);
	sigaddset(&disable_set, SIGINT);
	sigaddset(&disable_set, SIGUSR1);
	sigaddset(&disable_set, SIGUSR2);
	sigaddset(&disable_set, SIGALRM);
	sigaddset(&disable_set, SIGQUIT);
	sigaddset(&disable_set, SIGPIPE);
	sigaddset(&disable_set, SIGTERM);
	sigaddset(&disable_set, SIGCHLD);
	sigaddset(&disable_set, SIGWINCH);
	sigaddset(&disable_set, SIGVTALRM);
	sigaddset(&disable_set, SIGPROF);


	/* block out these signals */
	sigprocmask(SIG_BLOCK, &disable_set, NULL);
#endif

	SimpleThreadArguments* simple_thread_arguments = (SimpleThreadArguments*)user_data;
	simple_thread_arguments->simpleThread->threadID = SimpleThread_GetCurrentThreadID();

	user_function = simple_thread_arguments->userFunction;
	function_user_data = simple_thread_arguments->userData;
	status_val = &simple_thread_arguments->simpleThread->threadStatus;

	
	/* I hope this is safe to delete on a different thread than it was created for. */
	free(simple_thread_arguments);

	*status_val = user_function(function_user_data);

	pthread_exit(NULL);
	return NULL;
}


SimpleThread* SimpleThread_CreateThread(int (*user_function)(void*), void* user_data)
{
	pthread_attr_t thread_attributes;
	int ret_val;
	SimpleThread* new_thread;
	SimpleThreadArguments* simple_thread_arguments;

	new_thread = (SimpleThread*)malloc(sizeof(SimpleThread));
	if(NULL == new_thread)
	{
		THRDDBG(("Out of memory.\n"));				
		return NULL;
	}

	ret_val = pthread_attr_init(&thread_attributes);
	if(0 != ret_val)
	{
		/* failed */
		THRDDBG(("pthread_attr_init failed with: %d\n", ret_val));		
		free(new_thread);
		return 0;
	}
	ret_val = pthread_attr_setdetachstate(&thread_attributes, PTHREAD_CREATE_JOINABLE);
	if(0 != ret_val)
	{
		THRDDBG(("pthread_attr_setdetachstate failed with: %d\n", ret_val));
		free(new_thread);
		return NULL;
	}

	simple_thread_arguments = (SimpleThreadArguments*)malloc(sizeof(SimpleThreadArguments));
	if(NULL == simple_thread_arguments)
	{
		THRDDBG(("Out of memory.\n"));
		free(new_thread);		
		return NULL;
	}
	simple_thread_arguments->userFunction = user_function;
	simple_thread_arguments->userData = user_data;
	simple_thread_arguments->simpleThread = new_thread;

	ret_val = pthread_create(&new_thread->nativeThread, &thread_attributes, Internal_RunThread, simple_thread_arguments);
	if(0 != ret_val)
	{
		THRDDBG(("pthread_create failed with: %d\n", ret_val));
		free(simple_thread_arguments);		
		free(new_thread);		
		return NULL;
	}

#if defined(Rtt_NXS_ENV)
	// set mixer thread priority as max to avoid sound interuuption when main loop is too busy
	const int policy = SCHED_FIFO;
	const int max_priority = sched_get_priority_max(policy);
	struct sched_param param;
	memset(&param, 0, sizeof(struct sched_param));
	param.sched_priority = max_priority;
	pthread_setschedparam(new_thread->nativeThread, policy, &param);
#endif

	return new_thread;
}



size_t SimpleThread_GetCurrentThreadID()
{
#ifdef NXS_LIB
	return (size_t)pthread_self().p;
#else
	return (size_t)pthread_self();
#endif
}

void SimpleThread_WaitThread(SimpleThread* simple_thread, int* thread_status)
{
	int ret_val;	
	if(NULL == simple_thread)
	{
		THRDDBG(("SimpleThread_WaitThread was passed NULL\n"));	
		return;
	}


    ret_val = pthread_join(simple_thread->nativeThread, 0);
	if(0 != ret_val)
	{
		THRDDBG(("pthread_join failed with: %d\n", ret_val));
	}
	if(NULL != thread_status)
	{
		*thread_status = simple_thread->threadStatus;
	}
	free(simple_thread);
}

size_t SimpleThread_GetThreadID(SimpleThread* simple_thread)
{
	if(NULL == simple_thread)
	{
		THRDDBG(("SimpleThread_GetThreadID was passed NULL\n"));	
		return 0;
	}
	return simple_thread->threadID;
}


int SimpleThread_GetThreadPriority(SimpleThread* simple_thread)
{
	struct sched_param schedule_param;
	int sched_policy;
	int ret_val;
	
	if(NULL == simple_thread)
	{
		THRDDBG(("SimpleThread_GetThreadPriority was passed NULL\n"));	
		return -1; /* Not sure what to return. Do other platforms use negative numbers? */
	}
	ret_val = pthread_getschedparam(simple_thread->nativeThread, &sched_policy, &schedule_param);
	if(0 != ret_val)
	{
		THRDDBG(("SimpleThread_GetThreadPriority pthread_getschedparam failed with: %d\n", ret_val));
		return -1;
	}
	return schedule_param.sched_priority;
}

void SimpleThread_SetThreadPriority(SimpleThread* simple_thread, int priority_level)
{
	struct sched_param schedule_param;
	int ret_val;
	
	if(NULL == simple_thread)
	{
		THRDDBG(("SimpleThread_SetThreadPriority was passed NULL\n"));	
		return;
	}
	schedule_param.sched_priority = priority_level; /* PTHREAD_MIN_PRIORITY=0 to PTHREAD_MAX_PRIORITY=31 */
	ret_val = pthread_setschedparam(simple_thread->nativeThread, SCHED_OTHER, &schedule_param);
	if(0 != ret_val)
	{
		THRDDBG(("SimpleThread_SetThreadPriority pthread_setschedparam failed with: %d\n", ret_val));
		return;
	}
}

