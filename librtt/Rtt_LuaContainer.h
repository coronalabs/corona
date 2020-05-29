//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __Plugin__LuaMap__
#define __Plugin__LuaMap__

#include "Rtt_LuaData.h"
#include "CoronaLua.h"
#include <map>
#include <vector>
#include <string>


namespace Rtt
{
	
	
	//This set of LuaData wrappers/containers is designed to be a cross platform
	//container using stl, that allows for a normalized means of loading nested data
	//from within lua tables, as well as provides copy's of data for marshalling purposes
	//User's should be aware of the ownership rules when using this set of wrappers
	
	
	class LuaVector : public LuaData
	{
	public:
		LuaVector();
		virtual ~LuaVector();
		
	public:
		virtual void* GetData();
		void AddData(LuaData *data);
		virtual LuaData *GetCopy();
		LuaData *GetData(int index);
	private:
		std::vector<LuaData*> fVector;
		
	};
	
// ----------------------------------------------------------------------------
	class LuaMap : public LuaData
	{
	public:
		LuaMap();
		LuaMap(lua_State *L, int t);
		virtual ~LuaMap();
		
	public:
		virtual void* GetData();
		void SetData(std::string key, LuaData *data);
		bool HasKey(std::string key);
		LuaData *GetData(std::string key);
		virtual LuaData *GetCopy();
		std::vector<std::string> GetKeys();
		
	private:
		std::map<std::string,LuaData*> fDataMap;
		
	};
	
	
//	Reader to strings types, the LuaVector is the generic implementation
//	class LuaStringVector : public LuaData
//	{
//	public:
//		LuaStringVector();
//		LuaStringVector(lua_State *L, int t);
//		
//	public:
//		virtual LuaData *GetCopy();
//		virtual void* GetData();
//		int GetCount();
//		std::string GetVal(int index);
//		void SetVal(int index, std::string val);
//		void Push(std::string val);
//	private:
//		std::vector<std::string> fVec;
//	};
	
}

#endif /* defined(__Plugin__LuaMap__) */
