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


#ifndef _AndroidDisplayObjectRegistry_H__
#define _AndroidDisplayObjectRegistry_H__


// Set up forward declarations.
extern "C"
{
	struct LuaHashMap;
}
namespace Rtt
{
	class AndroidDisplayObject;
}


/// Stores a collection of AndroidDisplayObject instances for tracking purposes.
/// The intent is to have AndroidDisplayObject instances be registered into this collection when
/// they are created and unregistered from this collection when they are deleted. This registry
/// will automatically assign a unique ID to the object which is to be used by its associated
/// Java UI object to pass data back to the C++ display object.
class AndroidDisplayObjectRegistry
{
	public:
		AndroidDisplayObjectRegistry();
		virtual ~AndroidDisplayObjectRegistry();

		int Register(Rtt::AndroidDisplayObject *objectPointer);
		void Unregister(int objectId);
		bool ContainsId(int objectId);
		Rtt::AndroidDisplayObject* GetById(int objectId);

		enum
		{
			/// Value used to indicate an invalid object Id.
			INVALID_ID = 0
		};

	private:
		LuaHashMap *fLuaHashMapPointer;
};

#endif // _AndroidDisplayObjectRegistry_H__
