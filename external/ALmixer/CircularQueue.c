/*
    CircularQueue
    Copyright (C) 2002  Eric Wing

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "CircularQueue.h"
#include <stddef.h> /* for NULL */
#include <stdlib.h> /* for malloc/free */
#include <stdio.h> /* for debugging */

CircularQueueUnsignedInt* CircularQueueUnsignedInt_CreateQueue(unsigned int max_size)
{
	CircularQueueUnsignedInt* ret_ptr;
	if(max_size < 1)
	{
		return NULL;
	}
	ret_ptr = (CircularQueueUnsignedInt*)malloc(sizeof(CircularQueueUnsignedInt));
	if(NULL == ret_ptr)
	{
		/* Out of memory */
		return NULL;
	}
	ret_ptr->internalQueue = (unsigned int*)malloc(sizeof(unsigned int) * max_size);
	if(NULL == ret_ptr->internalQueue)
	{
		/* Out of memory */
		free(ret_ptr);
		return NULL;
	}
	ret_ptr->maxSize = max_size;
	ret_ptr->currentSize = 0;
	ret_ptr->headIndex = 0;
	ret_ptr->tailIndex = 0;

	return ret_ptr;
}

void CircularQueueUnsignedInt_FreeQueue(CircularQueueUnsignedInt* queue)
{
	if(NULL == queue)
	{
		return;
	}

	free(queue->internalQueue);
	free(queue);
}

/**
 * Returns 1 if successful, 0 if failure.
 */
unsigned int CircularQueueUnsignedInt_PushBack(CircularQueueUnsignedInt* queue, unsigned int value)
{
//	printf("pushBack: %d\n", value);
	
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(queue->currentSize >= queue->maxSize)
	{
		printf("failed to pushBack: %d\n", value);
		
		return 0;
	}
	temp_index = queue->tailIndex + 1;
	if(temp_index >= queue->maxSize)
	{
		/* need to wrap tail index around */
		temp_index = 0;
	}
	/* So with my implementation, the tail index actually points 
	 * to the slot right after the last value.
	 * So we will enter the value in the current tail, and then increment
	 * the tail.
	 * Note that in a full queue, the head and tail will be the same (I think).
	 */
	queue->internalQueue[queue->tailIndex] = value;
	queue->tailIndex = temp_index;
	queue->currentSize++;
		
	return 1;
}

/**
 * Returns 1 if successful, 0 if failure.
 */
unsigned int CircularQueueUnsignedInt_PushFront(CircularQueueUnsignedInt* queue, unsigned int value)
{
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(queue->currentSize >= queue->maxSize)
	{
		return 0;
	}

	/* This check is needed to prevent the unsigned int from overflowing. */
	if(0 == queue->headIndex)
	{
		/* Need to wrap head index around */
		temp_index = queue->maxSize - 1;
	}
	else
	{
		temp_index = queue->headIndex - 1;
	}
	/* So unlike the tail, the head index actually points to the element
	 * at the head, and not an element before (or after) the head.
	 */
	queue->internalQueue[temp_index] = value;
	queue->headIndex = temp_index;
	queue->currentSize++;
	return 1;
}

unsigned int CircularQueueUnsignedInt_PopFront(CircularQueueUnsignedInt* queue)
{
	unsigned int temp_index;
//	printf("PopFront: %d, %d\n", queue->headIndex,queue->internalQueue[queue->headIndex]  );
	
	if(NULL == queue)
	{
		return 0;
	}
	if(queue->currentSize == 0)
	{
		return 0;
	}
	temp_index = queue->headIndex + 1;
	if(temp_index >= queue->maxSize)
	{
		/* need to wrap tail index around */
		temp_index = 0;
	}
	queue->headIndex = temp_index;
	queue->currentSize--;
	return 1;
}


unsigned int CircularQueueUnsignedInt_PopBack(CircularQueueUnsignedInt* queue)
{
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(queue->currentSize == 0)
	{
		return 0;
	}

	/* This check is needed to prevent the unsigned int from overflowing. */
	if(0 == queue->tailIndex)
	{
		temp_index = queue->maxSize - 1;
	}
	else
	{
		temp_index = queue->tailIndex - 1;
	}

	queue->tailIndex = temp_index;
	queue->currentSize--;
	return 1;
}

unsigned int CircularQueueUnsignedInt_Front(CircularQueueUnsignedInt* queue)
{
	if(NULL == queue)
	{
		return 0;
	}
	if(0 == queue->currentSize)
	{
		return 0;
	}
	return queue->internalQueue[queue->headIndex];
}


unsigned int CircularQueueUnsignedInt_Back(CircularQueueUnsignedInt* queue)
{
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(0 == queue->currentSize)
	{
		return 0;
	}
	if(0 == queue->tailIndex)
	{
		/* need to wrap tail index around */
		temp_index = queue->maxSize-1;
	}
	else
	{
		temp_index = queue->tailIndex-1;
	}
	
	return queue->internalQueue[temp_index];
}

unsigned int CircularQueueUnsignedInt_Size(CircularQueueUnsignedInt* queue)
{
	if(NULL == queue)
	{
		return 0;
	}
	return queue->currentSize;
}

unsigned int CircularQueueUnsignedInt_MaxSize(CircularQueueUnsignedInt* queue)
{
	if(NULL == queue)
	{
		return 0;
	}
	return queue->maxSize;
}

void CircularQueueUnsignedInt_Clear(CircularQueueUnsignedInt* queue)
{
	if(NULL == queue)
	{
		return;
	}

	queue->currentSize = 0;
	queue->headIndex = 0;
	queue->tailIndex = 0;
}

void CircularQueueUnsignedInt_Print(CircularQueueUnsignedInt* queue)
{
	unsigned int i;
	unsigned int count;
	if(NULL == queue)
	{
		return;
	}
	fprintf(stderr, "Queue: ");
	for(count=0, i=queue->headIndex; count<queue->currentSize; count++, i++)
	{
		if(i >= queue->maxSize)
		{
			i=0;
		}
		fprintf(stderr, "%d ", queue->internalQueue[i]);
	}
	fprintf(stderr, "\n");
}

unsigned int CircularQueueUnsignedInt_ValueAtIndex(CircularQueueUnsignedInt* queue, unsigned int the_index)
{
	unsigned int i;
	if(NULL == queue)
	{
		return 0;
	}
	if(the_index >= queue->currentSize)
	{
		return 0;
	}
	i = (queue->headIndex + the_index) % queue->currentSize;
//	fprintf(stderr, "%d\n", queue->internalQueue[i]);
	return queue->internalQueue[i];
}

/*
 * Implementation for void* version starts here.
 */

CircularQueueVoid* CircularQueueVoid_CreateQueue(unsigned int max_size)
{
	CircularQueueVoid* ret_ptr;
	if(max_size < 1)
	{
		return NULL;
	}
	ret_ptr = (CircularQueueVoid*)malloc(sizeof(CircularQueueVoid));
	if(NULL == ret_ptr)
	{
		/* Out of memory */
		return NULL;
	}
	ret_ptr->internalQueue = (void**)malloc(sizeof(void*) * max_size);
	if(NULL == ret_ptr->internalQueue)
	{
		/* Out of memory */
		free(ret_ptr);
		return NULL;
	}
	ret_ptr->maxSize = max_size;
	ret_ptr->currentSize = 0;
	ret_ptr->headIndex = 0;
	ret_ptr->tailIndex = 0;

	return ret_ptr;
}

void CircularQueueVoid_FreeQueue(CircularQueueVoid* queue)
{
	if(NULL == queue)
	{
		return;
	}

	free(queue->internalQueue);
	free(queue);
}

/**
 * Returns 1 if successful, 0 if failure.
 */
unsigned int CircularQueueVoid_PushBack(CircularQueueVoid* queue, void* value)
{
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(queue->currentSize >= queue->maxSize)
	{
		return 0;
	}
	temp_index = queue->tailIndex + 1;
	if(temp_index >= queue->maxSize)
	{
		/* need to wrap tail index around */
		temp_index = 0;
	}
	queue->internalQueue[queue->tailIndex] = value;
	queue->tailIndex = temp_index;
	queue->currentSize++;
		
	return 1;
}

/**
 * Returns 1 if successful, 0 if failure.
 */
unsigned int CircularQueueVoid_PushFront(CircularQueueVoid* queue, void* value)
{
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(queue->currentSize >= queue->maxSize)
	{
		return 0;
	}

	/* This check is needed to prevent the unsigned int from overflowing. */
	if(0 == queue->headIndex)
	{
		/* Need to wrap head index around */
		temp_index = queue->maxSize - 1;
	}
	else
	{
		temp_index = queue->headIndex - 1;
	}
	queue->internalQueue[temp_index] = value;
	queue->headIndex = temp_index;
	queue->currentSize++;
	return 1;
}

unsigned int CircularQueueVoid_PopFront(CircularQueueVoid* queue)
{
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(queue->currentSize == 0)
	{
		return 0;
	}
	temp_index = queue->headIndex + 1;
	if(temp_index >= queue->maxSize)
	{
		/* need to wrap tail index around */
		temp_index = 0;
	}
	queue->headIndex = temp_index;
	queue->currentSize--;
	return 1;
}


unsigned int CircularQueueVoid_PopBack(CircularQueueVoid* queue)
{
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(queue->currentSize == 0)
	{
		return 0;
	}

	/* This check is needed to prevent the unsigned int from overflowing. */
	if(0 == queue->tailIndex)
	{
		temp_index = queue->maxSize - 1;
	}
	else
	{
		temp_index = queue->tailIndex - 1;
	}

	queue->tailIndex = temp_index;
	queue->currentSize--;
	return 1;
}

void* CircularQueueVoid_Front(CircularQueueVoid* queue)
{
	if(NULL == queue)
	{
		return 0;
	}
	if(0 == queue->currentSize)
	{
		return 0;
	}
	return queue->internalQueue[queue->headIndex];
}


void* CircularQueueVoid_Back(CircularQueueVoid* queue)
{
	unsigned int temp_index;
	if(NULL == queue)
	{
		return 0;
	}
	if(0 == queue->currentSize)
	{
		return 0;
	}
	if(0 == queue->tailIndex)
	{
		/* need to wrap tail index around */
		temp_index = queue->maxSize-1;
	}
	else
	{
		temp_index = queue->tailIndex-1;
	}
	
	return queue->internalQueue[temp_index];
}

unsigned int CircularQueueVoid_Size(CircularQueueVoid* queue)
{
	if(NULL == queue)
	{
		return 0;
	}
	return queue->currentSize;
}

unsigned int CircularQueueVoid_MaxSize(CircularQueueVoid* queue)
{
	if(NULL == queue)
	{
		return 0;
	}
	return queue->maxSize;
}

void CircularQueueVoid_Clear(CircularQueueVoid* queue)
{
	if(NULL == queue)
	{
		return;
	}

	queue->currentSize = 0;
	queue->headIndex = 0;
	queue->tailIndex = 0;
}

void CircularQueueVoid_Print(CircularQueueVoid* queue)
{
	unsigned int i;
	unsigned int count;
	if(NULL == queue)
	{
		return;
	}
	fprintf(stderr, "Queue: ");
	for(count=0, i=queue->headIndex; count<queue->currentSize; count++, i++)
	{
		if(i >= queue->maxSize)
		{
			i=0;
		}
		fprintf(stderr, "%x ", (unsigned int)queue->internalQueue[i]);
	}
	fprintf(stderr, "\n");
}

void* CircularQueueVoid_ValueAtIndex(CircularQueueVoid* queue, unsigned int the_index)
{
	unsigned int i;
	if(NULL == queue)
	{
		return NULL;
	}
	if(the_index >= queue->currentSize)
	{
		return NULL;
	}
	i = (queue->headIndex + the_index) % queue->currentSize;
	//	fprintf(stderr, "%d\n", queue->internalQueue[i]);
	return queue->internalQueue[i];
}


