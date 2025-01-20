//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Resource_H__
#define _Rtt_Resource_H__

// #include "Core/Rtt_Types.h"
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class CachedResource;
class Resource;

// ----------------------------------------------------------------------------

class Resource
{
	protected:
		Resource();

		// Resource deletes itself in Release() so don't make this public
		virtual ~Resource();

	public:
		// Let's see if we can get away with these *not* being virtual
		void Retain() 
		{ 
			++fCount; 
		}
		virtual void Release();

	private:
		S32 fCount;
};

// ----------------------------------------------------------------------------

class MCachedResourceLibrary
{
	public:
		virtual CachedResource* LookupResource( const char key[] ) = 0;
		virtual void SetResource( CachedResource* resource, const char key[] ) = 0;
		virtual Rtt_Allocator* GetAllocator() const = 0;
		virtual void MarkRecentlyUsed( CachedResource& resource ) = 0;
};

class CachedResource : public Resource
{
	public:
		typedef Resource Super;
		typedef CachedResource Self;

	public:
		// Pass in NULL for key if the resource shouldn't be added to the library
		CachedResource( MCachedResourceLibrary& library, const char* key );

	protected:
		virtual ~CachedResource();

	protected:
		void RegisterResource( const char* key );

		const MCachedResourceLibrary& GetLibrary() const { return fLibrary; }
		MCachedResourceLibrary& GetLibrary() { return fLibrary; }
//		const char* Key() const { return fKey; }

	public:
		Self* Next() const { return fNext; }
		Self* Prev() const { return fPrev; }

		void Remove();
		void Append( Self* newValue );
		void Prepend( Self* newValue );

	private:
		MCachedResourceLibrary& fLibrary;
		String fKey;
		Self* fNext;
		Self* fPrev;
};

class CachedResourceIterator
{
		CachedResource * fCurrent;
		CachedResource * fStop;
	public:
		CachedResourceIterator( CachedResource * c )
		{
			fStop = c;
			fCurrent = c->Next();
		}
		bool More() const {
			return (fCurrent != fStop) && (fCurrent->Next() != fCurrent->Prev());
		}
		CachedResource * Next()
		{
			CachedResource * result = fCurrent;
			fCurrent = fCurrent->Next();
			return result;
		}
};
	
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Resource_H__
