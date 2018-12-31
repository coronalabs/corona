// ----------------------------------------------------------------------------
// 
// vlong_io.h
// Copyright (c) 2009 Ansca, Inc. All rights reserved.
// 
// Reviewers:
// 		Walter
//
// ----------------------------------------------------------------------------

#ifndef _vlong_io_H__
#define _vlong_io_H__

#include <stdio.h>

// ----------------------------------------------------------------------------

class vlong;

// ----------------------------------------------------------------------------

class vlong_io
{
	public:
		static void ToWords( U32* output, const U8* input, size_t numBytes );
		static void ToBytes( U8* output, const U32* input, size_t len, bool littleEndian );
		static U8 HexToByte( const char *s );

	public:
		static void FilePrintU32( FILE* f, const char* prefix, const U32* a, size_t numWords );
		static void FilePrintVlong( FILE* f, const char* prefix, const vlong& n );
		static void FileReadU32( FILE* f, const U32* a, size_t numWords );
		static void FileReadVlong( FILE* f, const char* prefix, vlong& n );
		static void PrintU32( const char* prefix, const U32* a, size_t numWords );
		static void PrintVlong( const char* prefix, const vlong& n );
};

// ----------------------------------------------------------------------------

#endif // _vlong_io_H__
