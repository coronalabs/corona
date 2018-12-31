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
