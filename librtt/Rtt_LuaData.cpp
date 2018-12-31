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


#include <iostream>
#include "Rtt_LuaData.h"


namespace Rtt
{
	
	LuaData::LuaData()
	:fDataType(LuaData::kDouble)
	{
		
	}
	
	LuaData::~LuaData()
	{
	}
	void LuaData::SetDataType( LuaType t )
	{
		fDataType = t;
	}
	LuaData::LuaType LuaData::GetDataType()
	{
		return fDataType;
	}
	
	
	
// ----------------------------------------------------------------------------	
	LuaUserData::LuaUserData(void* val)
	{
		fData = val;
		SetDataType(kLUD);
	}
	LuaUserData::~LuaUserData()
	{
		//Doesn't own, owner must delete this
		fData = NULL;
	}
	
	void* LuaUserData::GetData()
	{
		return static_cast<void*>(fData);
	}
	LuaData *LuaUserData::GetCopy()
	{
		//Not a real copy, just a wrapper around the pointer
		return new LuaUserData(fData);
	}
	
	
// ----------------------------------------------------------------------------	
	LuaDouble::LuaDouble(double val)
	{
		fData = val;
		SetDataType(kDouble);
	}
	void* LuaDouble::GetData()
	{
		return static_cast<void*>(&fData);
	}
	LuaData *LuaDouble::GetCopy()
	{
		return new LuaDouble(fData);
	}
	
// ----------------------------------------------------------------------------	
	LuaString::LuaString(std::string val)
	{
		fData = val;
		SetDataType(kString);
	}
	LuaString::~LuaString()
	{
		
	}
	void* LuaString::GetData()
	{
		return static_cast<void*>(&fData);
	}
	LuaData *LuaString::GetCopy()
	{
		return new LuaString(fData);
	}
	std::string LuaString::GetString()
	{
		return fData;
	}
	
	
// ----------------------------------------------------------------------------	
	LuaBool::LuaBool(int val)
	{
		fData = (val == 0 ? 0 : 1);
		SetDataType(kBoolean);
	}
	void* LuaBool::GetData()
	{
		return static_cast<void*>(&fData);
	}
	LuaData *LuaBool::GetCopy()
	{
		return new LuaBool(fData);
	}
	bool LuaBool::GetBool()
	{
		return fData;
	}
	
	
// ----------------------------------------------------------------------------	
	LuaListener::LuaListener(CoronaLuaRef lListener)
	{
		fListener = lListener;
		SetDataType(kListener);
	}
	LuaListener::~LuaListener()
	{
		//Doesn't delete
		fListener = 0;
	}
	void* LuaListener::GetData()
	{
		return static_cast<void*>(&fListener);
	}
	LuaData *LuaListener::GetCopy()
	{
		return new LuaListener(fListener);
	}
	
	CoronaLuaRef LuaListener::GetListener()
	{
		return fListener;
	}
	
	
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
