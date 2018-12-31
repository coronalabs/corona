#include "SimpleMutex.h"
#include <stdlib.h>

#if defined(DEBUG)
#include <stdio.h>
#define MUTEXDBG(x) printf x
#else
#define MUTEXDBG(x)
#endif

#if defined(_WIN32) && !defined(__CYGWIN32__)
		#include <windows.h>
		#include <winbase.h> /* For CreateMutex(), LockFile() */

		struct SimpleMutex
		{
			HANDLE nativeMutex;
		};


		SimpleMutex* SimpleMutex_CreateMutex()
		{
			SimpleMutex* simple_mutex = (SimpleMutex*)malloc(sizeof(SimpleMutex));
			if(NULL == simple_mutex)
			{
				MUTEXDBG(("Out of memory.\n"));				
				return NULL;		
			}
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
			simple_mutex->nativeMutex = CreateMutexExW(NULL, NULL, 0, SYNCHRONIZE);
#else
			simple_mutex->nativeMutex = CreateMutex(NULL, FALSE, NULL);
#endif
			if(NULL == simple_mutex->nativeMutex)
			{
				MUTEXDBG(("Out of memory.\n"));				
				free(simple_mutex);
				return NULL;		
			}
			return simple_mutex;
		}
		void SimpleMutex_DestroyMutex(SimpleMutex* simple_mutex)
		{
			if(NULL == simple_mutex)
			{
				return;
			}
			CloseHandle(simple_mutex->nativeMutex);
			free(simple_mutex);
		}
		/* This will return true if locking is successful, false if not.
		 */
		int SimpleMutex_LockMutex(SimpleMutex* simple_mutex)
		{
#ifdef DEBUG
			if(NULL == simple_mutex)
			{
				MUTEXDBG(("SimpleMutex_LockMutex was passed NULL\n"));	
				return 0;
			}
#endif
			return(
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
				WaitForSingleObjectEx(simple_mutex->nativeMutex, INFINITE, FALSE) != WAIT_FAILED
#else
				WaitForSingleObject(simple_mutex->nativeMutex, INFINITE) != WAIT_FAILED
#endif
			);
		}
		void SimpleMutex_UnlockMutex(SimpleMutex* simple_mutex)
		{
#ifdef DEBUG
			if(NULL == simple_mutex)
			{
				MUTEXDBG(("SimpleMutex_UnlockMutex was passed NULL\n"));	
				return;
			}
#endif
			ReleaseMutex(
				simple_mutex->nativeMutex
			);
		}
#else /* Assuming POSIX...maybe not a good assumption. */
		#include <pthread.h>

		struct SimpleMutex
		{
			pthread_mutex_t* nativeMutex;
		};

		SimpleMutex* SimpleMutex_CreateMutex()
		{
			int ret_val;
			SimpleMutex* simple_mutex = (SimpleMutex*)malloc(sizeof(SimpleMutex));
			if(NULL == simple_mutex)
			{
				MUTEXDBG(("Out of memory.\n"));				
				return NULL;		
			}
			simple_mutex->nativeMutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
			if(NULL == simple_mutex->nativeMutex)
			{
				MUTEXDBG(("Out of memory.\n"));
				free(simple_mutex);
				return NULL;		
			}

			ret_val = pthread_mutex_init(simple_mutex->nativeMutex, NULL);
			if(0 != ret_val)
			{
				free(simple_mutex->nativeMutex);
				free(simple_mutex);
				return NULL;
			}
			return simple_mutex;
		}
		void SimpleMutex_DestroyMutex(SimpleMutex* simple_mutex)
		{
			if(NULL != simple_mutex)
			{
				pthread_mutex_destroy(simple_mutex->nativeMutex);
				free(simple_mutex->nativeMutex);
				free(simple_mutex);
			}
		}
		/* This will return true if locking is successful, false if not.
		 * (This is the opposite of pthread_mutex_lock which returns 
		 * 0 for success.)
		 */
		int SimpleMutex_LockMutex(SimpleMutex* simple_mutex)
		{
#ifdef DEBUG
			if(NULL == simple_mutex)
			{
				MUTEXDBG(("SimpleMutex_LockMutex was passed NULL\n"));	
				return 0;
			}
#endif
			return(
				pthread_mutex_lock(
					simple_mutex->nativeMutex
				) == 0
			);
		}
		void SimpleMutex_UnlockMutex(SimpleMutex* simple_mutex)
		{
#ifdef DEBUG			
			if(NULL == simple_mutex)
			{
				MUTEXDBG(("SimpleMutex_LockMutex was passed NULL\n"));	
				return;
			}
#endif
			pthread_mutex_unlock(
				simple_mutex->nativeMutex
			);
		}
#endif


