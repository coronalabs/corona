//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"
#include "Rtt_LuaContainer.h"
#include "Rtt_LuaData.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include <sstream>
#include <algorithm>


namespace Rtt
{

//Required for deleting
struct LuaDataDeleter
{
    void operator()(LuaData* obj)
	{
		Rtt_DELETE(obj);
	}
};

//Vector container
	LuaVector::LuaVector()
	{
		SetDataType(kVector);
	}
	LuaVector::~LuaVector()
	{
		std::for_each(fVector.begin(), fVector.end(), LuaDataDeleter());
		
		fVector.clear();
	}
	
	void* LuaVector::GetData()
	{
		return static_cast<void*>(&fVector);
	}
	
	void LuaVector::AddData(LuaData *data)
	{
		fVector.push_back(data);
	}
	LuaData *LuaVector::GetData(int index)
	{
		return fVector[index];
	}
	
	LuaData *LuaVector::GetCopy()
	{
		LuaVector *result = new LuaVector();
		
		return result;
	}
	
	
//Map container
	LuaMap::LuaMap()
	{
		SetDataType(kMap);
	}
	LuaMap::LuaMap(lua_State *L, int t)
	{

		SetDataType(kMap);
		
		if (t < 0)
		{
			t = lua_gettop( L ) + (t + 1);
		}
		
		for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1))
		{
			std::string key;
			int luaKeyType = lua_type(L, -2);
			if (LUA_TNUMBER == luaKeyType)
			{
				std::stringstream ss;
				ss << lua_tonumber( L, -2 );
				key.assign(ss.str());
			}
			else if (LUA_TSTRING == luaKeyType)
			{
				key.assign(lua_tostring(L, -2));
			}
			else
			{
				continue;
			}
			
			int vType = lua_type( L, -1 );
			if ( LUA_TSTRING == vType )
			{
				const char *v = lua_tostring( L, -1 );
				if ( (key.length() > 0) && v )
				{
					std::string val = std::string(v);
					//LuaData *dataVal = new LuaString(val);
					LuaData *dataVal = Rtt_NEW( LuaContext::GetRuntime( L )->GetAllocator(),LuaString(val));
					
					SetData(key,dataVal);
				}
			}
			else if ( LUA_TBOOLEAN == vType )
			{
				int b = lua_toboolean( L, -1 );
				//LuaData *dataVal = new LuaBool(b);
				LuaData *dataVal = Rtt_NEW( LuaContext::GetRuntime( L )->GetAllocator(),LuaBool(b));
				SetData(key,dataVal);
				
			}
			else if ( LUA_TNUMBER == vType )
			{
				
				double d = lua_tonumber( L, -1 );
				//LuaData *dataVal = new LuaDouble(d);
				LuaData *dataVal = Rtt_NEW( LuaContext::GetRuntime( L )->GetAllocator(),LuaDouble(d));
				SetData(key,dataVal);
				
			}
			else if ( LUA_TTABLE == vType )
			{
				//LuaData *dataVal = new LuaMap(L,-1);//lua_gettop( L ));
				LuaData *dataVal = Rtt_NEW( LuaContext::GetRuntime( L )->GetAllocator(),LuaMap(L,-1));
				
				if ( dataVal )
				{
					SetData(key,dataVal);
				}
			}
			else if (vType == LUA_TFUNCTION)
			{
				Rtt_ASSERT_NOT_IMPLEMENTED();
			
//				Needs an event name to key off of
//				if ( CoronaLuaIsListener( L, -1, "zipevent" ) )
//				{
//					CoronaLuaRef fListener	= CoronaLuaNewRef( L, -1 );
//					//LuaData *listener = new LuaListener(fListener);//lua_gettop( L ));
//					LuaData *listener = Rtt_NEW( LuaContext::GetRuntime( L )->GetAllocator(),LuaListener(fListener));
//					SetData(key,listener);
//				}
			}
			else if (vType == LUA_TLIGHTUSERDATA)
			{
				void* lud = lua_touserdata( L, -1 );
				
				//LuaUserData *dataVal = new LuaUserData(lud);
				LuaUserData *dataVal = Rtt_NEW( LuaContext::GetRuntime( L )->GetAllocator(),LuaUserData(lud));
				SetData(key,dataVal);
			}
			else
			{
				Rtt_ASSERT_NOT_IMPLEMENTED();
			}
		}
	}
	
	LuaMap::~LuaMap()
	{
		
		for(std::map<std::string,LuaData*>::iterator it = fDataMap.begin(); it != fDataMap.end(); ++it)
		{
			Rtt_DELETE(it->second);
		}
		fDataMap.clear();
		
		
	}
	void* LuaMap::GetData()
	{
		return static_cast<void*>(&fDataMap);
	}
	
	void LuaMap::SetData(std::string key, LuaData *data)
	{
		fDataMap[key] = data;
	}
	bool LuaMap::HasKey(std::string key)
	{
		return false;
	}
	LuaData *LuaMap::GetData(std::string key)
	{
		return fDataMap[key];
	}
	
	LuaData *LuaMap::GetCopy()
	{
		LuaMap *result = new LuaMap();
		
		std::vector<std::string> keyList = GetKeys();
		
		for(std::map<std::string,LuaData*>::iterator it = fDataMap.begin(); it != fDataMap.end(); ++it)
		{
			LuaData *dataCopy = (it->second)->GetCopy();
			
			result->SetData(it->first, dataCopy);
		}
		
		return result;
	}
	
	std::vector<std::string> LuaMap::GetKeys()
	{
		std::vector<std::string> result;
		
		for(std::map<std::string,LuaData*>::iterator it = fDataMap.begin(); it != fDataMap.end(); ++it)
		{
			result.push_back(it->first);
		}
		
		return result;
	}
	
	
//String Vector helper container for loading in table of "filenames" in plugin
//
//	LuaStringVector::LuaStringVector()
//	{
//	}
//	
//	LuaStringVector::LuaStringVector(lua_State *L, int t)
//	{
//		SetDataType( kVector );
//		
//		lua_getfield( L, framesIndex, "filenames" );
//		{
//			int framesIndex = t;//lua_gettop( L );
//
//			if( lua_istable( L, -1 ) )
//			{
//				for( int i = 0, iMax = lua_objlen( L, framesIndex ); i < iMax; i++ )
//				{
//					int index = ( i + 1 ); // Lua is 1-based so (i+1)th frame.
//					lua_rawgeti( L, framesIndex, index );
//					{
//						if( ! lua_isnil( L, -1 ) )
//						{
//							const char *val = lua_tostring( L, -1 );
//							std::string strVal = std::string( val );
//							fVec.push_back( strVal );
//						}
//					}
//					lua_pop( L, 1 );
//				}
//			}
//		}
//		lua_pop( L, 1 );
//	}
//	LuaData *LuaStringVector::GetCopy()
//	{
//		return NULL;
//	}
//	void* LuaStringVector::GetData()
//	{
//		return NULL;//return static_cast<void*>(&fDataMap);
//	}
//	int LuaStringVector::GetCount()
//	{
//		return fVec.size();
//	}
//	std::string LuaStringVector::GetVal(int index)
//	{
//		return fVec[index];
//	}
//	void LuaStringVector::SetVal(int index, std::string val)
//	{
//		fVec[index] = val;
//	}
//	void LuaStringVector::Push(std::string val)
//	{
//		fVec.push_back(val);
//	}
	
	
	
	
};

