#include "LinkedList.h"
#include <stddef.h> /* for NULL */
#include <stdlib.h> /* for malloc/free */
#include <stdio.h> /* for debugging */

struct LinkedListNode
{
	LinkedListNode* nextNode;
	LinkedListNode* previousNode;
	void* dataPtr;
};

struct LinkedList
{
	size_t currentSize;
	LinkedListNode* headPtr;
	LinkedListNode* tailPtr;
};


LinkedListNode* LinkedListNode_Create()
{
	LinkedListNode* list_node;
	list_node = (LinkedListNode*)calloc(1, sizeof(LinkedListNode));
	if(NULL == list_node)
	{
		/* Very bad, but not sure what to do here. */
		return NULL;
	}

	return list_node;
}


void LinkedListNode_Free(LinkedListNode* list_node)
{
	if(NULL == list_node)
	{
		return;
	}
	free(list_node);
}


LinkedList* LinkedList_Create()
{
	LinkedList* linked_list;
	linked_list = (LinkedList*)calloc(1, sizeof(LinkedList));
	if(NULL == linked_list)
	{
		/* Very bad, but not sure what to do here. */
		return NULL;
	}

	return linked_list;
}

void LinkedList_Free(LinkedList* linked_list)
{
	/* Both functions check for NULL */
	LinkedList_Clear(linked_list);
	free(linked_list);
}


/**
 * Returns 1 if successful, 0 if failure.
 */
unsigned int LinkedList_PushFront(LinkedList* linked_list, void* new_item)
{
	LinkedListNode* new_node;
	
	if(NULL == linked_list)
	{
		return 0;
	}

	new_node = LinkedListNode_Create();
	if(NULL == new_node)
	{
		return 0;
	}

	new_node->dataPtr = new_item;

	if(0 == linked_list->currentSize)
	{
		linked_list->tailPtr = new_node;
	}
	else
	{
		LinkedListNode* head_node = linked_list->headPtr;
		new_node->nextNode = head_node;
		head_node->previousNode = new_node;
	}

	linked_list->headPtr = new_node;
	linked_list->currentSize++;
	return 1;
}

unsigned int LinkedList_PushBack(LinkedList* linked_list, void* new_item)
{
	LinkedListNode* new_node;
	
	if(NULL == linked_list)
	{
		return 0;
	}

	new_node = LinkedListNode_Create();
	if(NULL == new_node)
	{
		return 0;
	}

	new_node->dataPtr = new_item;

	if(0 == linked_list->currentSize)
	{
		linked_list->headPtr = new_node;
	}
	else
	{
		LinkedListNode* tail_node = linked_list->tailPtr;
		new_node->previousNode = tail_node;
		tail_node->nextNode = new_node;
	}

	linked_list->tailPtr = new_node;
	linked_list->currentSize++;
	return 1;

}

void* LinkedList_PopFront(LinkedList* linked_list)
{
	LinkedListNode* head_node;
	void* return_data;
	
	if(NULL == linked_list)
	{
		return 0;
	}
	if(0 == linked_list->currentSize)
	{
		return NULL;
	}

	head_node = linked_list->headPtr;
	return_data = head_node->dataPtr;
	
	if(1 ==  linked_list->currentSize)
	{
		LinkedList_Clear(linked_list);
	}
	else
	{
		LinkedListNode* next_node = head_node->nextNode;
		next_node->previousNode = NULL;
		LinkedListNode_Free(head_node);
		linked_list->headPtr = next_node;
		linked_list->currentSize = linked_list->currentSize - 1;

	}
	return return_data;
}

void* LinkedList_PopBack(LinkedList* linked_list)
{
	LinkedListNode* tail_node;
	void* return_data;
	
	if(NULL == linked_list)
	{
		return NULL;
	}
	if(0 == linked_list->currentSize)
	{
		return NULL;
	}

	tail_node = linked_list->tailPtr;
	return_data = tail_node->dataPtr;
	
	if(1 == linked_list->currentSize)
	{
		LinkedList_Clear(linked_list);
	}
	else
	{
		LinkedListNode* previous_node = tail_node->previousNode;
		previous_node->nextNode = NULL;
		LinkedListNode_Free(tail_node);
		linked_list->tailPtr = previous_node;
		linked_list->currentSize = linked_list->currentSize - 1;
	}
	return return_data;
}


void* LinkedList_Front(LinkedList* linked_list)
{
	LinkedListNode* head_node;
	void* return_data;
	
	if(NULL == linked_list)
	{
		return 0;
	}
	if(0 == linked_list->currentSize)
	{
		return NULL;
	}
	
	head_node = linked_list->headPtr;
	return_data = head_node->dataPtr;
	
	return return_data;
}

void* LinkedList_Back(LinkedList* linked_list)
{
	LinkedListNode* tail_node;
	void* return_data;
	
	if(NULL == linked_list)
	{
		return NULL;
	}
	if(0 == linked_list->currentSize)
	{
		return NULL;
	}
	
	tail_node = linked_list->tailPtr;
	return_data = tail_node->dataPtr;

	return return_data;
}


size_t LinkedList_Size(LinkedList* linked_list)
{
	if(NULL == linked_list)
	{
		return 0;
	}
	return linked_list->currentSize;
}

void LinkedList_Clear(LinkedList* linked_list)
{
	LinkedListNode* current_node;
	LinkedListNode* next_node;
	if(NULL == linked_list)
	{
		return;
	}

	current_node = linked_list->headPtr;
	while(NULL != current_node)
	{
		next_node = current_node->nextNode;
		LinkedListNode_Free(current_node);
		current_node = next_node;
	}
	linked_list->headPtr = NULL;
	linked_list->tailPtr = NULL;
	linked_list->currentSize = 0;
}

void* LinkedListNode_GetData(LinkedListNode* list_node)
{
	if(NULL == list_node)
	{
		return NULL;
	}
	return list_node->dataPtr;
}

LinkedListNode* LinkedList_Find(LinkedList* linked_list, void* the_data, LinkedListNode* start_node)
{
	LinkedListNode* current_node;
	if(NULL == linked_list)
	{
		return NULL;
	}
	if(NULL == start_node)
	{
		start_node = linked_list->headPtr;
	}
	
	for(current_node = start_node; NULL != current_node; current_node = current_node->nextNode)
	{
		if(current_node->dataPtr == the_data)
		{
			return current_node;
		}
	}
	return current_node;
}

/* Make sure your LinkedListNode is actually connected to the 
 * LinkedList instance you pass.
 */
unsigned int LinkedList_Remove(LinkedList* linked_list, LinkedListNode* list_node)
{
	LinkedListNode* previous_node;
	LinkedListNode* next_node;
	if(NULL == linked_list)
	{
		return 0;
	}
	if(NULL == list_node)
	{
		return 0;
	}
		
	if(1 ==  linked_list->currentSize)
	{
		LinkedList_Clear(linked_list);
	}
	else if(list_node == linked_list->headPtr)
	{
		LinkedList_PopFront(linked_list);
	}
	else if(list_node == linked_list->tailPtr)
	{
		LinkedList_PopBack(linked_list);
	}
	else
	{
		previous_node = list_node->previousNode;
		next_node = list_node->nextNode;

		previous_node->nextNode = next_node;
		next_node->previousNode = previous_node;
		LinkedListNode_Free(list_node);

		linked_list->currentSize = linked_list->currentSize - 1;
	}

	return 1;

}



