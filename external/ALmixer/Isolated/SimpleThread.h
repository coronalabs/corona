#ifndef SIMPLE_THREAD
#define SIMPLE_THREAD

#ifdef __cplusplus
extern "C" {
#endif
	
	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#if defined(SIMPLE_THREAD_BUILD_LIBRARY)
			#define SIMPLE_THREAD_DECLSPEC __declspec(dllexport)
		#else
			#define SIMPLE_THREAD_DECLSPEC
		#endif
	#else
		#if defined(SIMPLE_THREAD_BUILD_LIBRARY)
			#if defined (__GNUC__) && __GNUC__ >= 4
				#define SIMPLE_THREAD_DECLSPEC __attribute__((visibility("default")))
			#else
				#define SIMPLE_THREAD_DECLSPEC
			#endif
		#else
			#define SIMPLE_THREAD_DECLSPEC
		#endif
	#endif

	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#define SIMPLE_THREAD_CALL __cdecl
	#else
		#define SIMPLE_THREAD_CALL
	#endif

#include <stddef.h>

typedef struct SimpleThread SimpleThread;


extern SIMPLE_THREAD_DECLSPEC SimpleThread* SIMPLE_THREAD_CALL SimpleThread_CreateThread(int (*user_function)(void*), void* user_data);

extern SIMPLE_THREAD_DECLSPEC size_t SIMPLE_THREAD_CALL SimpleThread_GetCurrentThreadID(void);
extern SIMPLE_THREAD_DECLSPEC size_t SIMPLE_THREAD_CALL SimpleThread_GetThreadID(SimpleThread* simple_thread);

extern SIMPLE_THREAD_DECLSPEC void SIMPLE_THREAD_CALL SimpleThread_WaitThread(SimpleThread* simple_thread, int* thread_status);


extern SIMPLE_THREAD_DECLSPEC int SIMPLE_THREAD_CALL SimpleThread_GetThreadPriority(SimpleThread* simple_thread);
extern SIMPLE_THREAD_DECLSPEC void SIMPLE_THREAD_CALL SimpleThread_SetThreadPriority(SimpleThread* simple_thread, int priority_level);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif

