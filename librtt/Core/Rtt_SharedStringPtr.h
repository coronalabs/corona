//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_SharedStringPtr_H__
#define __Rtt_SharedStringPtr_H__

#include "Rtt_Allocator.h"
#include "Rtt_Macros.h"
#include "Rtt_SharedPtr.h"
#include "Rtt_String.h"
#include <string>


// ----------------------------------------------------------------------------
// Shared Pointer Type Definitions
// ----------------------------------------------------------------------------

namespace Rtt
{
	typedef Rtt::SharedPtr<Rtt::String> SharedStringPtr;
	typedef Rtt::SharedPtr<const Rtt::String> SharedConstStringPtr;
	typedef Rtt::SharedPtr<std::string> SharedStdStringPtr;
	typedef Rtt::SharedPtr<const std::string> SharedConstStdStringPtr;
}


// ----------------------------------------------------------------------------
// Rtt::SharedPtr<Rtt::String> Creating Functions
// ----------------------------------------------------------------------------

Rtt_INLINE Rtt::SharedStringPtr Rtt_MakeSharedStringPtr()
{
	return Rtt::SharedStringPtr::ForCppNewCreatedPtr(new Rtt::String);
}

Rtt_INLINE Rtt::SharedStringPtr Rtt_MakeSharedStringPtr(const char* stringPointer)
{
	return Rtt::SharedStringPtr::ForCppNewCreatedPtr(new Rtt::String(stringPointer));
}

Rtt_INLINE Rtt::SharedStringPtr Rtt_MakeSharedStringPtr(Rtt_Allocator* allocatorPointer)
{
	return Rtt::SharedStringPtr(Rtt_NEW(allocatorPointer, Rtt::String(allocatorPointer)));
}

Rtt_INLINE Rtt::SharedStringPtr Rtt_MakeSharedStringPtr(Rtt_Allocator* allocatorPointer, const char* stringPointer)
{
	return Rtt::SharedStringPtr(Rtt_NEW(allocatorPointer, Rtt::String(allocatorPointer, stringPointer)));
}


// ----------------------------------------------------------------------------
// Rtt::SharedPtr<const Rtt::String> Creating Functions
// ----------------------------------------------------------------------------

Rtt_INLINE Rtt::SharedConstStringPtr Rtt_MakeSharedConstStringPtr()
{
	return Rtt::SharedConstStringPtr::ForCppNewCreatedPtr(new Rtt::String);
}

Rtt_INLINE Rtt::SharedConstStringPtr Rtt_MakeSharedConstStringPtr(const char* stringPointer)
{
	return Rtt::SharedConstStringPtr::ForCppNewCreatedPtr(new Rtt::String(stringPointer));
}

Rtt_INLINE Rtt::SharedConstStringPtr Rtt_MakeSharedConstStringPtr(Rtt_Allocator* allocatorPointer)
{
	return Rtt::SharedConstStringPtr(Rtt_NEW(allocatorPointer, Rtt::String(allocatorPointer)));
}

Rtt_INLINE Rtt::SharedConstStringPtr Rtt_MakeSharedConstStringPtr(
	Rtt_Allocator* allocatorPointer, const char* stringPointer)
{
	return Rtt::SharedConstStringPtr(Rtt_NEW(allocatorPointer, Rtt::String(allocatorPointer, stringPointer)));
}


// ----------------------------------------------------------------------------
// Rtt::SharedPtr<std::string> Creating Functions
// ----------------------------------------------------------------------------

Rtt_INLINE Rtt::SharedStdStringPtr Rtt_MakeSharedStdStringPtr()
{
	return Rtt::SharedStdStringPtr::ForCppNewCreatedPtr(new std::string());
}

Rtt_INLINE Rtt::SharedStdStringPtr Rtt_MakeSharedStdStringPtr(const char* stringPointer)
{
	if (!stringPointer)
	{
		return Rtt_MakeSharedStdStringPtr();
	}
	return Rtt::SharedStdStringPtr::ForCppNewCreatedPtr(new std::string(stringPointer));
}

Rtt_INLINE Rtt::SharedStdStringPtr Rtt_MakeSharedStdStringPtr(const char* stringPointer, size_t length)
{
	if (!stringPointer || (0 == length))
	{
		return Rtt_MakeSharedStdStringPtr();
	}
	return Rtt::SharedStdStringPtr::ForCppNewCreatedPtr(new std::string(stringPointer, length));
}

Rtt_INLINE Rtt::SharedStdStringPtr Rtt_MakeSharedStdStringPtr(const std::string& stringReference)
{
	return Rtt::SharedStdStringPtr::ForCppNewCreatedPtr(new std::string(stringReference));
}

Rtt_INLINE Rtt::SharedStdStringPtr Rtt_MakeSharedStdStringPtr(
	const std::string& stringReference, size_t position, size_t length = std::string::npos)
{
	return Rtt::SharedStdStringPtr::ForCppNewCreatedPtr(new std::string(stringReference, position, length));
}


// ----------------------------------------------------------------------------
// Rtt::SharedPtr<const std::string> Creating Functions
// ----------------------------------------------------------------------------

Rtt_INLINE Rtt::SharedConstStdStringPtr Rtt_MakeSharedConstStdStringPtr()
{
	return Rtt::SharedConstStdStringPtr::ForCppNewCreatedPtr(new std::string());
}

Rtt_INLINE Rtt::SharedConstStdStringPtr Rtt_MakeSharedConstStdStringPtr(const char* stringPointer)
{
	if (!stringPointer)
	{
		return Rtt_MakeSharedConstStdStringPtr();
	}
	return Rtt::SharedConstStdStringPtr::ForCppNewCreatedPtr(new std::string(stringPointer));
}

Rtt_INLINE Rtt::SharedConstStdStringPtr Rtt_MakeSharedConstStdStringPtr(const char* stringPointer, size_t length)
{
	if (!stringPointer || (0 == length))
	{
		return Rtt_MakeSharedConstStdStringPtr();
	}
	return Rtt::SharedConstStdStringPtr::ForCppNewCreatedPtr(new std::string(stringPointer, length));
}

Rtt_INLINE Rtt::SharedConstStdStringPtr Rtt_MakeSharedConstStdStringPtr(const std::string& stringReference)
{
	return Rtt::SharedConstStdStringPtr::ForCppNewCreatedPtr(new std::string(stringReference));
}

Rtt_INLINE Rtt::SharedConstStdStringPtr Rtt_MakeSharedConstStdStringPtr(
	const std::string& stringReference, size_t position, size_t length = std::string::npos)
{
	return Rtt::SharedConstStdStringPtr::ForCppNewCreatedPtr(new std::string(stringReference, position, length));
}

#endif // __Rtt_SharedStringPtr_H__
