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


#ifndef __Rtt_LuaData__
#define __Rtt_LuaData__

#include <string>
#include "CoronaLua.h"


namespace Rtt
{
	//This set of LuaData wrappers/containers is designed to be a cross platform
	//container using stl, that allows for a normalized means of loading nested data
	//from within lua tables, as well as provides copy's of data for marshalling purposes
	//User's should be aware of the ownership rules when using this set of wrappers
	
	class LuaData
	{
	public:
		typedef LuaData Self;
		
	public:
		typedef enum LuaType
		{
			kDouble = 0,
			kString = 1,
			kMap	= 2,
			kVector	= 3,
			kListener = 4,
			kLUD	= 5,
			kBoolean = 6,
			
			kNumTypes
		}
		LuaType;
		
	public:
	
		LuaData();
		virtual ~LuaData();

		virtual void* GetData() = 0;
		virtual LuaData *GetCopy() = 0;
		void SetDataType( LuaType t );
		LuaType GetDataType();
		
	protected:
		LuaType fDataType;
		
	};

// ----------------------------------------------------------------------------
	class LuaDouble : public LuaData
	{
	public:
		LuaDouble(double val);
		virtual void* GetData();
		virtual LuaData *GetCopy();
	private:
		double fData;
		
	};
	
// ----------------------------------------------------------------------------	
	class LuaString : public LuaData
	{
	
	public:
		LuaString(std::string val);
		virtual ~LuaString();
		virtual void* GetData();
		virtual LuaData *GetCopy();
		std::string GetString();
		
	private:
		std::string fData;
	};

// ----------------------------------------------------------------------------	
	class LuaBool : public LuaData
	{
	
	public:
		LuaBool(int val);
		virtual void* GetData();
		virtual LuaData *GetCopy();
		bool GetBool();
		
	private:
		bool fData;
		
	};
	
	class LuaListener : public LuaData
	{
	
	public:
		LuaListener(CoronaLuaRef lListener);
		virtual ~LuaListener();
		virtual void* GetData();
		virtual LuaData *GetCopy();
		CoronaLuaRef GetListener();
		
	private:
		CoronaLuaRef fListener;
		
	};

// ----------------------------------------------------------------------------	
	//LuaUserData is a simple wrapper around a userData pointer
	//The fData pointer is not owned by this, so it must be freed externally
	//GetCopy returns a new wrapper around the pointer
	class LuaUserData : public LuaData
	{
	
	public:
		LuaUserData(void* val);
		virtual ~LuaUserData();
		virtual void* GetData();
		virtual LuaData *GetCopy();
		
	private:
		void* fData;
		
	};
	
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


#endif /* defined(__Rtt_LuaData__) */
