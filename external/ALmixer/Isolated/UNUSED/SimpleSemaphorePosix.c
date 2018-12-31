#include "SimpleSemaphore.h"
#include <semaphore.h>
#include <stdlib.h>

#if defined(DEBUG)
#include <stdio.h>
#include <string.h>
#include <errno.h>
#define SEMDBG(x) printf x
#else
#define SEMDBG(x)
#endif



struct SimpleSemaphore
{
    sem_t theSemaphore;
};

SimpleSemaphore* SimpleSemaphore_CreateSemaphore(int initial_value)
{
	int ret_val;	
    SimpleSemaphore* simple_semaphore = (SimpleSemaphore*)malloc(sizeof(SimpleSemaphore));
	if(NULL == simple_semaphore)
	{
		SEMDBG(("Out of memory.\n"));						
		return NULL;
	}
	/* Drat: sem_init isn't available on OS X */
#ifdef __APPLE__
	#warning "sem_init (unnamed semaphores) do not work on OS X. This code is broken."
#endif
	ret_val = sem_init(&simple_semaphore->theSemaphore, 0, initial_value);
	if(0 != ret_val)
	{
		/* failed */
		SEMDBG(("sem_init failed with: %d\n", ret_val));		
		free(simple_semaphore);
		return 0;
	}

    return simple_semaphore;
}

void SimpleSemaphore_DestroySemaphore(SimpleSemaphore* simple_semaphore)
{
	if(NULL == simple_semaphore)
	{
		return;
	}
	sem_destroy(&simple_semaphore->theSemaphore);
	free(simple_semaphore);
}

int SimpleSemaphore_SemaphoreTryWait(SimpleSemaphore* simple_semaphore)
{
    int ret_val;
	if(NULL == simple_semaphore)
	{
		SEMDBG(("SimpleSemaphore_SemTryWait was passed a NULL semaphore\n"));	
		return 0;
	}
    ret_val = sem_trywait(&simple_semaphore->theSemaphore);
	if(0 == ret_val)
	{
		return 1;
    }
	else
	{
		return 0;
	}
}

int SimpleSemaphore_SemaphoreWait(SimpleSemaphore* simple_semaphore)
{
    int ret_val;
	if(NULL == simple_semaphore)
	{
		SEMDBG(("SimpleSemaphore_SemaphoreWait was passed a NULL semaphore\n"));	
		return 0;
	}
    ret_val = sem_wait(&simple_semaphore->theSemaphore);
	if(0 == ret_val)
	{
		return 1;
    }
	else
	{
		return 0;
	}
}

int SimpleSemaphore_SemaphoreGetValue(SimpleSemaphore* simple_semaphore)
{
    int ret_val = 0;
	if(NULL == simple_semaphore)
	{
		SEMDBG(("SimpleSemaphore_SemaphoreGetValue was passed a NULL semaphore\n"));	
		return 0;
	}
	sem_getvalue(&simple_semaphore->theSemaphore, &ret_val);
	return ret_val;
}

int SimpleSemaphore_SemaphorePost(SimpleSemaphore* simple_semaphore)
{
    int ret_val;
	if(NULL == simple_semaphore)
	{
		SEMDBG(("SimpleSemaphore_SemaphorePost was passed a NULL semaphore\n"));	
		return 0;
	}

    ret_val = sem_post(&simple_semaphore->theSemaphore);
 	if(-1 == ret_val)
	{
		SEMDBG(("sem_post failed with: %s\n", strerror(errno)));		
	}
    return ret_val;
}

