
#include "lua_hash_map.hpp"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <string>


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

int DoKeyStringValueString()
{
	size_t ret_val;

	std::cerr << "create\n";
	lhm::lua_hash_map<const char*, const char*> hash_map;


	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, const char*>("key1", "value1"));
						
//	LuaHashMap_SetValueStringForKeyString(hash_map, "value1", "key1");
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, const char*>("key2", "value2"));

//	LuaHashMap_SetValueStringForKeyString(hash_map, "value2", "key2");
	std::cerr << "insert3\n";

//	LuaHashMap_SetValueStringForKeyString(hash_map, "value3", "key3");
//	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "key3");
	hash_map.insert(std::pair<const char*, const char*>("key3", "value3"));

	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	const char* ret_string = NULL;

	std::cerr << "bracket[]\n";

	ret_string = hash_map["key1"];
	assert(0 == Internal_safestrcmp("value1", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;

	std::cerr << "bracket[]\n";

	ret_string = hash_map["key2"];
	assert(0 == Internal_safestrcmp("value2", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	std::cerr << "bracket[]\n";

	ret_string = hash_map["key3"];
	assert(0 == Internal_safestrcmp("value3", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	
	//	hash_map["key3"] = "fee";
	//	ret_string = hash_map["key3"];
	// std::cerr << "ret_string=" << ret_string << std::endl;
	

#endif
	
//	lhm::lua_hash_map<const char*, void*>::iterator iter = hash_map.find("key3");

	lhm::lua_hash_map<const char*, const char*>::iterator iter;

	iter = hash_map.find("key1");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value1", (*iter).second));

	iter = hash_map.find("key2");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value2", (*iter).second));

	iter = hash_map.find("key3");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value3", (*iter).second));


	iter = hash_map.find("key3");
	std::pair<const char*, const char*> ret_pair = *iter;

	
	std::cerr << "erasing key3\n";
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;

	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase("key3");
	assert(0 == ret_val);


	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase("key2");
	assert(1 == ret_val);

	
	hash_map.insert(std::pair<const char*, const char*>("key2", "value2"));
	ret_val = hash_map.size();
	assert(2 == ret_val);

	hash_map.insert(std::pair<const char*, const char*>("key4", "value4"));
	ret_val = hash_map.size();
	assert(3 == ret_val);

	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;


	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);

	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;

		

	return 0;
}


static void* s_valuePointer1 = (void*)0x1;
static void* s_valuePointer2 = (void*)0x2;
static void* s_valuePointer3 = (void*)0x3;
static void* s_valuePointer4 = (void*)0x4;

int DoKeyStringValuePointer()
{
	size_t ret_val;
	
	std::cerr << "create\n";

	lhm::lua_hash_map<const char*, void*> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, void*>("key1", s_valuePointer1));
	
	//	LuaHashMap_SetValueStringForKeyString(hash_map, "value1", "key1");
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, void*>("key2", s_valuePointer2));
	
	//	LuaHashMap_SetValueStringForKeyString(hash_map, "value2", "key2");
	std::cerr << "insert3\n";
	
	//	LuaHashMap_SetValueStringForKeyString(hash_map, "value3", "key3");
	//	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "key3");
	hash_map.insert(std::pair<const char*, void*>("key3", s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;

	
	lhm::lua_hash_map<const char*, void*>::iterator iter;

	iter = hash_map.find("key1");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;

	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find("key2");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find("key3");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	

	iter = hash_map.find("key3");
	
	std::pair<const char*, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;

	
	
	
	std::cerr << "erasing key3\n";

	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase("key3");
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase("key2");
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<const char*, void*>("key2", s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<const char*, void*>("key4", s_valuePointer4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;

	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}

int DoKeyStringValueNumber()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<const char*, lua_Number> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, lua_Number>("key1", 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, lua_Number>("key2", 2.2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<const char*, lua_Number>("key3", 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	
	lhm::lua_hash_map<const char*, lua_Number>::iterator iter;
	
	iter = hash_map.find("key1");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find("key2");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find("key3");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	

	iter = hash_map.find("key3");
	
	std::pair<const char*, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase("key3");
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase("key2");
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<const char*, lua_Number>("key2", 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<const char*, lua_Number>("key4", 4.4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyStringValueInteger()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<const char*, lua_Integer> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, lua_Integer>("key1", 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, lua_Integer>("key2", 2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<const char*, lua_Integer>("key3", 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	
	lhm::lua_hash_map<const char*, lua_Integer>::iterator iter;
	
	iter = hash_map.find("key1");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find("key2");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find("key3");
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find("key3");
	
	std::pair<const char*, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase("key3");
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase("key2");
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<const char*, lua_Integer>("key2", 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<const char*, lua_Integer>("key4", 4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


static void* s_keyPointer1 = (void*)0x1;
static void* s_keyPointer2 = (void*)0x2;
static void* s_keyPointer3 = (void*)0x3;
static void* s_keyPointer4 = (void*)0x4;

int DoKeyPointerValuePointer()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, void*> hash_map;
		
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, void*>(s_keyPointer1, s_valuePointer1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, void*>(s_keyPointer2, s_valuePointer2));

	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, void*>(s_keyPointer3, s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, void*>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, void*>(s_keyPointer2, s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, void*>(s_keyPointer4, s_valuePointer4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}

struct SomeClass
{
	int someInt;
};


static SomeClass* s_keyPointerS1 = (SomeClass*)0x1;
static SomeClass* s_keyPointerS2 = (SomeClass*)0x2;
static SomeClass* s_keyPointerS3 = (SomeClass*)0x3;
static SomeClass* s_keyPointerS4 = (SomeClass*)0x4;

static SomeClass* s_valuePointerS1 = (SomeClass*)0x1;
static SomeClass* s_valuePointerS2 = (SomeClass*)0x2;
static SomeClass* s_valuePointerS3 = (SomeClass*)0x3;
static SomeClass* s_valuePointerS4 = (SomeClass*)0x4;


int DoKeyPointer2ValuePointer2()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<SomeClass*, SomeClass*> hash_map;
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS1, s_valuePointerS1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS2, s_valuePointerS2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS3, s_valuePointerS3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<SomeClass*, SomeClass*>::iterator iter;
	
	iter = hash_map.find(s_keyPointerS1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(s_keyPointerS2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(s_keyPointerS3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(s_keyPointerS3);
	
	std::pair<void*, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointerS3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointerS2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS2, s_valuePointerS2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<SomeClass*, SomeClass*>(s_keyPointerS4, s_valuePointerS4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyPointerValueString()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, const char*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer1, "key1"));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer2, "key2"));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer3, "key3"));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, const char*>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
    assert(0 == Internal_safestrcmp("key1", (*iter).second));

	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
    assert(0 == Internal_safestrcmp("key2", (*iter).second));

	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
    assert(0 == Internal_safestrcmp("key3", (*iter).second));

	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, const char*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer2, "key2"));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, const char*>(s_keyPointer4, "key4"));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyPointerValueNumber()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, lua_Number> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer1, 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer2, 2.2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer3, 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, lua_Number>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	
	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer2, 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, lua_Number>(s_keyPointer4, 4.4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyPointerValueInteger()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, lua_Integer> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer1, 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer2, 2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer3, 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, lua_Integer>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer2, 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, lua_Integer>(s_keyPointer4, 4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}







int DoKeyIntegerValuePointer()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, void*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, void*>(1, s_valuePointer1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, void*>(2, s_valuePointer2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, void*>(3, s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, void*>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, void*>(2, s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, void*>(4, s_valuePointer4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyIntegerValueString()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, const char*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, const char*>(1, "value1"));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, const char*>(2, "value2"));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, const char*>(3, "value3"));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, const char*>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(0 == Internal_safestrcmp("value1", (*iter).second));
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value2", (*iter).second));
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value3", (*iter).second));
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, const char*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, const char*>(2, "value2"));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, const char*>(4, "value4"));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}



int DoKeyIntegerValueInteger()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, lua_Integer> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(1, 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(2, 2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(3, 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, lua_Integer>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(2, 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, lua_Integer>(4, 4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}

int DoKeyIntegerValueNumber()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, lua_Number> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, lua_Number>(1, 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, lua_Number>(2, 2.2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, lua_Number>(3, 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, lua_Number>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, lua_Number>(2, 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, lua_Number>(4, 4.4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}

















int DoKeyNumberValuePointer()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, void*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, void*>(1.1, s_valuePointer1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, void*>(2.2, s_valuePointer2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, void*>(3.3, s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, void*>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, void*>(2.5, s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, void*>(4.4, s_valuePointer4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyNumberValueString()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, const char*> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, const char*>(1.1, "value1"));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, const char*>(2.2, "value2"));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, const char*>(3.3, "value3"));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, const char*>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(0 == Internal_safestrcmp("value1", (*iter).second));
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value2", (*iter).second));
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(0 == Internal_safestrcmp("value3", (*iter).second));
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, const char*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, const char*>(2.5, "value2"));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, const char*>(4.4, "value4"));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}



int DoKeyNumberValueInteger()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, lua_Integer> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, lua_Integer>(1.1, 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, lua_Integer>(2.2, 2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, lua_Integer>(3.3, 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, lua_Integer>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, lua_Integer>(2.5, 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, lua_Integer>(4.4, 4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}

int DoKeyNumberValueNumber()
{
	size_t ret_val;
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, lua_Number> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, lua_Number>(1.1, 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, lua_Number>(2.2, 2.2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, lua_Number>(3.3, 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, lua_Number>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, lua_Number>(2.5, 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, lua_Number>(4.4, 4.4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyStringCppValueStringCpp()
{
	size_t ret_val;

	std::string key1 = std::string("key1");
	std::string key2 = std::string("key2");
	std::string key3 = std::string("key3");
	std::string key4 = std::string("key4");

	std::string value1 = std::string("value1");
	std::string value2 = std::string("value2");
	std::string value3 = std::string("value3");
	std::string value4 = std::string("value4");
	
	std::cerr << "create\n";
	lhm::lua_hash_map<std::string, std::string> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<std::string, std::string>(key1, value1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<std::string, std::string>(key2, value2));

	std::cerr << "insert3\n";
	hash_map.insert(std::pair<std::string, std::string>(key3, value3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	const char* ret_string = NULL;
	
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key1"];
	assert(0 == Internal_safestrcmp("value1", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key2"];
	assert(0 == Internal_safestrcmp("value2", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key3"];
	assert(0 == Internal_safestrcmp("value3", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	
	//	hash_map["key3"] = "fee";
	//	ret_string = hash_map["key3"];
	// std::cerr << "ret_string=" << ret_string << std::endl;
	
	
#endif
	
	//	lhm::lua_hash_map<const char*, void*>::iterator iter = hash_map.find("key3");
	
	lhm::lua_hash_map<std::string, std::string>::iterator iter;
	
	iter = hash_map.find(key1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value1 == (*iter).second);
	
	iter = hash_map.find(key2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value2 == (*iter).second);
	
	iter = hash_map.find(key3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value3 == (*iter).second);
	
	
	iter = hash_map.find(key3);
	std::pair<std::string, std::string> ret_pair = *iter;
	
	
	std::cerr << "erasing key3\n";
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(key3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(key2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<std::string, std::string>(key2, value2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<std::string, std::string>(key4, value4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyStringCppValueString()
{
	size_t ret_val;
	
	std::string key1 = std::string("key1");
	std::string key2 = std::string("key2");
	std::string key3 = std::string("key3");
	std::string key4 = std::string("key4");
	
	std::string value1 = std::string("value1");
	std::string value2 = std::string("value2");
	std::string value3 = std::string("value3");
	std::string value4 = std::string("value4");
	
	std::cerr << "create\n";
	lhm::lua_hash_map<std::string, const char*> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<std::string, const char*>(key1, value1.c_str()));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<std::string, const char*>(key2, value2.c_str()));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<std::string, const char*>(key3, value3.c_str()));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	const char* ret_string = NULL;
	
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key1"];
	assert(0 == Internal_safestrcmp("value1", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key2"];
	assert(0 == Internal_safestrcmp("value2", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key3"];
	assert(0 == Internal_safestrcmp("value3", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	
	//	hash_map["key3"] = "fee";
	//	ret_string = hash_map["key3"];
	// std::cerr << "ret_string=" << ret_string << std::endl;
	
	
#endif
	
	//	lhm::lua_hash_map<const char*, void*>::iterator iter = hash_map.find("key3");
	
	lhm::lua_hash_map<std::string, const char*>::iterator iter;
	
	iter = hash_map.find(key1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value1 == std::string((*iter).second));
	
	iter = hash_map.find(key2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value2 == std::string((*iter).second));
	
	iter = hash_map.find(key3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value3 == std::string((*iter).second));
	
	
	iter = hash_map.find(key3);
	std::pair<std::string, const char*> ret_pair = *iter;
	
	
	std::cerr << "erasing key3\n";
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(key3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(key2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<std::string, const char*>(key2, value2.c_str()));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<std::string, const char*>(key4, value4.c_str()));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}

int DoKeyStringValueStringCpp()
{
	size_t ret_val;
	
	std::string key1 = std::string("key1");
	std::string key2 = std::string("key2");
	std::string key3 = std::string("key3");
	std::string key4 = std::string("key4");
	
	std::string value1 = std::string("value1");
	std::string value2 = std::string("value2");
	std::string value3 = std::string("value3");
	std::string value4 = std::string("value4");
	
	std::cerr << "create\n";
	lhm::lua_hash_map<const char*, std::string> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<const char*, std::string>(key1.c_str(), value1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<const char*, std::string>(key2.c_str(), value2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<const char*, std::string>(key3.c_str(), value3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
#ifdef LUAHASHMAPCPP_USE_BRACKET_OPERATOR
	const char* ret_string = NULL;
	
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key1"];
	assert(0 == Internal_safestrcmp("value1", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key2"];
	assert(0 == Internal_safestrcmp("value2", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	std::cerr << "bracket[]\n";
	
	ret_string = hash_map["key3"];
	assert(0 == Internal_safestrcmp("value3", ret_string));
	std::cerr << "ret_string=" << ret_string << std::endl;
	
	//	hash_map["key3"] = "fee";
	//	ret_string = hash_map["key3"];
	// std::cerr << "ret_string=" << ret_string << std::endl;
	
	
#endif
	
	//	lhm::lua_hash_map<const char*, void*>::iterator iter = hash_map.find("key3");
	
	lhm::lua_hash_map<const char*, std::string>::iterator iter;
	
	iter = hash_map.find(key1.c_str());
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value1 == (*iter).second);
	
	iter = hash_map.find(key2.c_str());
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value2 == (*iter).second);
	
	iter = hash_map.find(key3.c_str());
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value3 == (*iter).second);
	
	
	iter = hash_map.find(key3.c_str());
	std::pair<const char*, std::string> ret_pair = *iter;
	
	
	std::cerr << "erasing key3\n";
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(key3.c_str());
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(key2.c_str());
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<const char*, std::string>(key2.c_str(), value2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<const char*, std::string>(key4.c_str(), value4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}



int DoKeyStringCppValuePointer()
{
	size_t ret_val;
	std::string key1 = std::string("key1");
	std::string key2 = std::string("key2");
	std::string key3 = std::string("key3");
	std::string key4 = std::string("key4");
		std::cerr << "create\n";
	
	lhm::lua_hash_map<std::string, void*> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<std::string, void*>(key1, s_valuePointer1));
	
	//	LuaHashMap_SetValueStringForKeyString(hash_map, "value1", "key1");
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<std::string, void*>(key2, s_valuePointer2));
	
	//	LuaHashMap_SetValueStringForKeyString(hash_map, "value2", "key2");
	std::cerr << "insert3\n";
	
	//	LuaHashMap_SetValueStringForKeyString(hash_map, "value3", "key3");
	//	LuaHashMap_SetValueStringForKeyString(hash_map, NULL, "key3");
	hash_map.insert(std::pair<std::string, void*>(key3, s_valuePointer3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<std::string, void*>::iterator iter;
	
	iter = hash_map.find(key1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert((void*)0x1 == (void*)(*iter).second);
	
	iter = hash_map.find(key2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x2 == (void*)(*iter).second);
	
	iter = hash_map.find(key3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert((void*)0x3 == (void*)(*iter).second);
	
	
	iter = hash_map.find(key3);
	
	std::pair<std::string, void*> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(key3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(key2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<std::string, void*>(key2, s_valuePointer2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<std::string, void*>(key4, s_valuePointer4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}



int DoKeyStringCppValueNumber()
{
	size_t ret_val;
	std::string key1 = std::string("key1");
	std::string key2 = std::string("key2");
	std::string key3 = std::string("key3");
	std::string key4 = std::string("key4");
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<std::string, lua_Number> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<std::string, lua_Number>(key1, 1.1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<std::string, lua_Number>(key2, 2.2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<std::string, lua_Number>(key3, 3.3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	
	lhm::lua_hash_map<std::string, lua_Number>::iterator iter;
	
	iter = hash_map.find(key1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1.1 == (*iter).second);
	
	iter = hash_map.find(key2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2.2 == (*iter).second);
	
	iter = hash_map.find(key3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3.3 == (*iter).second);
	
	
	iter = hash_map.find(key3);
	
	std::pair<std::string, lua_Number> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(key3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(key2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<std::string, lua_Number>(key2, 2.5));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<std::string, lua_Number>(key4, 4.4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyStringCppValueInteger()
{
	size_t ret_val;
	std::string key1 = std::string("key1");
	std::string key2 = std::string("key2");
	std::string key3 = std::string("key3");
	std::string key4 = std::string("key4");
	
	std::cerr << "create\n";
	
	lhm::lua_hash_map<std::string, lua_Integer> hash_map;
	
	
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<std::string, lua_Integer>(key1, 1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<std::string, lua_Integer>(key2, 2));
	
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<std::string, lua_Integer>(key3, 3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	
	lhm::lua_hash_map<std::string, lua_Integer>::iterator iter;
	
	iter = hash_map.find(key1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(1 == (*iter).second);
	
	iter = hash_map.find(key2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(2 == (*iter).second);
	
	iter = hash_map.find(key3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(3 == (*iter).second);
	
	
	iter = hash_map.find("key3");
	
	std::pair<std::string, lua_Integer> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(key3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(key2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<std::string, lua_Integer>(key2, 2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<std::string, lua_Integer>(key4, 4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}



int DoKeyPointerValueStringCpp()
{
	size_t ret_val;

	std::string key1 = std::string("key1");
	std::string key2 = std::string("key2");
	std::string key3 = std::string("key3");
	std::string key4 = std::string("key4");

	std::cerr << "create\n";
	
	lhm::lua_hash_map<void*, std::string> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<void*, std::string>(s_keyPointer1, key1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<void*, std::string>(s_keyPointer2, key2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<void*, std::string>(s_keyPointer3, key3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<void*, std::string>::iterator iter;
	
	iter = hash_map.find(s_keyPointer1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
    assert(key1 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
    assert(key2 == (*iter).second);
	
	iter = hash_map.find(s_keyPointer3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
    assert(key3 == (*iter).second);
	
	
	iter = hash_map.find(s_keyPointer3);
	
	std::pair<void*, std::string> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(s_keyPointer3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(s_keyPointer2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<void*, std::string>(s_keyPointer2, key2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<void*, std::string>(s_keyPointer4, key4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}



int DoKeyIntegerValueStringCpp()
{
	size_t ret_val;
	
	std::string value1 = std::string("value1");
	std::string value2 = std::string("value2");
	std::string value3 = std::string("value3");
	std::string value4 = std::string("value4");
	

	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Integer, std::string> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Integer, std::string>(1, value1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Integer, std::string>(2, value2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Integer, std::string>(3, value3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Integer, std::string>::iterator iter;
	
	iter = hash_map.find(1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(value1 == (*iter).second);
	
	iter = hash_map.find(2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value2 == (*iter).second);
	
	iter = hash_map.find(3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value3 == (*iter).second);
	
	
	iter = hash_map.find(3);
	
	std::pair<lua_Integer, std::string> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Integer, std::string>(2, value2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Integer, std::string>(4, value4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}


int DoKeyNumberValueStringCpp()
{
	size_t ret_val;
	
	std::string value1 = std::string("value1");
	std::string value2 = std::string("value2");
	std::string value3 = std::string("value3");
	std::string value4 = std::string("value4");
	

	std::cerr << "create\n";
	
	lhm::lua_hash_map<lua_Number, std::string> hash_map;
    
	std::cerr << "insert1\n";
	hash_map.insert(std::pair<lua_Number, std::string>(1.1, value1));
	
	std::cerr << "insert2\n";
	hash_map.insert(std::pair<lua_Number, std::string>(2.2, value2));
    
	std::cerr << "insert3\n";
	hash_map.insert(std::pair<lua_Number, std::string>(3.3, value3));
	
	
	
	ret_val = hash_map.size();
	assert(3 == ret_val);
	std::cerr << "size=" << ret_val << std::endl;
	
	
	lhm::lua_hash_map<lua_Number, std::string>::iterator iter;
	
	iter = hash_map.find(1.1);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	
	assert(value1 == (*iter).second);
	
	iter = hash_map.find(2.2);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value2 == (*iter).second);
	
	iter = hash_map.find(3.3);
	std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	assert(value3 == (*iter).second);
	
	
	iter = hash_map.find(3.3);
	
	std::pair<lua_Number, std::string> ret_pair = *iter;
	std::cerr << "*iter (pair)=" << ret_pair.first << ", " << ret_pair.second << std::endl;
	
	
	
	
	std::cerr << "erasing key3\n";
	
	ret_val = hash_map.erase(iter);
	assert(1 == ret_val);
	
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	std::cerr << "size=" << ret_val << std::endl;
	
	std::cerr << "erasing key3 again\n";
	ret_val = hash_map.erase(3.3);
	assert(0 == ret_val);
	
	
	std::cerr << "erasing key2\n";
	ret_val = hash_map.erase(2.2);
	assert(1 == ret_val);
	
	
	hash_map.insert(std::pair<lua_Number, std::string>(2.5, value2));
	ret_val = hash_map.size();
	assert(2 == ret_val);
	
	hash_map.insert(std::pair<lua_Number, std::string>(4.4, value4));
	ret_val = hash_map.size();
	assert(3 == ret_val);
	
	for(iter=hash_map.begin(); iter!=hash_map.end(); ++iter)
	{
		std::cerr << "*iter (pair)=" << (*iter).first << ", " << (*iter).second << std::endl;
	}
	
	
	assert(false == hash_map.empty());
	std::cerr << "IsEmpty should be no: " << hash_map.empty() << std::endl;
	
	
	hash_map.clear();
	ret_val = hash_map.size();
	assert(0 == ret_val);
	
	assert(true == hash_map.empty());
	std::cerr << "IsEmpty should be yes: " << hash_map.empty() << std::endl;
	
	
	
	return 0;
}




int main(int argc, char* argv[])
{
	DoKeyStringValueString();
	DoKeyStringValuePointer();
	DoKeyStringValueNumber();
	DoKeyStringValueInteger();
    
	DoKeyPointerValuePointer();
	DoKeyPointer2ValuePointer2();
    DoKeyPointerValueString();
    DoKeyPointerValueNumber();
    DoKeyPointerValueInteger();
    
    DoKeyIntegerValuePointer();
	DoKeyIntegerValueString();
	DoKeyIntegerValueInteger();
	DoKeyIntegerValueNumber();
	
	DoKeyNumberValuePointer();
	DoKeyNumberValueString();
	DoKeyNumberValueInteger();
	DoKeyNumberValueNumber();
	
	
	
	DoKeyStringCppValueStringCpp();
	DoKeyStringCppValueString();
	DoKeyStringValueStringCpp();

	DoKeyStringCppValuePointer();
	DoKeyStringCppValueNumber();
	DoKeyStringCppValueInteger();

	DoKeyPointerValueStringCpp();
	DoKeyIntegerValueStringCpp();
	DoKeyNumberValueStringCpp();

	
	fprintf(stderr, "Program passed all tests!\n");
    return 0;
}
