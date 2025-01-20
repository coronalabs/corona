//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _H_PngReader_
#define _H_PngReader_

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Data.h"

#include "png.h"
#include "AndroidImage.h"

class Rtt_Allocator;

class PngReader
{
private:
	const Rtt::Data<char> *			myBytes;
	int								myBytePosition;
	
	int								myHeight;
	int								myWidth;
	
	Rtt_Allocator *					myAllocator;

	static void ErrorFunc( png_structp, png_const_charp );

	static void WarningFunc( png_structp, png_const_charp );

	static png_voidp MallocFunc( png_structp png_ptr, png_size_t size );

	static void FreeFunc( png_structp png_ptr, png_voidp ptr );

	static void ReadFunc( png_structp png_ptr, png_bytep data, png_size_t length );

public:
	
	PngReader( Rtt_Allocator * a )
	{
		myBytePosition = 0;
		myBytes = NULL;
		myHeight = myWidth = 0;
		myAllocator = a;
	}

	static bool Check(const Rtt::Data<char> & array);

	bool Decode(const Rtt::Data<char> & bytes, AndroidImage & image);
	
	int Width() const;
	int Height() const;
};

#endif
