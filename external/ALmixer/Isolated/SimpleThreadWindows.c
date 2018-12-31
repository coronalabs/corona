/* Copyright PlayControl Software LLC / Eric Wing.
 */
#include "SimpleThread.h"

#define WIN32_LEAN_AND_MEAN
#include <process.h>
#include <windows.h>
#include <stdlib.h>

#if defined(DEBUG)
#include <stdio.h>
#define THRDDBG(x) printf x
#else
#define THRDDBG(x)
#endif


struct SimpleThread
{
	unsigned threadID;
	HANDLE nativeThread;
	unsigned threadStatus;
};

typedef struct SimpleThreadArguments
{
    int (*userFunction)(void*);
    void* userData;
    SimpleThread* simpleThread;
} SimpleThreadArguments;



static unsigned __stdcall Internal_RunThread(void* user_data)
{
	int (*user_function)(void*);
	void* function_user_data;
	unsigned* status_val;

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
	/* It looks like the last parameter of _beginthreadex sets the threadID for me */
/*	simple_thread_arguments->simpleThread->threadID = SimpleThread_GetCurrentThreadID(); */

	user_function = simple_thread_arguments->userFunction;
	function_user_data = simple_thread_arguments->userData;
	status_val = &simple_thread_arguments->simpleThread->threadStatus;


	/* I hope this is safe to delete on a different thread than it was created for. */
	free(simple_thread_arguments);

	*status_val = user_function(function_user_data);

    _endthreadex( 0 );

	return 0;
}


SimpleThread* SimpleThread_CreateThread(int (*user_function)(void*), void* user_data)
{
	SimpleThread* new_thread;
	SimpleThreadArguments* simple_thread_arguments;

	new_thread = (SimpleThread*)malloc(sizeof(SimpleThread));
	if(NULL == new_thread)
	{
		THRDDBG(("Out of memory.\n"));
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


	new_thread->nativeThread = (HANDLE)_beginthreadex(NULL, 0, &Internal_RunThread, simple_thread_arguments, 0, &new_thread->threadID);
	if(0 == new_thread->nativeThread)
	{
		THRDDBG(("_beginthreadex failed with: %d\n", errno));
		free(simple_thread_arguments);
		free(new_thread);
		return NULL;
	}

	return new_thread;
}



size_t SimpleThread_GetCurrentThreadID()
{
    return (size_t)GetCurrentThreadId();
}

void SimpleThread_WaitThread(SimpleThread* simple_thread, int* thread_status)
{
	if(NULL == simple_thread)
	{
		THRDDBG(("SimpleThread_WaitThread was passed NULL\n"));
		return;
	}

    WaitForSingleObject(simple_thread->nativeThread, INFINITE);
    CloseHandle(simple_thread->nativeThread);

	if(NULL != thread_status)
	{
		*thread_status = (int)simple_thread->threadStatus;
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
	return (size_t)simple_thread->threadID;
}

/* TODO: Figure out portable/normalized range for levels */
int SimpleThread_GetThreadPriority(SimpleThread* simple_thread)
{
	int ret_val = 0;

	if(NULL == simple_thread)
	{
		THRDDBG(("SimpleThread_GetThreadPriority was passed NULL\n"));
		return THREAD_PRIORITY_ERROR_RETURN; /* Windows ranges seem to go from -15 to +15 */
	}
	ret_val = GetThreadPriority(simple_thread->nativeThread);
	if(THREAD_PRIORITY_ERROR_RETURN == ret_val)
	{
		THRDDBG(("SimpleThread_GetThreadPriority GetThreadPriority failed with: %d\n", ret_val));
		return THREAD_PRIORITY_ERROR_RETURN;
	}
	return ret_val;
}

/* TODO: Figure out portable/normalized range for levels */
void SimpleThread_SetThreadPriority(SimpleThread* simple_thread, int priority_level)
{
	BOOL ret_val;

	if(NULL == simple_thread)
	{
		THRDDBG(("SimpleThread_SetThreadPriority was passed NULL\n"));
		return;
	}

	ret_val = SetThreadPriority(simple_thread->nativeThread, priority_level);
	if(0 == ret_val)
	{
		THRDDBG(("SimpleThread_SetThreadPriority SetThreadPriority failed"));
		return;
	}
}

