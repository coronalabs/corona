//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
