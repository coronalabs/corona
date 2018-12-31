/*
 LinkedList
 Copyright (C) 2010  Eric Wing <ewing . public @ playcontrol.net>
 
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

/* I'm so sick of constantly rewriting linked lists in C. 
So here's some initial work to write a very simple reusable implementation.
The API could use a lot more functions, but I'll add them as I need them.
*/

#ifndef C_LINKED_LIST_H
#define C_LINKED_LIST_H

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif


#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */

/* Note: For Doxygen to produce clean output, you should set the 
 * PREDEFINED option to remove C_LINKED_LIST_DECLSPEC, C_LINKED_LIST_CALL, and
 * the DOXYGEN_SHOULD_IGNORE_THIS blocks.
 * PREDEFINED = DOXYGEN_SHOULD_IGNORE_THIS=1 C_LINKED_LIST_DECLSPEC= C_LINKED_LIST_CALL=
 */

/** Windows needs to know explicitly which functions to export in a DLL. */
#if defined(_WIN32) || defined(WINAPI_FAMILY)
	#if defined(C_LINKED_LIST_BUILD_LIBRARY)
		#define C_LINKED_LIST_DECLSPEC __declspec(dllexport)
	#else
		#define C_LINKED_LIST_DECLSPEC
	#endif
#else
	#if defined(C_LINKED_LIST_BUILD_LIBRARY)
		#if defined (__GNUC__) && __GNUC__ >= 4
			#define C_LINKED_LIST_DECLSPEC __attribute__((visibility("default")))
		#else
			#define C_LINKED_LIST_DECLSPEC
		#endif
	#else
		#define C_LINKED_LIST_DECLSPEC
	#endif
#endif

/* For Windows, by default, use the C calling convention */
#if defined(_WIN32) || defined(WINAPI_FAMILY)
	#define C_LINKED_LIST_CALL __cdecl
#else
	#define C_LINKED_LIST_CALL
#endif

/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */

	
#include <stddef.h>

typedef struct LinkedListNode LinkedListNode;
typedef struct LinkedList LinkedList;

extern C_LINKED_LIST_DECLSPEC LinkedList* C_LINKED_LIST_CALL LinkedList_Create(void);

extern C_LINKED_LIST_DECLSPEC void C_LINKED_LIST_CALL LinkedList_Free(LinkedList* linked_list);

extern C_LINKED_LIST_DECLSPEC void* C_LINKED_LIST_CALL LinkedList_Front(LinkedList* linked_list);

extern C_LINKED_LIST_DECLSPEC void* C_LINKED_LIST_CALL LinkedList_Back(LinkedList* linked_list);

extern C_LINKED_LIST_DECLSPEC unsigned int C_LINKED_LIST_CALL LinkedList_PushFront(LinkedList* linked_list, void* new_item);

extern C_LINKED_LIST_DECLSPEC unsigned int C_LINKED_LIST_CALL LinkedList_PushBack(LinkedList* linked_list, void* new_item);

extern C_LINKED_LIST_DECLSPEC void* C_LINKED_LIST_CALL LinkedList_PopFront(LinkedList* linked_list);

extern C_LINKED_LIST_DECLSPEC void* C_LINKED_LIST_CALL LinkedList_PopBack(LinkedList* linked_list);

extern C_LINKED_LIST_DECLSPEC size_t C_LINKED_LIST_CALL LinkedList_Size(LinkedList* linked_list);

extern C_LINKED_LIST_DECLSPEC void C_LINKED_LIST_CALL LinkedList_Clear(LinkedList* linked_list);

extern C_LINKED_LIST_DECLSPEC void* C_LINKED_LIST_CALL LinkedListNode_GetData(LinkedListNode* list_node);

extern C_LINKED_LIST_DECLSPEC LinkedListNode* C_LINKED_LIST_CALL LinkedList_Find(LinkedList* linked_list, void* the_data, LinkedListNode* start_node);

extern C_LINKED_LIST_DECLSPEC unsigned int C_LINKED_LIST_CALL LinkedList_Remove(LinkedList* linked_list, LinkedListNode* list_node);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
	
#endif /* C_LINKED_LIST_H */

