//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Core\Rtt_String.h"
#include "Display\Rtt_PlatformBitmap.h"


#pragma region Forward Declarations
namespace Gdiplus
{
	class Bitmap;
	class BitmapData;
}
namespace Interop
{
	namespace Graphics
	{
		class HorizontalAlignment;
	}
	class RuntimeEnvironment;
}
namespace Rtt
{
	class WinFont;
}

#pragma endregion


namespace Rtt
{

class WinBitmap : public PlatformBitmap
{
	public:
		typedef PlatformBitmap Super;
		typedef WinBitmap Self;
		static const int kBytePackingAlignment = 4;

	protected:
		WinBitmap();
		virtual ~WinBitmap();

	public:
		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual PlatformBitmap::Format GetFormat() const;

	protected:
		mutable void * fData;

	protected:
		Gdiplus::Bitmap *	fBitmap;
		U32 fWidth;
		U32 fHeight;

		virtual void Lock( Rtt_Allocator* context ) {}
};

class WinFileBitmap : public WinBitmap
{

	public:
		typedef WinBitmap Super;

	protected:
		WinFileBitmap( Rtt_Allocator &context );

	public:
		WinFileBitmap( const char *inPath, Rtt_Allocator &context );
		virtual ~WinFileBitmap();

	private:
		void InitializeMembers();

	protected:
		float CalculateScale() const;
		virtual U32 SourceWidth() const;
		virtual U32 SourceHeight() const;

	public:
		virtual PlatformBitmap::Orientation GetOrientation() const;

	protected:
		Rtt_INLINE bool IsPropertyInternal( PropertyMask mask ) const { return (fProperties & mask) ? true : false; }

	public:
		virtual U32 Width() const;
		virtual U32 Height() const;
		virtual bool IsProperty( PropertyMask mask ) const;
		virtual void SetProperty( PropertyMask mask, bool newValue );
		virtual U32 UprightWidth() const;
		virtual U32 UprightHeight() const;

	protected:
		virtual void Lock( Rtt_Allocator* context );

	public:
		struct FileView {
			FileView();
			
			bool Map( HANDLE hFile );
			void Close();

			HANDLE fMapping;
			void* fData;
		};

	private:
		float fScale;
		U8 fProperties;
		S8 fOrientation;
		S16 fAngle; // [0, +-90, +-180]
		FileView fView; // n.b. takes ownership

	protected:
#ifdef Rtt_DEBUG
		String fPath;
#endif
};

class WinFileGrayscaleBitmap : public WinFileBitmap
{
	public:
		typedef WinFileBitmap Super;

		WinFileGrayscaleBitmap( const char *inPath, Rtt_Allocator &context );
		virtual ~WinFileGrayscaleBitmap();

		virtual void FreeBits() const;
		virtual PlatformBitmap::Format GetFormat() const;

	protected:
		virtual U32 SourceWidth() const;
		virtual U32 SourceHeight() const;
};

class WinTextBitmap : public PlatformBitmap
{
	public:
		typedef WinBitmap Super;

		WinTextBitmap(
				Interop::RuntimeEnvironment& environment, const char str[], const WinFont& font,
				int width, int height, const Interop::Graphics::HorizontalAlignment& alignment, Real& baselineOffset);
		virtual ~WinTextBitmap();

		virtual PlatformBitmap::Format GetFormat() const;
		virtual const void* GetBits( Rtt_Allocator* context ) const;
		virtual void FreeBits() const;
		virtual U32 Width() const;
		virtual U32 Height() const;

	private:
		mutable void *fData;
		U32 fWidth;
		U32 fHeight;
};

} // namespace Rtt
