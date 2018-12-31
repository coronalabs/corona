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


#include "Core/Rtt_Build.h"
#include "AndroidDisplayObjectRegistry.h"
#include "Rtt_PlatformDisplayObject.h"
#include "LuaHashMap.h"


/// Creates a new AndroidDisplayObject registry.
AndroidDisplayObjectRegistry::AndroidDisplayObjectRegistry()
{
	fLuaHashMapPointer = LuaHashMap_Create();
}

/// Destroys the AndroidDisplayObject registry.
AndroidDisplayObjectRegistry::~AndroidDisplayObjectRegistry()
{
	if (fLuaHashMapPointer)
	{
		LuaHashMap_Free(fLuaHashMapPointer);
	}
}

/// Adds the given display object to the registry.
/// @param objectPointer Pointer to the display object to be added to the registry. Cannot be NULL.
/// @return Returns a unique ID to be used to access this given object from the registry.
///         Returns AndroidDisplayObjectRegistry::INVALID_ID if given a NULL pointer.
int AndroidDisplayObjectRegistry::Register(Rtt::AndroidDisplayObject *objectPointer)
{
	// Validate.
	if (!fLuaHashMapPointer || !objectPointer)
	{
		return AndroidDisplayObjectRegistry::INVALID_ID;
	}
	
	// Generate a unique ID for the given object.
	static int sNextId = 0;
	for (sNextId++; (AndroidDisplayObjectRegistry::INVALID_ID == sNextId) || ContainsId(sNextId); sNextId++);
	
	// Add the object to the hash map.
	LuaHashMap_SetValuePointerForKeyInteger(fLuaHashMapPointer, objectPointer, (lua_Integer)sNextId);
	
	// Return the unique ID to the object in the hash map.
	return sNextId;
}

/// Removes a display object from the registry by its unique ID.
/// @param The object's unique ID assigned to it via the Register() function.
void AndroidDisplayObjectRegistry::Unregister(int objectId)
{
	// Validate.
	if (!fLuaHashMapPointer || (AndroidDisplayObjectRegistry::INVALID_ID == objectId))
	{
		return;
	}
	
	// Remove the object from the hash map.
	LuaHashMap_RemoveKeyInteger(fLuaHashMapPointer, (lua_Integer)objectId);
}

/// Determines if the given object ID exists in the registry.
/// @param objectId The unique ID of the object to look for.
/// @return Returns true if the object ID was found. Returns false if not.
bool AndroidDisplayObjectRegistry::ContainsId(int objectId)
{
	// Validate.
	if (!fLuaHashMapPointer || (AndroidDisplayObjectRegistry::INVALID_ID == objectId))
	{
		return false;
	}
	
	// Check if the given ID exists in the hash map.
	return LuaHashMap_ExistsKeyInteger(fLuaHashMapPointer, objectId) ? true : false;
}

/// Fetches a display object from the registry by its unique ID.
/// @param objectId The unique ID of the object that was assigned via the Register() function.
/// @return Returns a pointer to the object if found. Returns NULL if not found.
Rtt::AndroidDisplayObject* AndroidDisplayObjectRegistry::GetById(int objectId)
{
	// Validate.
	if (!fLuaHashMapPointer || (AndroidDisplayObjectRegistry::INVALID_ID == objectId))
	{
		return NULL;
	}
	
	// Fetch the specified object.
	return (Rtt::AndroidDisplayObject*)LuaHashMap_GetValuePointerForKeyInteger(fLuaHashMapPointer, (lua_Number)objectId);
}
