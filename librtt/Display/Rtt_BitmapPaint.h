//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		static BitmapPaint* NewBitmap( Runtime& runtime, const FilePath& data, U32 flags, bool isMask, bool onlyForHitTests = false );

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
