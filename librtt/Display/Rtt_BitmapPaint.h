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

#ifndef _Rtt_BitmapPaint_H__
#define _Rtt_BitmapPaint_H__

#include "Display/Rtt_Paint.h"

#include "Rtt_Matrix.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class FilePath;
class ImageSheet;
class MCachedResourceLibrary;
class TextureFactory;
class TextureResource;

// ----------------------------------------------------------------------------

class BitmapPaint : public Paint
{
	public:
		typedef Paint Super;

///		static SharedPtr< TextureResource > NewTextureResource( Runtime& runtime, const char* filename, MPlatform::Directory baseDir, U32 flags, bool isMask );

		// Load bitmap from file or reuse bitmap from image cache
		static BitmapPaint* NewBitmap( Runtime& runtime, const char* filename, MPlatform::Directory baseDir, U32 flags );

		// Load bitmap mask from file or reuse bitmap from image cache
		static BitmapPaint* NewBitmap( Runtime& runtime, const FilePath& data, U32 flags, bool isMask );

		// Wrap platform bitmap in Paint-compatible interface. Typically used
		// in conjunction with PlatformImageProvider
		static BitmapPaint* NewBitmap( TextureFactory& factory, PlatformBitmap* bitmap, bool sharedTexture );

		// Device text: create bitmap from string
		static BitmapPaint* NewBitmap( Runtime& runtime, const char text[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset );

	public:
		BitmapPaint( const SharedPtr< TextureResource >& resource );

	public:
		S32 DegreesToUpright() const;

	public:
		virtual const Paint* AsPaint( Super::Type type ) const;
		virtual const MLuaUserdataAdapter& GetAdapter() const;
		virtual void UpdateTransform( Transform& t ) const;

	public:
		virtual PlatformBitmap *GetBitmap() const;

///public:
///const SharedPtr< TextureResource >& GetTextureResource() const { return fResource; }

	public:
		const Transform& GetTransform() const { return fTransform; }
		Transform& GetTransform() { return fTransform; }

	private:
		Transform fTransform; // Texture coordinate transform
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_BitmapPaint_H__
