//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
