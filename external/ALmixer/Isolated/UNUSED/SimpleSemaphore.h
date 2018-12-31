#ifndef SIMPLE_SEMAPHORE_H
#define SIMPLE_SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

	
typedef struct SimpleSemaphore SimpleSemaphore;

SimpleSemaphore* SimpleSemaphore_CreateSemaphore(int initial_value);

void SimpleSemaphore_DestroySemaphore(SimpleSemaphore* simple_semaphore);

int SimpleSemaphore_SemaphoreTryWait(SimpleSemaphore* simple_semaphore);
int SimpleSemaphore_SemaphoreWait(SimpleSemaphore* simple_semaphore);
int SimpleSemaphore_SemaphoreGetValue(SimpleSemaphore* simple_semaphore);
int SimpleSemaphore_SemaphorePost(SimpleSemaphore* simple_semaphore);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif

