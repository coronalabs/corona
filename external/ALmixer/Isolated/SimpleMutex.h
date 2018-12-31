/* Copyright: Eric Wing 2003 */

#ifndef SIMPLE_MUTEX_H
#define SIMPLE_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#if defined(SIMPLE_MUTEX_BUILD_LIBRARY)
			#define SIMPLE_MUTEX_DECLSPEC __declspec(dllexport)
		#else
			#define SIMPLE_MUTEX_DECLSPEC
		#endif
	#else
		#if defined(SIMPLE_MUTEX_BUILD_LIBRARY)
			#if defined (__GNUC__) && __GNUC__ >= 4
				#define SIMPLE_MUTEX_DECLSPEC __attribute__((visibility("default")))
			#else
				#define SIMPLE_MUTEX_DECLSPEC
			#endif
		#else
			#define SIMPLE_MUTEX_DECLSPEC
		#endif
	#endif

	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#define SIMPLE_MUTEX_CALL __cdecl
	#else
		#define SIMPLE_MUTEX_CALL
	#endif

typedef struct SimpleMutex SimpleMutex;

extern SIMPLE_MUTEX_DECLSPEC SimpleMutex* SIMPLE_MUTEX_CALL SimpleMutex_CreateMutex(void);
extern SIMPLE_MUTEX_DECLSPEC void SIMPLE_MUTEX_CALL SimpleMutex_DestroyMutex(SimpleMutex* simple_mutex);
extern SIMPLE_MUTEX_DECLSPEC int SIMPLE_MUTEX_CALL SimpleMutex_LockMutex(SimpleMutex* simple_mutex);
extern SIMPLE_MUTEX_DECLSPEC void SIMPLE_MUTEX_CALL SimpleMutex_UnlockMutex(SimpleMutex* simple_mutex);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif

