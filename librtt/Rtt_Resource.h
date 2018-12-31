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
