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

#ifndef _Rtt_String_H__
#define _Rtt_String_H__

#include "Core/Rtt_Types.h"
#include <string.h>

// ----------------------------------------------------------------------------

Rtt_EXPORT_BEGIN

// ----------------------------------------------------------------------------

int Rtt_StringCompare( const char * s1, const char * s2 );
int Rtt_StringCompareNoCase( const char * s1, const char * s2 );
int Rtt_StringEndsWith( const char * s, const char * suffix );
int Rtt_StringEndsWithNoCase( const char * s, const char * suffix );
int Rtt_StringStartsWith( const char * s, const char * prefix );
bool Rtt_StringIsEmpty( const char *s );

// ----------------------------------------------------------------------------

Rtt_EXPORT_END

// ----------------------------------------------------------------------------
struct Rtt_Allocator;

namespace Rtt
{

// ----------------------------------------------------------------------------

class String
{
	Rtt_CLASS_NO_COPIES( String )

	public:
        String();
		String( Rtt_Allocator * a );
		String(Rtt_Allocator * a, const char * str);
		String(const char * str);
		~String();

	public:
		// const char * cast
		inline operator const char*() const { return (fSrcBuffer != NULL) ? fSrcBuffer : fBuffer; }
		inline operator char*() const { return (fSrcBuffer != NULL) ? ((char *)fSrcBuffer) : fBuffer; }

		// Comparison operators
		inline bool operator == (const String& rhs)
		{
		   if (GetString() == rhs.GetString())
		   {
			  // String pointers equal. Definitely a match
			  return true;
		   }
		   else if (GetString() == NULL || rhs.GetString() == NULL)
		   {
			   // One of the strings is null and the other is not.
			   // Note: We know they're not both null due to the previous check
			   return false;
		   }

		   return (strcmp(GetString(), rhs.GetString()) == 0);
		}

		inline bool operator != (const String& rhs)
		{
		   return ! (*this == rhs);
		}

		// Copies "s"
		void Set( const char *s );

		// Shallow ref to "srcBuffer" following copy-on-write semantics. 
		// Caller must ensure that srcBuffer outlives receiver (or that receiver 
		// does an operation so that it no longer needs to refer to srcBuffer 
		// (e.g. call to Append or Set)
		void SetSrc( const char * srcBuffer ) 
		{ 
			Rtt_DELETE( fBuffer );
			fBuffer = NULL;
			fSrcBuffer = srcBuffer; 
		}

		Rtt_INLINE const char * GetString() const
		{
			return fSrcBuffer ? fSrcBuffer : fBuffer;
		}

		Rtt_INLINE size_t GetLength() const
		{
			if (GetString() != NULL)
			{
				return strlen(GetString());
			}
			else
			{
				return 0;
			}
		}

		Rtt_FORCE_INLINE char * GetMutableString() const
		{
			return fBuffer;
		}
		
		void Append( const char * );

		void Reserve( size_t length );
		
		Rtt_FORCE_INLINE Rtt_Allocator *	GetAllocator() const
		{
			return fAllocator;
		}

        Rtt_INLINE bool IsEmpty() const
		{
			return ( (NULL == GetString()) 
				|| ('\0' == GetString()[0]) );
		}

		// Trim characters from either or both ends of the string
		void Trim(const char *trimChars);
		void LTrim(const char *trimChars);
		void RTrim(const char *trimChars);

		// Path operations

		// Append the given string to the end of the path including one path separator
		void AppendPathComponent(const char *);
		// Remove the last path component from the string (noop if no path separators in string)
		void RemovePathComponent();
		// Ensure string ends with one path separator
		void AppendPathSeparator();
		// Remove any path separator(s) at the end of the string
		void RemovePathSeparator();
		// Return a pointer to the first character of the last path component in the string
		const char *GetLastPathComponent();

	private:
		bool IsPathSeparator(char c) { return (c != 0 && strchr(fPathSeparator, c) != NULL); }

		char *fBuffer;
		const char *fSrcBuffer;
		Rtt_Allocator *	fAllocator;
		const char *fPathSeparator;
};

class StringArray
{
		int fLength;
		String *fElements;
		Rtt_Allocator * fAllocator;

		Rtt_CLASS_NO_COPIES( StringArray )

	public:
		StringArray( Rtt_Allocator * a );
		~StringArray();

		Rtt_FORCE_INLINE int GetLength() const
		{
			return fLength;
		}

		void Reserve( int length );
		void SetElement( int index, const char *str );
		const char* GetElement( int index ) const;
};

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_String_H__
