//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
