
#include "LuaHashMap.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

//#define ENABLE_BENCHMARK

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
#include <QuartzCore/QuartzCore.h>
#endif

static int Internal_safestrcmp(const char* str1, const char* str2)
{
	if(NULL == str1 && NULL == str2)
	{
		return 0;
	}
	else if(NULL == str1)
	{
		return 1;
	}
	else if(NULL == str2)
	{
		return -1;
	}
	else
	{
		return strcmp(str1, str2);
	}
}



static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
	(void)ud;
	(void)osize;
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

void TestSimpleKeyStringNumberValue()
{
	LuaHashMap* hash_map = LuaHashMap_Create();
	
	fprintf(stderr, "TestSimpleKeyStringNumberValue start\n");
	
	LuaHashMap_SetValueNumberForKeyString(hash_map, 3.99, "milk");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 4.349, "gas");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 2.99, "bread");
	
	
	fprintf(stderr, "Price of gas: %lf\n", LuaHashMap_GetValueNumberForKeyString(hash_map, "gas"));
	assert(3.99 == LuaHashMap_GetValueNumberForKeyString(hash_map, "milk"));
	assert(4.349 == LuaHashMap_GetValueNumberForKeyString(hash_map, "gas"));
	assert(2.99 == LuaHashMap_GetValueNumberForKeyString(hash_map, "bread"));
	assert(3 == LuaHashMap_Count(hash_map));
	LuaHashMap_Free(hash_map);
	
	fprintf(stderr, "TestSimpleKeyStringNumberValue done\n");

}

// http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
void gen_random_string(char *s, const int len) {
    static const char alphanum[] =
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz";
	
    for (int i = 0; i < len-1; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
	
    s[len-1] = 0;
}

void TestSimpleKeyStringNumberValueWithIterator()
{	
	int i;
	LuaHashMap* hash_map = LuaHashMap_Create();
	/* Better to declare when you assign it, but you need C99 */
	LuaHashMapIterator hash_iterator;
	
	fprintf(stderr, "TestSimpleKeyStringNumberValueWithIterator start\n");

	LuaHashMap_SetValueNumberForKeyString(hash_map, 3.99, "milk");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 4.349, "gas");
	LuaHashMap_SetValueNumberForKeyString(hash_map, 2.99, "bread");
	
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Price of %s: %lf\n", 
			   LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
			   LuaHashMap_GetValueNumberAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));

	assert(3.99 == LuaHashMap_GetValueNumberForKeyString(hash_map, "milk"));
	assert(4.349 == LuaHashMap_GetValueNumberForKeyString(hash_map, "gas"));
	assert(2.99 == LuaHashMap_GetValueNumberForKeyString(hash_map, "bread"));
	assert(3 == LuaHashMap_Count(hash_map));

	
	
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Removing: Price of %s: %lf\n", 
				LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
				LuaHashMap_GetValueNumberAtIterator(&hash_iterator));
		LuaHashMap_RemoveAtIterator(&hash_iterator);
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));
	assert(0 == LuaHashMap_Count(hash_map));

	
	
	
	LuaHashMap_Clear(hash_map);
	
	srand(1); // for consistency
	for(i=0; i<1024*64; i++)
	{	
		static char str_buffer[1024];
		int str_length = rand()%1024;
		gen_random_string(str_buffer, str_length);
		LuaHashMap_SetValueNumberForKeyString(hash_map, i, str_buffer);
	}
//	assert(1000000 == LuaHashMap_Count(hash_map));
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));
	
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
//		fprintf(stderr, "Removing: Price of %s: %lf\n", 
//				LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
//				LuaHashMap_GetValueNumberAtIterator(&hash_iterator));
		LuaHashMap_RemoveAtIterator(&hash_iterator);
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));
	assert(0 == LuaHashMap_Count(hash_map));
	
	
	
	LuaHashMap_Free(hash_map);
	
	fprintf(stderr, "TestSimpleKeyStringNumberValueWithIterator done\n");

}


void TestValueStringNULL()
{	
	LuaHashMap* hash_map = LuaHashMap_Create();
	/* Better to declare when you assign it, but you need C99 */
	LuaHashMapIterator hash_iterator;

	fprintf(stderr, "TestValueStringNULL start\n");
	
	LuaHashMap_SetValueStringForKeyString(hash_map, "$3.99", "milk");
	/* Lua treats NULL strings as pushing nil which removes an entry */
	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "gas");
	LuaHashMap_SetValueStringForKeyString(hash_map, "$2.99", "bread");
	
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Price of %s: %s\n", 
			   LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
			   LuaHashMap_GetValueStringAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));

	/* Lua treats NULL strings as pushing nil which removes an entry. Thus we only have 2. */
	/* Internal change: I now use lua_pushlstring and pass an explict 0 length. This causes Lua to push an empty string and create a key/value pair. So the count is 3. */
//	assert(2 == LuaHashMap_Count(hash_map));
	assert(3 == LuaHashMap_Count(hash_map));
//	assert(0 == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "milk"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "bread"));

	/* Lua treats NULL strings as pushing nil which removes an entry */
	/* Internal change: lua_pushlstring causes the value to be set to an empty string so the entry remains. */
	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "bread");

	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);	
	do
	{
		fprintf(stderr, "Price of %s: %s\n", 
			   LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
			   LuaHashMap_GetValueStringAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));
//	assert(1 == LuaHashMap_Count(hash_map));
	assert(3 == LuaHashMap_Count(hash_map));
//	assert(0 == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "milk"));
//	assert(0 == LuaHashMap_ExistsKeyString(hash_map, "bread"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "bread"));
	

	LuaHashMap_Free(hash_map);
	fprintf(stderr, "TestValueStringNULL done\n");	
}


void TestValuePointerNULL()
{	
	LuaHashMap* hash_map = LuaHashMap_Create();
	/* Better to declare when you assign it, but you need C99 */
	LuaHashMapIterator hash_iterator;
	LuaHashMapIterator hash_iterator_end;

	fprintf(stderr, "TestValuePointerNULL start\n");
	
	LuaHashMap_SetValuePointerForKeyString(hash_map, (void*)399, "milk");
	/* Unlike strings, Lua seems to push NULL pointers as 0 which does not remove an entry. */
	LuaHashMap_SetValuePointerForKeyString(hash_map, NULL, "gas");
	LuaHashMap_SetValuePointerForKeyString(hash_map, (void*)299, "bread");
	
	hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Price of %s: %zd\n", 
				LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
				LuaHashMap_GetValuePointerAtIterator(&hash_iterator));
		
	} while(LuaHashMap_IteratorNext(&hash_iterator));
	/* Unlike strings, Lua seems to push NULL pointers as 0 which does not remove an entry. Thus we have 3. */
	assert(3 == LuaHashMap_Count(hash_map));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "milk"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "bread"));
	assert((void*)399 == LuaHashMap_GetValuePointerForKeyString(hash_map, "milk"));
	assert(NULL == LuaHashMap_GetValuePointerForKeyString(hash_map, "gas"));
	assert((void*)299 == LuaHashMap_GetValuePointerForKeyString(hash_map, "bread"));

	
	LuaHashMap_SetValuePointerForKeyString(hash_map, NULL, "bread");

	/* Showing a different way to loop */
	for(hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map), hash_iterator_end = LuaHashMap_GetIteratorAtEnd(hash_map);
		! LuaHashMap_IteratorIsEqual(&hash_iterator, &hash_iterator_end);
		LuaHashMap_IteratorNext(&hash_iterator)
	)
	{
		fprintf(stderr, "Price of %s: %zd\n", 
				LuaHashMap_GetKeyStringAtIterator(&hash_iterator), 
				LuaHashMap_GetValuePointerAtIterator(&hash_iterator));
		
	}
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "gas"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "milk"));
	assert(1 == LuaHashMap_ExistsKeyString(hash_map, "bread"));
	assert((void*)399 == LuaHashMap_GetValuePointerForKeyString(hash_map, "milk"));
	assert(NULL == LuaHashMap_GetValuePointerForKeyString(hash_map, "gas"));
	assert(NULL == LuaHashMap_GetValuePointerForKeyString(hash_map, "bread"));
	
	
	
	
	/* Lua will also accept NULL for both keys and values when used as pointers (userdata) */
	LuaHashMap_SetValuePointerForKeyPointer(hash_map, NULL, NULL);
	assert(4 == LuaHashMap_Count(hash_map));
	 
	 
	 
	
	LuaHashMap_Free(hash_map);
	fprintf(stderr, "TestValuePointerNULL done\n");
	
	
}

void BenchMarkSameStringPointer()
{
	fprintf(stderr, "BenchMarkSameStringPointer start\n");

	const size_t NUMBER_OF_ELEMENTS = 1000000;
	const size_t NUM_OF_LOOPS = 2;
	char str_buffer[1024];
	char** array_of_strings = (char**)malloc(NUMBER_OF_ELEMENTS*sizeof(char*));
	LuaHashMap* hash_map = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	size_t i,j;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		gen_random_string(str_buffer, 1024);	
		array_of_strings[i] = LuaHashMap_SetValueIntegerForKeyString(hash_map, i, str_buffer);
	}
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	for(j=0; j<NUM_OF_LOOPS; j++)
	{
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			bool ret_flag;
			ret_flag = LuaHashMap_ExistsKeyString(hash_map, array_of_strings[i]);
			if(false == ret_flag)
			{
				fprintf(stderr, "Assertion failure. keystring: %s not in hash\n",  array_of_strings[i]);
			}
		}
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
#endif
	LuaHashMap_Free(hash_map);
	free(array_of_strings);
	fprintf(stderr, "BenchMarkSameStringPointer done\n");

}


void BenchMarkDifferentStringPointer()
{
	fprintf(stderr, "BenchMarkDifferentStringPointer start\n");

	const size_t NUMBER_OF_ELEMENTS = 1000000;
	const size_t NUM_OF_LOOPS = 2;
	char** array_of_strings = (char**)malloc(NUMBER_OF_ELEMENTS*sizeof(char*));
	LuaHashMap* hash_map = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	size_t i,j;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (char*)calloc(1024,sizeof(char));
		gen_random_string(array_of_strings[i], 1024);	
		LuaHashMap_SetValueIntegerForKeyString(hash_map, i, array_of_strings[i]);
	}
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	for(j=0; j<NUM_OF_LOOPS; j++)
	{
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			bool ret_flag;
			ret_flag = LuaHashMap_ExistsKeyString(hash_map, array_of_strings[i]);
			if(false == ret_flag)
			{
				fprintf(stderr, "Assertion failure. keystring: %s not in hash\n",  array_of_strings[i]);
			}
		}
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
#endif
	LuaHashMap_Free(hash_map);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]);
	}
	fprintf(stderr, "BenchMarkDifferentStringPointer done\n");
	free(array_of_strings);

}


void BenchMarkSameStringPointerWithLength()
{
	fprintf(stderr, "BenchMarkSameStringPointerWithLength start\n");

	const size_t NUMBER_OF_ELEMENTS = 1000000;
	const size_t NUM_OF_LOOPS = 2;
	char str_buffer[1024];
	LuaHashMapStringContainer** array_of_strings = (LuaHashMapStringContainer**)malloc(NUMBER_OF_ELEMENTS*sizeof(LuaHashMapStringContainer*));
	LuaHashMap* hash_map = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	size_t i,j;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (LuaHashMapStringContainer*)malloc(sizeof(LuaHashMapStringContainer));
		gen_random_string(str_buffer, 1024);
		array_of_strings[i]->stringLength = strlen(str_buffer);
		array_of_strings[i]->stringPointer = LuaHashMap_SetValueIntegerForKeyStringWithLength(hash_map, i, str_buffer, array_of_strings[i]->stringLength);
	}
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	for(j=0; j<NUM_OF_LOOPS; j++)
	{
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			bool ret_flag;
			ret_flag = LuaHashMap_ExistsKeyStringWithLength(hash_map, array_of_strings[i]->stringPointer, array_of_strings[i]->stringLength);
			if(false == ret_flag)
			{
				fprintf(stderr, "Assertion failure. keystring: %s not in hash\n",  array_of_strings[i]);
			}
		}
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
#endif
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]);
	}
	LuaHashMap_Free(hash_map);
	free(array_of_strings);
	fprintf(stderr, "BenchMarkSameStringPointerWithLength done\n");

}


void BenchMarkDifferentStringPointerWithLength()
{
	fprintf(stderr, "BenchMarkDifferentStringPointerWithLength start\n");

	const size_t NUMBER_OF_ELEMENTS = 1000000;
	const size_t NUM_OF_LOOPS = 2;
	LuaHashMapStringContainer** array_of_strings = (LuaHashMapStringContainer**)malloc(NUMBER_OF_ELEMENTS*sizeof(LuaHashMapStringContainer*));
	LuaHashMap* hash_map = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	size_t i,j;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (LuaHashMapStringContainer*)malloc(sizeof(LuaHashMapStringContainer));
		array_of_strings[i]->stringPointer = (char*)calloc(1024,sizeof(char));
		gen_random_string(array_of_strings[i]->stringPointer, 1024);
		array_of_strings[i]->stringLength = strlen(array_of_strings[i]->stringPointer);
		LuaHashMap_SetValueIntegerForKeyStringWithLength(hash_map, i, array_of_strings[i]->stringPointer, array_of_strings[i]->stringLength);
	}
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	for(j=0; j<NUM_OF_LOOPS; j++)
	{
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			bool ret_flag;
			ret_flag = LuaHashMap_ExistsKeyStringWithLength(hash_map, array_of_strings[i]->stringPointer, array_of_strings[i]->stringLength);
			if(false == ret_flag)
			{
				fprintf(stderr, "Assertion failure. keystring: %s not in hash\n",  array_of_strings[i]);
			}
		}
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
#endif
	LuaHashMap_Free(hash_map);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]->stringPointer);
		free(array_of_strings[i]);
	}
	fprintf(stderr, "BenchMarkDifferentStringPointerWithLength done\n");
	free(array_of_strings);

}

void BenchMarkExistsGetLookup()
{
	fprintf(stderr, "BenchMarkExistsGetLookup start\n");

	const size_t NUMBER_OF_ELEMENTS = 1000000;
	const size_t NUM_OF_LOOPS = 2;
	char str_buffer[1024];
	char** array_of_strings = (char**)malloc(NUMBER_OF_ELEMENTS*sizeof(char*));
	LuaHashMap* hash_map = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	size_t i,j;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		gen_random_string(str_buffer, 1024);	
		array_of_strings[i] = LuaHashMap_SetValueIntegerForKeyString(hash_map, i, str_buffer);
	}
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	for(j=0; j<NUM_OF_LOOPS; j++)
	{
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			bool ret_flag;
			ret_flag = LuaHashMap_ExistsKeyString(hash_map, array_of_strings[i]);
			if(false == ret_flag)
			{
				fprintf(stderr, "Assertion failure. keystring: %s not in hash\n",  array_of_strings[i]);
			}
			else
			{
				lua_Number ret_val = LuaHashMap_GetValueIntegerForKeyString(hash_map, array_of_strings[i]);
				/* Just so the compiler doesn't throw away code, use the value */
				if(ret_val > NUMBER_OF_ELEMENTS)
				{
					fprintf(stderr, "Assertion failure. value: %d exceeds NUMBER_OF_ELEMENTS\n", ret_val);
				}
			}
		}
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
#endif
	LuaHashMap_Free(hash_map);
	free(array_of_strings);
	fprintf(stderr, "BenchMarkExistsGetLookup done\n");

}


void BenchMarkIteratorGetLookup()
{
	fprintf(stderr, "BenchMarkIteratorGetLookup start\n");

	const size_t NUMBER_OF_ELEMENTS = 1000000;
	const size_t NUM_OF_LOOPS = 2;
	char str_buffer[1024];
	char** array_of_strings = (char**)malloc(NUMBER_OF_ELEMENTS*sizeof(char*));
	LuaHashMap* hash_map = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	size_t i,j;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		gen_random_string(str_buffer, 1024);	
		array_of_strings[i] = LuaHashMap_SetValueIntegerForKeyString(hash_map, i, str_buffer);
	}
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	for(j=0; j<NUM_OF_LOOPS; j++)
	{
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			bool ret_flag;
			LuaHashMapIterator the_iterator = LuaHashMap_GetIteratorForKeyString(hash_map, array_of_strings[i]);
			if(LuaHashMap_IteratorIsNotFound(&the_iterator))
			{
				fprintf(stderr, "Assertion failure. keystring: %s not in hash\n",  array_of_strings[i]);
			}
			else
			{
				lua_Number ret_val = LuaHashMap_GetCachedValueIntegerAtIterator(&the_iterator);
				/* Just so the compiler doesn't throw away code, use the value */
				if(ret_val > NUMBER_OF_ELEMENTS)
				{
					fprintf(stderr, "Assertion failure. value: %d exceeds NUMBER_OF_ELEMENTS\n", ret_val);
				}
			}
		}
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
#endif
	LuaHashMap_Free(hash_map);
	free(array_of_strings);
	fprintf(stderr, "BenchMarkIteratorGetLookup done\n");

}


void BenchMarkExistsGetLookupWithLength()
{
	fprintf(stderr, "BenchMarkExistsGetLookupWithLength start\n");

	const size_t NUMBER_OF_ELEMENTS = 1000000;
	const size_t NUM_OF_LOOPS = 2;
	char str_buffer[1024];
	LuaHashMapStringContainer** array_of_strings = (LuaHashMapStringContainer**)malloc(NUMBER_OF_ELEMENTS*sizeof(LuaHashMapStringContainer*));
	LuaHashMap* hash_map = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	size_t i,j;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (LuaHashMapStringContainer*)malloc(sizeof(LuaHashMapStringContainer));

		gen_random_string(str_buffer, 1024);
		array_of_strings[i]->stringLength = strlen(str_buffer);
		array_of_strings[i]->stringPointer = LuaHashMap_SetValueIntegerForKeyStringWithLength(hash_map, i, str_buffer, array_of_strings[i]->stringLength);
	}
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	for(j=0; j<NUM_OF_LOOPS; j++)
	{
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			bool ret_flag;
			ret_flag = LuaHashMap_ExistsKeyStringWithLength(hash_map, array_of_strings[i]->stringPointer, array_of_strings[i]->stringLength);
			if(false == ret_flag)
			{
				fprintf(stderr, "Assertion failure. keystring: %s not in hash\n",  array_of_strings[i]);
			}
			else
			{
				lua_Number ret_val = LuaHashMap_GetValueIntegerForKeyStringWithLength(hash_map, array_of_strings[i]->stringPointer, array_of_strings[i]->stringLength);
				/* Just so the compiler doesn't throw away code, use the value */
				if(ret_val > NUMBER_OF_ELEMENTS)
				{
					fprintf(stderr, "Assertion failure. value: %d exceeds NUMBER_OF_ELEMENTS\n", ret_val);
				}
			}
		}
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
#endif
	LuaHashMap_Free(hash_map);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]);
	}
	free(array_of_strings);
	fprintf(stderr, "BenchMarkExistsGetLookupWithLength done\n");

}


void BenchMarkIteratorGetLookupWithLength()
{
	fprintf(stderr, "BenchMarkIteratorGetLookupWithLength start\n");

	const size_t NUMBER_OF_ELEMENTS = 1000000;
	const size_t NUM_OF_LOOPS = 2;
	char str_buffer[1024];
	LuaHashMapStringContainer** array_of_strings = (LuaHashMapStringContainer**)malloc(NUMBER_OF_ELEMENTS*sizeof(LuaHashMapStringContainer*));
	LuaHashMap* hash_map = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	size_t i,j;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (LuaHashMapStringContainer*)malloc(sizeof(LuaHashMapStringContainer));
		gen_random_string(str_buffer, 1024);
		array_of_strings[i]->stringLength = strlen(str_buffer);
		array_of_strings[i]->stringPointer = LuaHashMap_SetValueIntegerForKeyStringWithLength(hash_map, i, str_buffer, array_of_strings[i]->stringLength);
	}
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map));

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	for(j=0; j<NUM_OF_LOOPS; j++)
	{
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			bool ret_flag;
			LuaHashMapIterator the_iterator = LuaHashMap_GetIteratorForKeyStringWithLength(hash_map, array_of_strings[i]->stringPointer, array_of_strings[i]->stringLength);
			if(LuaHashMap_IteratorIsNotFound(&the_iterator))
			{
				fprintf(stderr, "Assertion failure. keystring: %s not in hash\n",  array_of_strings[i]);
			}
			else
			{
				lua_Number ret_val = LuaHashMap_GetCachedValueIntegerAtIterator(&the_iterator);
				/* Just so the compiler doesn't throw away code, use the value */
				if(ret_val > NUMBER_OF_ELEMENTS)
				{
					fprintf(stderr, "Assertion failure. value: %d exceeds NUMBER_OF_ELEMENTS\n", ret_val);
				}
			}
		}
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
#endif
	LuaHashMap_Free(hash_map);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]);
	}
	free(array_of_strings);
	fprintf(stderr, "BenchMarkIteratorGetLookupWithLength done\n");

}


void BenchMarkStringInsertionDifferentPointer()
{
	fprintf(stderr, "BenchMarkStringInsertionDifferentPointer start\n");
	
	const size_t NUMBER_OF_ELEMENTS = 1000000;
	char** array_of_strings = (char**)malloc(NUMBER_OF_ELEMENTS*sizeof(char*));
	LuaHashMap* hash_map1 = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	LuaHashMap* hash_map2 = LuaHashMap_CreateShareWithSizeHints(hash_map1, 0, NUMBER_OF_ELEMENTS);
	size_t i;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (char*)calloc(1024,sizeof(char));
		gen_random_string(array_of_strings[i], 1024);	
	}
#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	{
	fprintf(stderr, "BenchMarkStringInsertionDifferentPointer insertion into map1\n");
	CFTimeInterval start_time = CACurrentMediaTime();

	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		LuaHashMap_SetValueIntegerForKeyString(hash_map1, i, array_of_strings[i]);
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	}	
	{
		lua_gc(LuaHashMap_GetLuaState(hash_map1), LUA_GCCOLLECT, 0);
	}
	{
	fprintf(stderr, "BenchMarkStringInsertionDifferentPointer insertion into map2\n");
	CFTimeInterval start_time = CACurrentMediaTime();
	
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		LuaHashMap_SetValueIntegerForKeyString(hash_map2, i, array_of_strings[i]);
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	}
#endif
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map1));
	
	LuaHashMap_FreeShare(hash_map2);
	LuaHashMap_Free(hash_map1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]);
	}
	fprintf(stderr, "BenchMarkStringInsertionDifferentPointer done\n");
	free(array_of_strings);
	
}


void BenchMarkStringInsertionSamePointer()
{
	fprintf(stderr, "BenchMarkStringInsertionSamePointer start\n");
	
	const size_t NUMBER_OF_ELEMENTS = 1000000;
	char** array_of_strings = (char**)malloc(NUMBER_OF_ELEMENTS*sizeof(char*));
	char** array_of_internal_strings = (char**)malloc(NUMBER_OF_ELEMENTS*sizeof(char*));
	LuaHashMap* hash_map1 = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	LuaHashMap* hash_map2 = LuaHashMap_CreateShareWithSizeHints(hash_map1, 0, NUMBER_OF_ELEMENTS);
	size_t i;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (char*)calloc(1024,sizeof(char));
		gen_random_string(array_of_strings[i], 1024);	
	}
#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	{
		fprintf(stderr, "BenchMarkStringInsertionSamePointer insertion into map1\n");
		CFTimeInterval start_time = CACurrentMediaTime();
		
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			array_of_internal_strings[i] = LuaHashMap_SetValueIntegerForKeyString(hash_map1, i, array_of_strings[i]);
		}
		CFTimeInterval end_time = CACurrentMediaTime();
		fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	}	
	{
		lua_gc(LuaHashMap_GetLuaState(hash_map1), LUA_GCCOLLECT, 0);
	}
	{
		fprintf(stderr, "BenchMarkStringInsertionSamePointer insertion into map2\n");
		CFTimeInterval start_time = CACurrentMediaTime();
		
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			array_of_internal_strings[i] = LuaHashMap_SetValueIntegerForKeyString(hash_map2, i, array_of_internal_strings[i]);
		}
		CFTimeInterval end_time = CACurrentMediaTime();
		fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	}
#endif
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map1));
	
	LuaHashMap_FreeShare(hash_map2);
	LuaHashMap_Free(hash_map1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]);
	}
	fprintf(stderr, "BenchMarkStringInsertionSamePointer done\n");
	free(array_of_strings);
	free(array_of_internal_strings);

}



void BenchMarkStringInsertionDifferentPointerWithLength()
{
	fprintf(stderr, "BenchMarkStringInsertionDifferentPointerWithLength start\n");
	
	const size_t NUMBER_OF_ELEMENTS = 1000000;
	LuaHashMapStringContainer** array_of_strings = (LuaHashMapStringContainer**)malloc(NUMBER_OF_ELEMENTS*sizeof(LuaHashMapStringContainer*));
	LuaHashMap* hash_map1 = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	LuaHashMap* hash_map2 = LuaHashMap_CreateShareWithSizeHints(hash_map1, 0, NUMBER_OF_ELEMENTS);
	size_t i;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (LuaHashMapStringContainer*)calloc(1,sizeof(LuaHashMapStringContainer));
		array_of_strings[i]->stringPointer = (char*)calloc(1024,sizeof(char));
		gen_random_string(array_of_strings[i]->stringPointer, 1024);
		array_of_strings[i]->stringLength = strlen(array_of_strings[i]->stringPointer);
	}
#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	{
	fprintf(stderr, "BenchMarkStringInsertionDifferentPointerWithLength insertion into map1\n");
	CFTimeInterval start_time = CACurrentMediaTime();

	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		LuaHashMap_SetValueIntegerForKeyStringWithLength(hash_map1, i, array_of_strings[i]->stringPointer, array_of_strings[i]->stringLength);
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	}	
	{
		lua_gc(LuaHashMap_GetLuaState(hash_map1), LUA_GCCOLLECT, 0);
	}
	{
	fprintf(stderr, "BenchMarkStringInsertionDifferentPointerWithLength insertion into map2\n");
	CFTimeInterval start_time = CACurrentMediaTime();
	
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		LuaHashMap_SetValueIntegerForKeyStringWithLength(hash_map2, i, array_of_strings[i]->stringPointer, array_of_strings[i]->stringLength);
	}
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	}
#endif
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map1));
	
	LuaHashMap_FreeShare(hash_map2);
	LuaHashMap_Free(hash_map1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]->stringPointer);
		free(array_of_strings[i]);
	}
	fprintf(stderr, "BenchMarkStringInsertionDifferentPointerWithLength done\n");
	free(array_of_strings);
	
}


void BenchMarkStringInsertionSamePointerWithLength()
{
	fprintf(stderr, "BenchMarkStringInsertionSamePointerWithLength start\n");
	
	const size_t NUMBER_OF_ELEMENTS = 1000000;
	char** array_of_strings = (char**)malloc(NUMBER_OF_ELEMENTS*sizeof(char*));
	LuaHashMapStringContainer** array_of_internal_strings = (LuaHashMapStringContainer**)malloc(NUMBER_OF_ELEMENTS*sizeof(LuaHashMapStringContainer*));
	LuaHashMap* hash_map1 = LuaHashMap_CreateWithSizeHints(0, NUMBER_OF_ELEMENTS);
	LuaHashMap* hash_map2 = LuaHashMap_CreateShareWithSizeHints(hash_map1, 0, NUMBER_OF_ELEMENTS);
	size_t i;
	srand(1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		array_of_strings[i] = (char*)calloc(1024,sizeof(char));
		array_of_internal_strings[i] = (LuaHashMapStringContainer*)malloc(sizeof(LuaHashMapStringContainer));
		gen_random_string(array_of_strings[i], 1024);
		array_of_internal_strings[i]->stringLength = strlen(array_of_strings[i]);
	}
#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	{
		fprintf(stderr, "BenchMarkStringInsertionSamePointerWithLength insertion into map1\n");
		CFTimeInterval start_time = CACurrentMediaTime();
		
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			array_of_internal_strings[i]->stringPointer = LuaHashMap_SetValueIntegerForKeyStringWithLength(hash_map1, i, array_of_strings[i], array_of_internal_strings[i]->stringLength);
		}
		CFTimeInterval end_time = CACurrentMediaTime();
		fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	}	
	{
		lua_gc(LuaHashMap_GetLuaState(hash_map1), LUA_GCCOLLECT, 0);
	}
	{
		fprintf(stderr, "BenchMarkStringInsertionSamePointerWithLength insertion into map2\n");
		CFTimeInterval start_time = CACurrentMediaTime();
		
		for(i=0; i<NUMBER_OF_ELEMENTS; i++)
		{
			array_of_internal_strings[i]->stringPointer = LuaHashMap_SetValueIntegerForKeyStringWithLength(hash_map2, i, array_of_internal_strings[i]->stringPointer, array_of_internal_strings[i]->stringLength);
		}
		CFTimeInterval end_time = CACurrentMediaTime();
		fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	}
#endif
	fprintf(stderr, "count: %d\n", LuaHashMap_Count(hash_map1));
	
	LuaHashMap_FreeShare(hash_map2);
	LuaHashMap_Free(hash_map1);
	for(i=0; i<NUMBER_OF_ELEMENTS; i++)
	{
		free(array_of_strings[i]);
		free(array_of_internal_strings[i]);
	}
	fprintf(stderr, "BenchMarkStringInsertionSamePointerWithLength done\n");
	free(array_of_strings);
	free(array_of_internal_strings);

}


#include <stddef.h>

struct dummy
{
    char x;
    LuaHashMapIterator t;
	char y;
};


#define MAX_ARRAY_SIZE 10

int main(int argc, char* argv[])
{

	const char* ret_string = NULL;
	const char* key_array[MAX_ARRAY_SIZE];
	size_t ret_size;
	size_t i;
	LuaHashMap* hash_map;
	LuaHashMapIterator the_iterator;

	size_t test_alignment = offsetof(struct dummy, t);
	fprintf(stderr, "test_alignment: %zu\n", test_alignment);

	fprintf(stderr, "test_alignment: %zu\n", offsetof(LuaHashMapIterator, currentKey));
	fprintf(stderr, "test_alignment: %zu\n", offsetof(LuaHashMapIterator, currentValue));
	fprintf(stderr, "test_alignment: %zu\n", offsetof(LuaHashMapIterator, hashMap));
	fprintf(stderr, "test_alignment: %zu\n", offsetof(LuaHashMapIterator, whichTable));
	fprintf(stderr, "test_alignment: %zu\n", offsetof(LuaHashMapIterator, keyType));
	fprintf(stderr, "test_alignment: %zu\n", offsetof(LuaHashMapIterator, atEnd));
	fprintf(stderr, "test_alignment: %zu\n", offsetof(LuaHashMapIterator, isNext));
	
	fprintf(stderr, "test_alignment: %zu\n", offsetof(struct dummy, y));

	
	fprintf(stderr, "create\n");
	hash_map = LuaHashMap_Create();
//	hash_map = LuaHashMap_CreateWithSizeHints(0, 600000, LUAHASHMAP_KEYSTRING_TYPE, LUAHASHMAP_VALUESTRING_TYPE);
//	hash_map = LuaHashMap_CreateWithAllocatorAndSizeHints(l_alloc, NULL, 0, 600000, LUAHASHMAP_KEYSTRING_TYPE, LUAHASHMAP_VALUESTRING_TYPE);

	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");
	LuaHashMap_SetValueStringForKeyString(hash_map, "value1", "key1");
	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");

	LuaHashMap_SetValueStringForKeyString(hash_map, "value2", "key2");
	fprintf(stderr, "LuaHashMap_SetValueStringForKeyString\n");

	LuaHashMap_SetValueStringForKeyString(hash_map, "value3", "key3");
//	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "key3");



	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key1");
	assert(0 == Internal_safestrcmp("value1", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key2");
	assert(0 == Internal_safestrcmp("value2", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);
	fprintf(stderr, "LuaHashMap_GetValueStringForKeyString\n");

	ret_string = LuaHashMap_GetValueStringForKeyString(hash_map, "key3");
	assert(0 == Internal_safestrcmp("value3", ret_string));
	fprintf(stderr, "ret_string=%s\n", ret_string);

	ret_size = LuaHashMap_GetKeysString(hash_map, key_array, MAX_ARRAY_SIZE);
	assert(3 == ret_size);
	for(i=0; i<ret_size; i++)
	{
		fprintf(stderr, "Key[%zd] is %s\n", i, key_array[i]);
	}

	assert(0 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be no: %d\n", LuaHashMap_IsEmpty(hash_map));


/*
	the_iterator = LuaHashMap_GetIteratorAtBeginForKeyString(hash_map);
 */
	the_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
	do
	{
		fprintf(stderr, "Using iterator: %s\n", LuaHashMap_GetValueStringAtIterator(&the_iterator));		
	} while(LuaHashMap_IteratorNext(&the_iterator));



	LuaHashMap_Clear(hash_map);
	ret_size = LuaHashMap_GetKeysString(hash_map, key_array, MAX_ARRAY_SIZE);
	assert(0 == ret_size);
	
	
	assert(1 == LuaHashMap_IsEmpty(hash_map));
	fprintf(stderr, "IsEmpty should be yes: %d\n", LuaHashMap_IsEmpty(hash_map));
	/* Lua does not allow NULL string keys. LuaHashMap gracefully returns early instead of letting Lua throw an error. */
	LuaHashMap_SetValueStringForKeyString(hash_map, "value3", NULL);
	fprintf(stderr, "LuaHashMap did not throw an error for a NULL key string (this is good)\n");

#if defined(ENABLE_BENCHMARK) && defined(__APPLE__)
	CFTimeInterval start_time = CACurrentMediaTime();

	void* ret_ptr = NULL;	
	for(i=0; i<400000; i++)
	{
		// Mixed types not really supported. Don't do this.
		LuaHashMap_SetValuePointerForKeyPointer(hash_map, (void*)i, (void*)rand());
		LuaHashMap_SetValueIntegerForKeyInteger(hash_map, rand(), rand());
		ret_ptr = LuaHashMap_GetValuePointerForKeyPointer(hash_map, (void*)i);
//		LuaHashMap_RemoveKeyPointer(hash_map, ret_ptr);
	}
	fprintf(stderr, "num keys= %d\n", LuaHashMap_GetKeysInteger(hash_map, NULL, 0));

	{
		LuaHashMapIterator hash_iterator = LuaHashMap_GetIteratorAtBegin(hash_map);
		do
		{
			LuaHashMap_RemoveAtIterator(&hash_iterator);
			
		} while(LuaHashMap_IteratorNext(&hash_iterator));
		assert(0 == LuaHashMap_Count(hash_map));
	}
	
	LuaHashMap_Clear(hash_map);
	CFTimeInterval end_time = CACurrentMediaTime();
	fprintf(stderr, "diff time: %lf\n", end_time-start_time);
	assert(1 == LuaHashMap_IsEmpty(hash_map));
	
	
	/* Results on iMac i3 64-bit release (Os), Lua 5.2.1rc2 as framework (O3?)
	 BenchMarkDifferentStringPointer start
	 count: 1000000
	 diff time: 4.626600
	 BenchMarkDifferentStringPointer done
	 BenchMarkSameStringPointer start
	 count: 1000000
	 diff time: 4.379855
	 BenchMarkSameStringPointer done
	 BenchMarkExistsGetLookup start
	 count: 1000000
	 diff time: 7.361952
	 BenchMarkExistsGetLookup done
	 BenchMarkIteratorGetLookup start
	 count: 1000000
	 diff time: 4.441443
	 BenchMarkIteratorGetLookup done
	 
	 (O2), added GC to BenchMarkStringInsertionDifferentPointer,BenchMarkStringInsertionSamePointer
	 BenchMarkDifferentStringPointer start
	 count: 1000000
	 diff time: 5.855453
	 BenchMarkDifferentStringPointer done
	 BenchMarkSameStringPointer start
	 count: 1000000
	 diff time: 4.460006
	 BenchMarkSameStringPointer done
	 BenchMarkExistsGetLookup start
	 count: 1000000
	 diff time: 7.389991
	 BenchMarkExistsGetLookup done
	 BenchMarkIteratorGetLookup start
	 count: 1000000
	 diff time: 4.491826
	 BenchMarkIteratorGetLookup done
	 BenchMarkStringInsertionDifferentPointer start
	 BenchMarkStringInsertionDifferentPointer insertion into map1
	 diff time: 1.861852
	 BenchMarkStringInsertionDifferentPointer insertion into map2
	 diff time: 1.770649
	 count: 1000000
	 BenchMarkStringInsertionDifferentPointer done
	 BenchMarkStringInsertionSamePointer start
	 BenchMarkStringInsertionSamePointer insertion into map1
	 diff time: 1.877305
	 BenchMarkStringInsertionSamePointer insertion into map2
	 diff time: 1.829054
	 count: 1000000
	 BenchMarkStringInsertionSamePointer done
	 
	 
	 (02) Rerun with additional WithLength APIs.
	 
	 BenchMarkDifferentStringPointer start
	 count: 1000000
	 diff time: 4.405813
	 BenchMarkDifferentStringPointer done
	 BenchMarkSameStringPointer start
	 count: 1000000
	 diff time: 4.213511
	 BenchMarkSameStringPointer done
	 BenchMarkDifferentStringPointerWithLength start
	 count: 1000000
	 diff time: 4.309061
	 BenchMarkDifferentStringPointerWithLength done
	 BenchMarkSameStringPointerWithLength start
	 count: 1000000
	 diff time: 4.025509
	 BenchMarkSameStringPointerWithLength done
	 BenchMarkExistsGetLookup start
	 count: 1000000
	 diff time: 7.044242
	 BenchMarkExistsGetLookup done
	 BenchMarkIteratorGetLookup start
	 count: 1000000
	 diff time: 4.350098
	 BenchMarkIteratorGetLookup done
	 BenchMarkExistsGetLookupWithLength start
	 count: 1000000
	 diff time: 6.729568
	 BenchMarkExistsGetLookupWithLength done
	 BenchMarkIteratorGetLookupWithLength start
	 count: 1000000
	 diff time: 4.191972
	 BenchMarkIteratorGetLookupWithLength done
	 BenchMarkStringInsertionDifferentPointer start
	 BenchMarkStringInsertionDifferentPointer insertion into map1
	 diff time: 1.744476
	 BenchMarkStringInsertionDifferentPointer insertion into map2
	 diff time: 1.649793
	 count: 1000000
	 BenchMarkStringInsertionDifferentPointer done
	 BenchMarkStringInsertionSamePointer start
	 BenchMarkStringInsertionSamePointer insertion into map1
	 diff time: 1.777968
	 BenchMarkStringInsertionSamePointer insertion into map2
	 diff time: 1.782678
	 count: 1000000
	 BenchMarkStringInsertionSamePointer done
	 BenchMarkStringInsertionDifferentPointerWithLength start
	 BenchMarkStringInsertionDifferentPointerWithLength insertion into map1
	 diff time: 1.725302
	 BenchMarkStringInsertionDifferentPointerWithLength insertion into map2
	 diff time: 1.654158
	 count: 1000000
	 BenchMarkStringInsertionDifferentPointerWithLength done
	 BenchMarkStringInsertionSamePointerWithLength start
	 BenchMarkStringInsertionSamePointerWithLength insertion into map1
	 diff time: 1.824345
	 BenchMarkStringInsertionSamePointerWithLength insertion into map2
	 diff time: 1.656639
	 count: 1000000
	 BenchMarkStringInsertionSamePointerWithLength done
	 
	 */
	 
	BenchMarkDifferentStringPointer();
	BenchMarkSameStringPointer();

	BenchMarkDifferentStringPointerWithLength();
	BenchMarkSameStringPointerWithLength();

	
	BenchMarkExistsGetLookup();
	BenchMarkIteratorGetLookup();
	
	BenchMarkExistsGetLookupWithLength();
	BenchMarkIteratorGetLookupWithLength();

	
	BenchMarkStringInsertionDifferentPointer();
	BenchMarkStringInsertionSamePointer();

	BenchMarkStringInsertionDifferentPointerWithLength();
	BenchMarkStringInsertionSamePointerWithLength();

#endif


	TestSimpleKeyStringNumberValue();
	TestSimpleKeyStringNumberValueWithIterator();
	TestValuePointerNULL();
	TestValueStringNULL();
	
	LuaHashMap_Free(hash_map);
	fprintf(stderr, "Program passed all tests!\n");
	return 0;
}
