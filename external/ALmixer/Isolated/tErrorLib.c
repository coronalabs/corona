/* See header file for license information. */

#include "tErrorLib.h"
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <stdarg.h> /* for vasprintf */
#include <stdio.h> /* also for vasprintf and for printf family */
#include <stddef.h> /* size_t */

/**
 * For string-only based usage, this implementation
 * still expects an actual error number to be set.
 * I am defining 1 as that error value. This might be changable,
 * but it is untested. If you change this value, you must recompile
 * the entire library. This can really be any integer except what 
 * TERROR_NOERROR_VALUE (in header) is set to.
 */
#define TERROR_ERROR_VALUE 1

#if defined(_WIN32) || defined(WINAPI_FAMILY)
	#ifndef DONT_USE_VASPRINTF
		#define DONT_USE_VASPRINTF
	#endif
#endif

#ifdef DONT_USE_VASPRINTF
	#define TERROR_DEFAULT_STRING_LENGTH 128
	/* Visual Studio doesn't define snprintf but _snprintf */
	#ifdef _MSC_VER
		#define snprintf _snprintf
		#define vsnprintf _vsnprintf
	#endif
#endif


#if defined(_WIN32) && !defined(__CYGWIN32__)
		#include <windows.h>
		#include <winbase.h> /* For CreateMutex(), LockFile() */

		static void* Internal_CreateMutex()
		{
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
			return (void*)CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE);
#else
			return (void*)CreateMutex(NULL, FALSE, NULL);
#endif
		}
		static void Internal_DestroyMutex(void* mutex)
		{
			if(NULL != mutex)
			{
				CloseHandle( (HANDLE)mutex );
			}
		}
		/* This will return true if locking is successful, false if not.
		 */
		static int Internal_LockMutex(void* mutex)
		{
			return(
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
				WaitForSingleObjectEx((HANDLE)mutex, INFINITE, FALSE) != WAIT_FAILED
#else
				WaitForSingleObject((HANDLE)mutex, INFINITE) != WAIT_FAILED
#endif
			);
		}
		static void Internal_UnlockMutex(void* mutex)
		{
			ReleaseMutex(
				(HANDLE)mutex
			);
		}
		size_t Internal_PlatformGetThreadID(void)
		{
			return((size_t)GetCurrentThreadId());
		}
#else /* Assuming POSIX...maybe not a good assumption. */
		#include <pthread.h>
		static void* Internal_CreateMutex()
		{
			int ret_val;
			pthread_mutex_t* m = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)); 
			if(NULL == m)
			{
				return NULL;
			}
			ret_val = pthread_mutex_init(m, NULL);
			if(0 != ret_val)
			{
				free(m);
				return NULL;
			}
			return((void*)m);
		}
		static void Internal_DestroyMutex(void* mutex)
		{
			if(NULL != mutex)
			{
				pthread_mutex_destroy((pthread_mutex_t*) (mutex));
				free(mutex);
			}
		}
		/* This will return true if locking is successful, false if not.
		 * (This is the opposite of pthread_mutex_lock which returns 
		 * 0 for success.)
		 */
		static int Internal_LockMutex(void* mutex)
		{
			return(
				pthread_mutex_lock(
					(pthread_mutex_t*)mutex
				) == 0
			);
		}
		static void Internal_UnlockMutex(void* mutex)
		{
			pthread_mutex_unlock(
				(pthread_mutex_t*)mutex
			);
		}

	size_t Internal_PlatformGetThreadID()
	{
		/* Basically, we need to convert a pthread_t into an id number. */
	    return (size_t)pthread_self();
	}
#endif


/**
 * Copies a source string, potentially to a target string, and returns 
 * the pointer to the copied string.
 * This function is a intended to be an efficient string copy function.
 * It's purpose is to copy a string into a string with preallocated memory
 * and avoid dynamic memory allocation if possible. If memory must 
 * be allocated, then the old string will be destroyed.
 *
 * This is only to be used where target_string was created with dynamic 
 * memory. This function will destroy the memory and allocate new memory
 * if there is not enough space in the target string.
 *
 * @param target_string This is the string you would like to try 
 * to copy into. If there is not enough space, a new string will
 * be created and the target_string will be freed. This string 
 * must have been created dynamically. This may be NULL if you 
 * wish for this function to dynamically create a new string 
 * for you.
 *
 * @param target_max_buffer_size This is a pointer that points to 
 * an address containing the size of the preallocated target_string. 
 * This size is the maximum buffer length which includes the '\\0'
 * character as part of that count. This pointer may not be NULL.
 * If you pass in NULL for the target_string (indicating you want 
 * a new string allocated for you), then the size should be set to 0.
 * When the function completes, the size will be set to the new 
 * max buffer size of the string if the string needed to be reallocated.
 *
 * @param source_string This is the string you want to copy. If it's NULL,
 * the target_string will have it's memory freed.
 *
 * @return Will return a pointer to the duplicated string. Be aware 
 * of several things:
 * - The returned pointer address may not be the same address as the 
 * target string passed in (due to a possible reallocation).
 * - If the pointer to the source and target string 
 * are the same, the pointer to the target string will be returned.
 * - If the source string is NULL, the target string
 * will be freed and will return NULL.
 * - If an error occurs, NULL will be returned.
 *
 * Also note that the value at the address target_max_buffer_size points 
 * to will be filled with the new max buffer size for the string.
 *
 * Example:
 * @code
 *
 * int main()
 * {
 * 		const char* original1 = "Hello World";
 * 		const char* original2 = "Smaller";
 * 		const char* original3 = "Good-Bye World";
 * 		char* ret_val;
 * 		char* target = NULL;
 * 		size_t target_max_buffer_size = 0;
 *
 * 		ret_val = CopyDynamicString(target, &target_max_buffer_size, original1);
 *
 * 		if(ret_val)
 * 		{
 * 			fprintf(stderr, "Target is '%s' with max size = %d\n", ret_val, target_max_buffer_size);
 * 		}
 * 		else
 * 		{
 * 			fprintf(stderr, "Error in function\n");
 * 		}
 *		target = ret_val;
 *
 *		ret_val = CopyDynamicString(target, &target_max_buffer_size, original2);
 * 		fprintf(stderr, "Target is '%s' with max size = %d\n", ret_val, target_max_buffer_size);
 *
 * 		target = ret_val; *
 * 		ret_val = CopyDynamicString(target, &target_max_buffer_size, original3);
 * 		fprintf(stderr, "Target is '%s' with max size = %d\n", ret_val, target_max_buffer_size);
 *
 * 		return 0;
 * }
 * @endcode
 * This outputs:
 * @code
 * Target is 'Hello World' with max size = 12
 * Target is 'Smaller' with max size = 12
 * Target is 'Good-Bye World' with max size = 15
 * @endcode
 */
static char* Internal_CopyDynamicString(char* target_string, size_t* target_max_buffer_size, const char* source_string)
{
	/* If the pointers are the same, no copy is needed. */
	if(source_string == target_string)
	{
		/* I don't feel like asserting if the sizes are the same. */
		/* Return 1 instead of 0 because maybe this isn't an error?
		 */
		return target_string;
	}

	/* Make sure the size pointer is valid. */
	if(NULL == target_max_buffer_size)
	{
		return NULL;
	}

	/* Yikes, if the string is NULL, should we make the target string NULL?
	 * For now, yes, we destroy the string. If you change this, realize that
	 * their is code that depends on this behavior.
	 */
	if(NULL == source_string)
	{
		*target_max_buffer_size = 0;
		free(target_string);
		target_string = NULL;
		return NULL;
	}

	/* If target_string is NULL, the *target_max_buffer_size should also be 0.
	 * Technically, the user should set this and this would be an error,
	 * but I'll be nice for now. An alternate implementation might suggest 
	 * that the size would be the desired size the user wants for a new string.
	 */
	if( (NULL == target_string) && (0 != *target_max_buffer_size) )
	{
		*target_max_buffer_size = 0;
	}

	/* If there is not enough preallocated memory in the target string,
	 * then we need to reallocate enough memory.
	 */
	if( *target_max_buffer_size < (strlen(source_string) + 1) )
	{
		*target_max_buffer_size = 0;
		if(NULL != target_string)
		{
			free(target_string);
		}
		target_string = (char*)calloc( (strlen(source_string) + 1), sizeof(char) );
		if(NULL == target_string)
		{
			return NULL;
		}
		*target_max_buffer_size = strlen(source_string) + 1;
	}

	/* At this point, there should be enough preallocated 
	 * memory to call strncpy.
	 */
	strncpy(target_string, source_string, *target_max_buffer_size);

	return target_string;
}

/**
 * This is a structure that contains everything needed for an 
 * error message entry (per thread). The linked list stuff
 * is fused in with it because I didn't want to write an entire 
 * linked list class.
 */
typedef struct TErrorMessageStructType
{
	size_t threadID; /** ThreadID for associated message. */
	int errorAvailable; /** 1 if an error has been set and not been checked. */
	int errorNumber; /**< For the user error number. */
	char* errorString; /**< For the user error message. */
	size_t errorMaxStringLength; /**< Max size of string buffer including \\0. */
	struct TErrorMessageStructType* nextItem; /**< Pointer to next error message in list. */
} TErrorMessage;

/**
 * This is a private struct that contains all private data for an
 * ErrorPool. Currently it is a linked list containing all error message
 * structs for every thread.
 */
typedef struct
{
	TErrorMessage* errorMessageListHead; /**< Head of the error list. */
	TErrorMessage* lastErrorMessage; /**< Points to the last set element in the list for GetLastError. */
	/* Mutex */
} TErrorPoolOpaqueData;

/**
 * This is a private helper function that creates a new TErrorMessage
 * and initializes all its values.
 * @return Returns a pointer to a newly allocated and initialized
 * TErrorMessage or NULL on failure.
 */
static TErrorMessage* Internal_CreateErrorMessageStructure()
{
	TErrorMessage* new_message;
	/* Use calloc to create a fully cleared structure,
	 * so I don't have to set/clear each member.
	 */
	new_message = (TErrorMessage*)calloc(1, sizeof(TErrorMessage));
	if(NULL == new_message)
	{
		/* Very bad, but not sure what to do. */
		return NULL;
	}
	new_message->errorNumber = TERROR_NOERROR_VALUE;
	return new_message;
}

/**
 * This is a private helper function that frees a TErrorMessage.
 *
 * @param err_mesg The pointer to the TErrorMessage to be freed.
 */
static void Internal_FreeErrorMessageStructure(TErrorMessage* err_mesg)
{
	if(NULL == err_mesg)
	{
		return;
	}
	if(NULL != err_mesg->errorString)
	{
		free(err_mesg->errorString);
		err_mesg->errorString = NULL;
	}
	err_mesg->nextItem = NULL;
	free(err_mesg);
}

/**
 * This is a private helper function that will search the error pool 
 * for the last set error message structure in the Linked list.
 * If the last error message was on a different thread, the error
 * data will be copied to the current thread's memory and the 
 * lastErrorMessage pointer will be set to the current thread's message.
 * (This is because I expect this message to be marked as cleared/read.)
 * This function does its own mutex locking.
 *
 * @param err_pool The error pool to be used.
 * @return Returns the a pointer to the TErrorMessage if found,
 * NULL if not found.
 */
static TErrorMessage* Internal_GetLastError(TErrorPool* err_pool)
{
	size_t thread_id;
	TErrorMessage* current_thread_err_mesg;
	TErrorPoolOpaqueData* err_pool_data;

	thread_id = Internal_PlatformGetThreadID();
	
	Internal_LockMutex(err_pool->mutexLock);
	
	err_pool_data = err_pool->opaqueData;

	if(NULL == err_pool_data->errorMessageListHead)
	{
		Internal_UnlockMutex(err_pool->mutexLock);
		return NULL;
	}

	/* I think this is actually an assertion failure.
	 * I do the check here so I don't have to keep checking below.
	 */
	if(NULL == err_pool_data->lastErrorMessage)
	{
		Internal_UnlockMutex(err_pool->mutexLock);
		return NULL;
	}

	/* We need to determine if the lastMessage pointer is pointing 
	 * to data on the current thread. If it is we can just return it.
	 * Otherwise, we need to copy the message to the current thread's
	 * error message memory area.
	 * We should also update the lastMessage pointer to point
	 * to this message since it will likely be marked cleared once read.
	 */
	if(thread_id == err_pool_data->lastErrorMessage->threadID)
	{
		/* Not copy is needed. The last error message already
		 * points to the memory on the current thread.
		 * We can short-circuit and return.
		 */
		Internal_UnlockMutex(err_pool->mutexLock);
		return err_pool_data->lastErrorMessage;
	}

	/* Sigh, I really should have a dedicated linked list structure,
	 * but I don't feel like writing it right now.
	 */
	for(current_thread_err_mesg = err_pool_data->errorMessageListHead; current_thread_err_mesg != NULL; current_thread_err_mesg = current_thread_err_mesg->nextItem)
	{
		/* First find the message (memory) for the current thread. */
		if(thread_id == current_thread_err_mesg->threadID)
		{
			/* Now we need to copy the message data from the lastErrorMessage
			 * to this thread's message (memory).
			 */
			current_thread_err_mesg->errorNumber = err_pool_data->lastErrorMessage->errorNumber;
			current_thread_err_mesg->errorAvailable = err_pool_data->lastErrorMessage->errorAvailable;
			/* This will copy the string and set the new errorMaxStringLength as needed. */
			current_thread_err_mesg->errorString = Internal_CopyDynamicString(current_thread_err_mesg->errorString, &current_thread_err_mesg->errorMaxStringLength, err_pool_data->lastErrorMessage->errorString);


			/* Finally, change the last error message to point to 
			 * the current thread since I expect the message to be
			 * marked cleared and we don't want to accidentally refetched
			 * the stale, uncleared entry.
			 */
			err_pool_data->lastErrorMessage = current_thread_err_mesg;

			Internal_UnlockMutex(err_pool->mutexLock);
			return current_thread_err_mesg;
		}
	}
	Internal_UnlockMutex(err_pool->mutexLock);
	return NULL;
}

/**
 * This is a private helper function that will search the error pool 
 * for an error message structure in the Linked list (by thread ID)
 * and return the pointer if found. This function does its own mutex
 * locking.
 * @param err_pool The error pool to be used.
 * @return Returns the a pointer to the TErrorMessage if found,
 * NULL if not found.
 */
static TErrorMessage* Internal_GetErrorOnCurrentThread(TErrorPool* err_pool)
{
	size_t thread_id;
	TErrorMessage* current_err_mesg;
	TErrorPoolOpaqueData* err_pool_data;

	thread_id = Internal_PlatformGetThreadID();
	
	Internal_LockMutex(err_pool->mutexLock);
	
	err_pool_data = err_pool->opaqueData;

	if(NULL == err_pool_data->errorMessageListHead)
	{
		Internal_UnlockMutex(err_pool->mutexLock);
		return NULL;
	}
	
	/* Sigh, I really should have a dedicated linked list structure,
	 * but I don't feel like writing it right now.
	 */
	for(current_err_mesg = err_pool_data->errorMessageListHead; current_err_mesg != NULL; current_err_mesg = current_err_mesg->nextItem)
	{
		if(thread_id == current_err_mesg->threadID)
		{
			Internal_UnlockMutex(err_pool->mutexLock);
			return current_err_mesg;
		}
	}
	Internal_UnlockMutex(err_pool->mutexLock);
	return NULL;
}

/**
 * Given a specific TErrorMessage*, will set the lastErrorMessage pointer to
 * the provided error message.
 * This function locks.
 *
 * @param err_pool The error pool to be used.
 * @param error_message The error message to set the lastErrorMessage pointer to
 */
static void Internal_SetLastErrorMessagePointerToErrorMessage(TErrorPool* err_pool, TErrorMessage* error_message)
{
	TErrorPoolOpaqueData* err_pool_data;	
	Internal_LockMutex(err_pool->mutexLock);
	err_pool_data = err_pool->opaqueData;
	err_pool_data->lastErrorMessage = error_message;
	Internal_UnlockMutex(err_pool->mutexLock);
}


/**
 * This is a private helper function that creates a new error message
 * structure for the current thread.
 * This currently does not check if an error already exists
 * before creating a new entry. Call GetErrorOnCurrentThread first
 * to make sure nothing exists or duplicate entries will be created.
 * This function does its own mutex locking.
 * 
 * @param err_pool The error pool to be used.
 * @return Returns the a pointer to the TErrorMessage if found,
 * NULL if there was an allocation error.
 */
static TErrorMessage* Internal_CreateErrorOnCurrentThread(TErrorPool* err_pool)
{
	TErrorMessage* new_err_mesg;
	TErrorPoolOpaqueData* err_pool_data;

	new_err_mesg = Internal_CreateErrorMessageStructure();
	if(NULL == new_err_mesg)
	{
		/* Serious problem, not sure what to do. */
		return NULL;
	}
	/* Copy the thread id so we can distinguish between entries. */
	new_err_mesg->threadID = Internal_PlatformGetThreadID();

	Internal_LockMutex(err_pool->mutexLock);

	err_pool_data = err_pool->opaqueData;
	/* Add the new message to the top of the list by making 
	 * its next pointer point to the head of the current list.
	 * (A formal linked list implementation would make this feel 
	 * less hacky.)
	 * This also (should) handle the case where errorMessageListHead
	 * is NULL.
	 */
	new_err_mesg->nextItem = err_pool_data->errorMessageListHead;
	/* Now set the head of the list to the new message.
	 */
	err_pool_data->errorMessageListHead = new_err_mesg;
	
	Internal_UnlockMutex(err_pool->mutexLock);

	return new_err_mesg;
}

/**
 * This is a private helper function that will clean up all the 
 * error message structures in the list. This function does its 
 * own locking.
 * @param err_pool The error pool to be used.
 */
static void Internal_FreeErrorMessageList(TErrorPool* err_pool)
{
	TErrorMessage* current_message = NULL;
	TErrorMessage* next_message = NULL;
	TErrorPoolOpaqueData* err_pool_data;

	Internal_LockMutex(err_pool->mutexLock);
	
	err_pool_data = err_pool->opaqueData;

	if(NULL == err_pool_data->errorMessageListHead)
	{
		Internal_UnlockMutex(err_pool->mutexLock);
		return;
	}
	
	/* Sigh, I really should have a dedicated linked list structure,
	 * but I don't feel like writing it right now.
	 */
	for(current_message = err_pool_data->errorMessageListHead;
		current_message != NULL;
		current_message = next_message
	)
	{
		next_message = current_message->nextItem;
		Internal_FreeErrorMessageStructure(current_message);
	}
	err_pool_data->errorMessageListHead = NULL;
	err_pool_data->lastErrorMessage = NULL;

	Internal_UnlockMutex(err_pool->mutexLock);
}

/* 
 * API functions start below.
 *
 */


void TError_DeleteEntryOnCurrentThread(TErrorPool* err_pool)
{
	TErrorMessage* prev_message = NULL;
	TErrorMessage* current_message = NULL;
	TErrorMessage* next_message = NULL;
	size_t thread_id;
	TErrorPoolOpaqueData* err_pool_data;

	thread_id = Internal_PlatformGetThreadID();
	
	Internal_LockMutex(err_pool->mutexLock);

	err_pool_data = err_pool->opaqueData;

	if(NULL == err_pool_data->errorMessageListHead)
	{
		Internal_UnlockMutex(err_pool->mutexLock);
		return;
	}
	
	/* Sigh, I really should have a dedicated linked list structure,
	 * but I don't feel like writing it right now.
	 */
	for(current_message = err_pool_data->errorMessageListHead;
		current_message != NULL;
		/* I'm not going to increment here because I
		 * may delete the item below which would probably 
		 * cause bad things to happen here.
		 */
/* 		current_message = current_message->nextItem */
	)
	{
		next_message = current_message->nextItem;
		
		if(thread_id == current_message->threadID)
		{
			/* Special case, current is only item in list:
			 * Both next and prev are NULL in this case.
			 * We should delete the item and set the errorMessageListHead
			 * to NULL.
			 */
			if((NULL == prev_message) && (NULL == next_message))
			{
				Internal_FreeErrorMessageStructure(current_message);
				current_message = NULL;
				err_pool_data->errorMessageListHead = NULL;
				err_pool_data->lastErrorMessage = NULL;
			}
			/* Special case, current is at head:
			 * Prev is NULL but next is not NULL in this case.
			 * We should delete the item and set the errorMessageListHead
			 * to point to next.
			 * (The code for the above case would probably work for 
			 * this case too, but for clarity, this remains.)
			 */
			else if(NULL == prev_message)
			{
				/* If the current message happened to be the last message
				 * set, we need to change the lastErrorMessage pointer
				 * so it is not dangling.
				 */
				if(current_message == err_pool_data->lastErrorMessage)
				{
					err_pool_data->lastErrorMessage = NULL;
				}
				Internal_FreeErrorMessageStructure(current_message);
				current_message = NULL;
				err_pool_data->errorMessageListHead = next_message;
			}
			/* Special case, current is at tail.
			 * Prev is not NULL, but next is NULL in this case.
			 * We should delete the item and set prev->next to NULL.
			 */
			else if(NULL == next_message)
			{
				/* If the current message happened to be the last message
				 * set, we need to change the lastErrorMessage pointer
				 * so it is not dangling.
				 */
				if(current_message == err_pool_data->lastErrorMessage)
				{
					err_pool_data->lastErrorMessage = NULL;
				}
				Internal_FreeErrorMessageStructure(current_message);
				current_message = NULL;
				prev_message->nextItem = NULL;
			}
			/* Normal case, current is somewhere in the middle of the list.
			 * The item should be deleted and
			 * the prev_message->next should connect to 
			 * the next_message.
			 */
			else
			{
				/* If the current message happened to be the last message
				 * set, we need to change the lastErrorMessage pointer
				 * so it is not dangling.
				 */
				if(current_message == err_pool_data->lastErrorMessage)
				{
					err_pool_data->lastErrorMessage = NULL;
				}
				Internal_FreeErrorMessageStructure(current_message);
				current_message = NULL;
				prev_message->nextItem = next_message;
			}
		}
		/* It's not this thread, so increment everything for the next loop. */
		else
		{
			prev_message = current_message;
			current_message = next_message;
		}
	} /* End for-loop */

	Internal_UnlockMutex(err_pool->mutexLock);
}


void TError_GetLinkedVersion(TErrorVersion* ver)
{
	/* Check the pointer */
	if(NULL == ver)
	{
		/* Do nothing */
		return;
	}
	ver->major = TERROR_MAJOR_VERSION;
	ver->minor = TERROR_MINOR_VERSION;
	ver->patch = TERROR_PATCH_VERSION;
}


#if 0
/* This is for global initialization, not pool initialization. */
int TError_Init()
{
	/* initialize platform? */
	/* initialize mutexes? */

}
#endif

TErrorPool* TError_CreateErrorPool()
{
	TErrorPool* err_pool;
	TErrorPoolOpaqueData* err_pool_data;
	
	err_pool = (TErrorPool*)calloc(1, sizeof(TErrorPool));
	if(NULL == err_pool)
	{
		/* Very bad, but not sure what to do here. */
		return NULL;
	}
	err_pool_data = (TErrorPoolOpaqueData*)calloc(1, sizeof(TErrorPoolOpaqueData));
	if(NULL == err_pool_data)
	{
		/* Very bad, but not sure what to do here. */
		free(err_pool);
		return NULL;
	}

	/* Create mutex */
	err_pool->mutexLock = Internal_CreateMutex();
	
	if(NULL == err_pool->mutexLock)
	{
		/* Very bad, but not sure what to do here. */
		free(err_pool_data);
		free(err_pool);
		return NULL;
	}
	
	/* Attach the opaque data to the error pool. */
	err_pool->opaqueData = err_pool_data;

	/* The OpaqueData will hold the error message list, but it is 
	 * allowed to be NULL for an empty list so we don't have to allocate
	 * it here.
	 */

	return err_pool;
}

/* There better not be any contention when this is called. */
void TError_FreeErrorPool(TErrorPool* err_pool)
{
	if(NULL == err_pool)
	{
		return;
	}
	/* Free all the error messages for each thread.
	 * This locks and unlocks as it needs.
	 */
	Internal_FreeErrorMessageList(err_pool);

	/* Free opaque data structure. */
	free(err_pool->opaqueData);

	/* Delete mutex after all locking functions. */
	Internal_DestroyMutex(err_pool->mutexLock);

	/* Free main data structure. */
	free(err_pool);
}

void TError_SetError(TErrorPool* err_pool, int err_num, const char* err_str, ...)
{
	va_list argp;
	va_start(argp, err_str);
	TError_SetErrorv(err_pool, err_num, err_str, argp);
	va_end(argp);
}

void TError_SetErrorv(TErrorPool* err_pool, int err_num, const char* err_str, va_list argp)
{
	TErrorMessage* error_message;
	int ret_num_chars;
	
	if(NULL == err_pool)
	{
		return;
	}

	error_message = Internal_GetErrorOnCurrentThread(err_pool);
	/* If no error message was found, that means we must allocate
	 * a new entry for this entry.
	 */
	if(NULL == error_message)
	{
		error_message = Internal_CreateErrorOnCurrentThread(err_pool);
		/* If this fails, this is bad...not sure what to do though. */
		if(NULL == error_message)
		{
			return;
		}
	}

	/*
	 * I don't think I have to lock here. The [Get|Create]ErrorOnCurrentThread
	 * functions lock err_pool as they need access. Here, I don't access
	 * err_pool (which is shared) and error_message should be unique for 
	 * each thread so I don't think there is any contention. (Remember that
	 * simultaneous calls to SetError would only happen if they are in 
	 * different threads.)
	 * There *might* be a problem with library calls (strncpy, calloc). 
	 * I'm not sure if the various platforms are reentrant. 
	 * I guess for now, I will assume they won't bite me.
	 */

	/* If the err_str is NULL, we need to free our current string
	 * for consistency. More aggressive optimizations to hold the 
	 * memory might be considered in the future.
	 */
	if(NULL == err_str)
	{
		if(NULL != error_message->errorString)
		{
			free(error_message->errorString);
			error_message->errorString = NULL;
			error_message->errorMaxStringLength = 0;
		}
	}
	/* Else, copy the string */
	else
	{
		/* I am using vasprintf which is a GNU extension so it is not 
		 * portable. However, vasprintf makes certain things possible
		 * which would not be otherwise, which is the reason for my 
		 * use. The main benefit of asprintf/vasprintf is that you can
		 * create a string using printf style formatters without 
		 * worrying about the buffer size. sprintf should never be 
		 * used because of potential buffer overflows. snprintf
		 * is safer, but you are limited to a fixed size string
		 * which from time-to-time, I have exceeded unless you make
		 * the number really big. 
		 * Furthermore, snprintf itself is not currently terribly portable
		 * because it is specified only for C99 which some compilers 
		 * still have not have embraced.
		 * If you can't use the vasprintf implementation, 
		 * you must add -DDONT_USE_VASPRINTF to your compile flags.
		 */
#ifdef DONT_USE_VASPRINTF
		/* This implementation uses vsnprintf instead of 
		 * vasprintf. It is strongly recommended you use
		 * the vasprintf implmententation instead.
		 * Never use vsprintf unless you like 
		 * buffer overflows and security exploits.
		 */
		
		/* If the string was set to NULL, we must reallocate memory first. */
		if(NULL == error_message->errorString)
		{
			error_message->errorString = (char*)calloc(TERROR_DEFAULT_STRING_LENGTH, sizeof(char));
			if(NULL == error_message->errorString)
			{
				/* Very bad...what should I do?
				 */
				error_message->errorMaxStringLength = 0;
			}
			else
			{
				error_message->errorMaxStringLength = TERROR_DEFAULT_STRING_LENGTH;
			}
		}
		/* Because of the "Very Bad" situation directly above, 
		 * I need to check again to make sure the string isn't NULL.
		 * This will let the very bad situation continue on so va_end
		 * can be called and the error_number still has a chance to be set.
		 */
		if(NULL != error_message->errorString)
		{
			ret_num_chars = vsnprintf(error_message->errorString, 
				error_message->errorMaxStringLength,
				err_str,
				argp
			);
		}

#else /* DONT_USE_VASPRINTF */
		/* You might be wondering why the #ifdef logic assumes 
		 * asprintf is available instead of requiring an explicit 
		 * #define for that. The reason is because asprintf is the 
		 * better option and I want you to realize that you are not 
		 * using it. Typically, nobody knows or understands the build
		 * system and/or files get copied into new projects with a 
		 * entirely new build system, so it is easy to forget to 
		 * add a -D flag. So if you compile without asprintf,
		 * you are encouraged to explicitly know this.
		 */
		/* There may be a slight performance advantage to using snprintf
		 * over asprintf depending how asprintf is written. But this 
		 * implementation will completely destroy and reallocate a 
		 * string regardless if a string is set to NULL, so there will
		 * actually be no performance gains for these cases. 
		 * (This could be optimized but some additional bookkeeping 
		 * might be needed which might not be worth the effort and 
		 * code clutter.)
		 * As for memory allocation safety, because new messages for 
		 * different threads must be allocated dynamically, there is no
		 * way for this library to use purely static memory.
		 * So I don't believe there is anything to be gained using
		 * snprintf over asprintf and you lose out on arbitrary lengthed
		 * error messages.
		 * If memory allocation must be minimized, I recommend just 
		 * using the error number interface by itself which 
		 * will always keep the strings at NULL, and don't mess 
		 * with the asprintf/sprintf code.
		 */
		if(NULL != error_message->errorString)
		{
			/* Need to free errorString from previous pass otherwise we leak. 
			 * Maybe there is a smarter way to avoid the free/malloc,
			 * but this would probably require determining the length of the 
			 * final string beforehand which probably implies two 
			 * *printf calls which may or may not be better.
			 */
			free(error_message->errorString);
			error_message->errorString = NULL;
		}

		ret_num_chars = vasprintf(&error_message->errorString, err_str, argp);
		/* vasprintf returns -1 as an error */
		if(-1 == ret_num_chars)
		{
			/* Very bad, but not sure what to do here. */
			if(NULL != error_message->errorString)
			{
				free(error_message->errorString);
				error_message->errorString = NULL;
				error_message->errorMaxStringLength = 0;
				/* Don't return here. Still need to va_end, and
				 * there is a chance that the err_num might work.
				 * Plus the availability needs to be set.
				 */
			}
		}
		/* else vasprint returns the number of characters in the string
		 * not including the \0 character.
		 */
		else
		{
			/* I actually don't know how much memory vasprintf allocated
			 * for the string. But it is at least ret_num_chars+1, so
			 * I will use that as my max string length (which is 
			 * mainly used by CopyDynamicString() for efficiency 
			 * which is becoming less used in this code).
			 */
			error_message->errorMaxStringLength = ret_num_chars+1;
		}	
#endif /* DONT_USE_VASPRINTF */
	}

	/* I'm allowing for a user to explicitly clear an error message by 
	 * clearing both attributes.
	 */
	if((TERROR_NOERROR_VALUE == err_num) && (NULL == err_str))
	{
		error_message->errorNumber = TERROR_NOERROR_VALUE;
		error_message->errorAvailable = 0;
	}
	/* This is the normal case, copy the error number
	 * and mark the error as unread.
	 */
	else
	{
		error_message->errorNumber = err_num;
		error_message->errorAvailable = 1;
	}

	/* Now that the data is set, we also want to denote that this
	 * thread is the last error message. We need to lock for this
	 * since the lastError pointer is shared across threads.
	 */
	Internal_SetLastErrorMessagePointerToErrorMessage(err_pool, error_message);
}

void TError_SetErrorNoFormat(TErrorPool* err_pool, int err_num, const char* err_str)
{
	TErrorMessage* error_message;
	if(NULL == err_pool)
	{
		return;
	}

	error_message = Internal_GetErrorOnCurrentThread(err_pool);
	/* If no error message was found, that means we must allocate
	 * a new entry for this entry.
	 */
	if(NULL == error_message)
	{
		error_message = Internal_CreateErrorOnCurrentThread(err_pool);
		/* If this fails, this is bad...not sure what to do though. */
		if(NULL == error_message)
		{
			return;
		}
	}

	/*
	 * I don't think I have to lock here. The [Get|Create]ErrorOnCurrentThread
	 * functions lock err_pool as they need access. Here, I don't access
	 * err_pool (which is shared) and error_message should be unique for 
	 * each thread so I don't think there is any contention. (Remember that
	 * simultaneous calls to SetError would only happen if they are in 
	 * different threads.)
	 * There *might* be a problem with library calls (strncpy, calloc). 
	 * I'm not sure if the various platforms are reentrant. 
	 * I guess for now, I will assume they won't bite me.
	 */
	error_message->errorNumber = err_num;
	/* This will copy the string and set the new errorMaxStringLength as needed. */
	error_message->errorString = Internal_CopyDynamicString(error_message->errorString, &error_message->errorMaxStringLength, err_str);
	/* I'm allowing for a user to explicitly clear an error message by 
	 * clearing both attributes.
	 */
	if((TERROR_NOERROR_VALUE == err_num) && (NULL == err_str))
	{
		error_message->errorAvailable = 0;
	}
	else
	{
		error_message->errorAvailable = 1;
	}
	
	/* Now that the data is set, we also want to denote that this
	 * thread is the last error message. We need to lock for this
	 * since the lastError pointer is shared across threads.
	 */
	Internal_SetLastErrorMessagePointerToErrorMessage(err_pool, error_message);
}

void TError_SetErrorNum(TErrorPool* err_pool, int err_num)
{
	TError_SetErrorNoFormat(err_pool, err_num, NULL);
}

void TError_SetErrorStr(TErrorPool* err_pool, const char* err_str, ...)
{
	va_list argp;
	va_start(argp, err_str);
	if(NULL == err_str)
	{
		TError_SetErrorv(err_pool, TERROR_NOERROR_VALUE, err_str, argp);
	}
	else
	{
		TError_SetErrorv(err_pool, TERROR_ERROR_VALUE, err_str, argp);
	}
	va_end(argp);
}

void TError_SetErrorStrv(TErrorPool* err_pool, const char* err_str, va_list argp) 
{
	if(NULL == err_str)
	{
		TError_SetErrorv(err_pool, TERROR_NOERROR_VALUE, err_str, argp);
	}
	else
	{
		TError_SetErrorv(err_pool, TERROR_ERROR_VALUE, err_str, argp);
	}
}

/* If a NULL string is set, then it is presumed no error actually occurred
 * and this is a reset. So the err_num will be implicitly set to 0. Otherwise
 * the err_num will be set to 1 (for internal consistency and conventions).
 */
void TError_SetErrorStrNoFormat(TErrorPool* err_pool, const char* err_str)
{
	if(NULL == err_str)
	{
		TError_SetErrorNoFormat(err_pool, TERROR_NOERROR_VALUE, err_str);
	}
	else
	{
		TError_SetErrorNoFormat(err_pool, TERROR_ERROR_VALUE, err_str);
	}
}

/* This currently returns 0 as a "no error found" value.
 * This could potentially conflict with a user. For now, users 
 * shouldn't use 0 to represent an error. If this becomes a 
 * problem, we could introduce a magic number like -999999 and 
 * define TERROR_NO_ERROR_FOUND.
 */
int TError_GetErrorNumOnCurrentThread(TErrorPool* err_pool)
{
	TErrorMessage* error_message;

	error_message = Internal_GetErrorOnCurrentThread(err_pool);
	
	/* If no error message was found for the thread. */
	if(NULL == error_message)
	{
		return 0;
	}
	/* If an error message was found for the thread, but 
	 * it has already been read/cleared.
	 */
	if(0 == error_message->errorAvailable)
	{
		return 0;
	}
	/* We found a legitimate error message, clear it and return it. */
	error_message->errorAvailable = 0;
	return error_message->errorNumber;
}

const char* TError_GetErrorStrOnCurrentThread(TErrorPool* err_pool)
{
	TErrorMessage* error_message;

	error_message = Internal_GetErrorOnCurrentThread(err_pool);
	
	/* If no error message was found for the thread. */
	if(NULL == error_message)
	{
		return 0;
	}
	/* If an error message was found for the thread, but 
	 * it has already been read/cleared.
	 */
	if(0 == error_message->errorAvailable)
	{
		return 0;
	}
	/* We found a legitimate error message, clear it and return it. */
	error_message->errorAvailable = 0;
	return error_message->errorString;
}

TErrorStatus TError_GetErrorOnCurrentThread(TErrorPool* err_pool)
{
	TErrorMessage* error_message;
	TErrorStatus error_container;
	error_container.errorNumber = TERROR_NOERROR_VALUE;
	error_container.errorString = NULL;
	
	error_message = Internal_GetErrorOnCurrentThread(err_pool);
	
	/* If no error message was found for the thread. */
	if(NULL == error_message)
	{
		return error_container;
	}
	/* If an error message was found for the thread, but 
	 * it has already been read/cleared.
	 */
	if(0 == error_message->errorAvailable)
	{
		return error_container;
	}
	/* We found a legitimate error message, clear it and return it. */
	error_message->errorAvailable = 0;
	
	error_container.errorNumber = error_message->errorNumber;
	error_container.errorString = error_message->errorString;
	return error_container;
}

/* This function is for alternative usage where you just want one error 
 * for all threads. The backend will still work the same, but when you
 * call this function, it will look up the last set error, copy (with locking)
 * the last error to the current thread's memory, and return the object.
 * As always, since the returned object is only accessed on this thread, you
 * don't have to worry about locking.
 */
TErrorStatus TError_GetLastError(TErrorPool* err_pool)
{

	// Lock the error pool to get the lastMessage pointer
	// if the lastMessage pointer is pointing to data on the current thread,
	// we can just return it.
	// Otherwise, we need to copy the message 

	TErrorMessage* error_message;
	TErrorStatus error_container;
	error_container.errorNumber = TERROR_NOERROR_VALUE;
	error_container.errorString = NULL;
	
	error_message = Internal_GetLastError(err_pool);
	
	/* If no error message was found for the thread. */
	if(NULL == error_message)
	{
		return error_container;
	}
	/* If an error message was found for the thread, but 
	 * it has already been read/cleared.
	 */
	if(0 == error_message->errorAvailable)
	{
		return error_container;
	}
	/* We found a legitimate error message, clear it and return it. */
	error_message->errorAvailable = 0;
	
	error_container.errorNumber = error_message->errorNumber;
	error_container.errorString = error_message->errorString;
	return error_container;
}

/* This currently returns 0 as a "no error found" value.
 * This could potentially conflict with a user. For now, users 
 * shouldn't use 0 to represent an error. If this becomes a 
 * problem, we could introduce a magic number like -999999 and 
 * define TERROR_NO_ERROR_FOUND.
 */
int TError_GetLastErrorNum(TErrorPool* err_pool)
{
	TErrorMessage* error_message;

	error_message = Internal_GetLastError(err_pool);
	
	/* If no error message was found for the thread. */
	if(NULL == error_message)
	{
		return 0;
	}
	/* If an error message was found for the thread, but 
	 * it has already been read/cleared.
	 */
	if(0 == error_message->errorAvailable)
	{
		return 0;
	}
	/* We found a legitimate error message, clear it and return it. */
	error_message->errorAvailable = 0;
	return error_message->errorNumber;
}

const char* TError_GetLastErrorStr(TErrorPool* err_pool)
{
	TErrorMessage* error_message;

	error_message = Internal_GetLastError(err_pool);
	
	/* If no error message was found for the thread. */
	if(NULL == error_message)
	{
		return 0;
	}
	/* If an error message was found for the thread, but 
	 * it has already been read/cleared.
	 */
	if(0 == error_message->errorAvailable)
	{
		return 0;
	}
	/* We found a legitimate error message, clear it and return it. */
	error_message->errorAvailable = 0;
	return error_message->errorString;
}



