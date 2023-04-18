//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinBitmap.h"
#include "Core\Rtt_Build.h"
#include "Display\Rtt_Display.h"
#include "Interop\Graphics\FontServices.h"
#include "Interop\Graphics\HorizontalAlignment.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_GPUStream.h"
#include "Rtt_WinFont.h"
#include "WinString.h"
#include <windows.h>
#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>
#include <objidl.h>


namespace Rtt
{

// Modeled on ALmixer RWops.
struct BitmapStream : public IStream
{
	public:
		#pragma region IUnknown Methods
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID, __RPC__deref_out void __RPC_FAR *__RPC_FAR * );
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();
		#pragma endregion

		#pragma region ISequentialStream Methods
		virtual HRESULT STDMETHODCALLTYPE Read( __out_bcount_part(cb, *pcbRead) void *pv, ULONG cb, __out_opt ULONG *pcbRead );
		virtual HRESULT STDMETHODCALLTYPE Write( __in_bcount(cb) const void *, ULONG, __out_opt  ULONG * ) { return E_NOTIMPL; }
		#pragma endregion

		#pragma region IStream Methods
		virtual HRESULT STDMETHODCALLTYPE Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, __out_opt ULARGE_INTEGER *plibNewPosition );
		virtual HRESULT STDMETHODCALLTYPE SetSize( ULARGE_INTEGER ) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE CopyTo( IStream *, ULARGE_INTEGER, __out_opt  ULARGE_INTEGER *, __out_opt ULARGE_INTEGER * ) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE Commit( DWORD ) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE Revert() { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE LockRegion( ULARGE_INTEGER, ULARGE_INTEGER, DWORD ) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE UnlockRegion( ULARGE_INTEGER, ULARGE_INTEGER, DWORD ) { return E_NOTIMPL; }
		virtual HRESULT STDMETHODCALLTYPE Stat( __RPC__out STATSTG *pstatstg, DWORD );
		virtual HRESULT STDMETHODCALLTYPE Clone( __RPC__deref_out_opt IStream ** ) { return E_NOTIMPL; }
		#pragma endregion

		#pragma region Public Static Functions
		BitmapStream( void* data, U32 size )
		:	fRefCount( 1 ),
			fStart( data ),
			fOffset( 0 ),
			fSize( size )
		{
		}

		#pragma endregion

	private:
		#pragma region Private Member Variables
		ULONG fRefCount;
		void* fStart;
		U32 fOffset;
		U32 fSize;
		#pragma endregion
};

HRESULT STDMETHODCALLTYPE BitmapStream::QueryInterface( REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject )
{
	if (!ppvObject)
	{
		return E_POINTER;
	}

	if ( ( __uuidof(IUnknown) == riid ) || ( __uuidof(ISequentialStream) == riid ) || ( __uuidof(IStream) == riid ) )
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE BitmapStream::AddRef()
{
	return InterlockedIncrement( &fRefCount );
}

ULONG STDMETHODCALLTYPE BitmapStream::Release()
{
	ULONG currentCount = InterlockedDecrement( &fRefCount );

	if ( 0 == currentCount )
	{
		delete this;
	}

	return currentCount;
}

HRESULT STDMETHODCALLTYPE BitmapStream::Read( __out_bcount_part(cb, *pcbRead) void *pv, ULONG cb, __out_opt ULONG *pcbRead )
{
	if ( !pv )
	{
		return STG_E_INVALIDPOINTER;
	}

	HRESULT result = S_OK;

	if ( fOffset + cb > fSize )
	{
		cb = fSize - fOffset;

		result = S_FALSE;
	}

	if ( cb )
	{
		memcpy( pv, static_cast<U8*>( fStart ) + fOffset, cb );

		fOffset += cb;
	}

	if ( pcbRead )
	{
		*pcbRead = cb;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE BitmapStream::Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, __out_opt ULARGE_INTEGER *plibNewPosition )
{
	S32 offset = 0;

	switch ( dwOrigin )
	{
	case STREAM_SEEK_SET:
		offset = dlibMove.QuadPart;
		break;
	case STREAM_SEEK_END:
		offset = S32( fSize ) + dlibMove.QuadPart;
		break;
	case STREAM_SEEK_CUR:
		offset = S32( fOffset ) + dlibMove.QuadPart;
		break;
	default:
		return STG_E_INVALIDFUNCTION;
	}

	if ( offset < 0 || offset >= S32( fSize ) )
	{
		return STG_E_INVALIDFUNCTION;
	}

	fOffset = U32( offset );

	if ( plibNewPosition )
	{
		plibNewPosition->QuadPart = fOffset;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE BitmapStream::Stat( __RPC__out STATSTG *pstatstg, DWORD )
{
	if ( !pstatstg )
	{
		return STG_E_INVALIDPOINTER;
	}

	memset( pstatstg, 0, sizeof(STATSTG) );

	pstatstg->type = STGTY_STREAM;
	pstatstg->clsid = CLSID_NULL;
	pstatstg->cbSize.QuadPart = fSize;

	return S_OK;
}

WinFileBitmap::FileView::FileView()
:	fMapping( NULL ),
	fData( NULL )
{
}
	
bool WinFileBitmap::FileView::Map( HANDLE hFile )
{
	Close();

    fMapping = CreateFileMapping( hFile, 0, PAGE_READONLY, 0, 0, 0 );

    if ( NULL == fMapping )
	{
		return false;
	}

    fData = MapViewOfFile( fMapping, FILE_MAP_READ, 0, 0, 0 );

    if ( NULL != fData )
	{
		return true;
	}

	else
	{
		CloseHandle( fMapping );

		return false;
	}
}

void WinFileBitmap::FileView::Close()
{
	if ( fData )
	{
		UnmapViewOfFile( fData );
	}

	if ( fMapping )
	{
		CloseHandle( fMapping );
	}

	fMapping = NULL;
	fData = NULL;
}

static U8* LockBitmapData( Rtt_Allocator& allocator, Gdiplus::Bitmap* src, U32* width, U32* height, const char* inPath )
{
	void* out = Rtt_MALLOC( allocator, src->GetWidth() * src->GetHeight() * 4 );

	if ( NULL == out )
	{
		Rtt_TRACE(( "LockBitmapData: unable to allocate data for '%S'\n", inPath ));
		return NULL;
	}

	Gdiplus::BitmapData srcData;
	
	// https://learn.microsoft.com/en-us/windows/win32/api/gdiplusheaders/nf-gdiplusheaders-bitmap-lockbits claim we need
	// to set all of these, though Width, Height, and PixelFormat seem to get set by LockBits() just fine.
	srcData.Width = src->GetWidth();
	srcData.Height = src->GetHeight();
	srcData.PixelFormat = PixelFormat32bppPARGB;
	srcData.Scan0 = out;
	srcData.Stride = srcData.Width * 4;

	Gdiplus::Rect rc( 0, 0, src->GetWidth(), src->GetHeight() );

	if ( Gdiplus::Ok == src->LockBits( &rc, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf, PixelFormat32bppPARGB, &srcData ) )
	{
		Rtt_ASSERT( width );
		Rtt_ASSERT( height );

		*width = srcData.Width;
		*height = srcData.Height;

		src->UnlockBits( &srcData );

		return static_cast<U8*>( out );
	}

	else
	{
		Rtt_TRACE(( "LockBitmapData: unable to lock bitmap for '%S'\n", inPath ));

		Rtt_FREE( out );

		return NULL;
	}
}

static U8* LoadMaskDataFromStream( Rtt_Allocator& allocator, IStream* pStream, U32* width, U32* height, const char* inPath )
{
	using namespace Gdiplus;
	Bitmap src( pStream, FALSE );

	if ( src.GetLastStatus() != Ok )
	{
		Rtt_TRACE(( "LoadImageDataFromStream: failed to create bitmap for '%S'\n", inPath ));
		return NULL;
	}

	return LockBitmapData( allocator, &src, width, height, inPath );
}

static Gdiplus::Bitmap* LoadBitmap( Rtt_Allocator& context, IStream* pStream, U32*, U32*, const char* )
{
	return Rtt_NEW( context, Gdiplus::Bitmap( pStream, FALSE ) );
}

static HANDLE
FileFromPath( const char *inPath )
{
	WinString wPath( inPath );

	return CreateFile( wPath.GetTCHAR(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
}

template<typename R, typename F>
R* LoadBitmapData( HANDLE hFile, Rtt_Allocator &context, F && func, const char *inPath, U32* width, U32* height, WinFileBitmap::FileView* pView = NULL )
{
	if (INVALID_HANDLE_VALUE == hFile)
	{
		Rtt_TRACE(( "LoadBitmapData: unable to load bitmap '%S'\n", inPath ));

		return NULL;
	}

	WinFileBitmap::FileView view;

	bool wasMapped = view.Map( hFile );
	U32 size = GetFileSize( hFile, 0 );

	CloseHandle( hFile );

	if ( !wasMapped )
	{
		Rtt_TRACE(( "LoadBitmapData: unable to map file into memory for '%S'\n", inPath ));

		return NULL;
	}

	R* result = NULL;
	BitmapStream* pStream = Rtt_NEW( context, BitmapStream( view.fData, size ) );

	if ( NULL != pStream )
	{
		result = func( context, pStream, width, height, inPath );

		pStream->Release();
	}

	else
	{
		Rtt_TRACE(( "LoadBitmapData: unable to create memory stream for '%S'\n", inPath ));
	}
	
	if ( NULL != result && NULL != pView )
	{
		*pView = view;
	}

	else
	{
		view.Close();
	}

	return result;
}

WinBitmap::WinBitmap() 
	: fData( NULL ), fBitmap( NULL ),
	  fWidth( 0 ), fHeight( 0 )
{
}

WinBitmap::~WinBitmap()
{
	Self::FreeBits();


	Rtt_DELETE( fBitmap );
}

const void * 
WinBitmap::GetBits( Rtt_Allocator* context ) const
{
	const_cast< WinBitmap * >( this )->Lock( context );

	return fData; 
}

void 
WinBitmap::FreeBits() const
{
}

U32 
WinBitmap::Width() const
{
	if ( 0 != fWidth )
		return fWidth;
	if ( fBitmap == NULL )
		return 0;
	return fBitmap->GetWidth();
}

U32 
WinBitmap::Height() const
{
	if ( 0 != fHeight )
		return fHeight;
	if ( fBitmap == NULL )
		return 0;
	return fBitmap->GetHeight();
}


PlatformBitmap::Format 
WinBitmap::GetFormat() const
{
	return kARGB;
}


// ----------------------------------------------------------------------------

static U8
GetInitialPropertiesValue()
{
	return PlatformBitmap::kIsPremultiplied;
}

WinFileBitmap::WinFileBitmap( Rtt_Allocator &context )
#ifdef Rtt_DEBUG
	: fPath(&context)
#endif
{
	InitializeMembers();
}

WinFileBitmap::WinFileBitmap( const char * inPath, Rtt_Allocator &context )
#ifdef Rtt_DEBUG
	: fPath(&context)
#endif
{
	// Initialize all member variables.
	InitializeMembers();

	// Load bitmap from file.
	HANDLE global = NULL;
	Gdiplus::Bitmap *bm = LoadBitmapData<Gdiplus::Bitmap>( FileFromPath( inPath ), context, LoadBitmap, inPath, NULL, NULL, &fView );

	if ( bm != NULL && bm->GetLastStatus() == Gdiplus::Ok )
	{
		fBitmap = bm;
#ifdef Rtt_DEBUG
		fPath.Set( inPath );
#endif
	}
	else
	{
		fView.Close();

		delete bm;
	}
}

WinFileBitmap::~WinFileBitmap()
{
	fView.Close();

	Rtt_FREE( fData );
}

void
WinFileBitmap::InitializeMembers()
{
	fScale = 1.0;
	fOrientation = kUp;
	fProperties = GetInitialPropertiesValue();
	fBitmap = NULL;
	fData = NULL;
}

PlatformBitmap::Orientation 
WinFileBitmap::GetOrientation() const
{
	return kUp;
}

float
WinFileBitmap::CalculateScale() const
{
	return 1.0;
}


void
WinFileBitmap::Lock( Rtt_Allocator* context )
{
	if ( fBitmap == NULL )
		return;

	fData = LockBitmapData( *context, fBitmap, &fWidth, &fHeight,
	#ifdef Rtt_DEBUG
		fPath.GetString() );
	#else
		"?" );
	#endif

	Rtt_DELETE( fBitmap );

	fBitmap = NULL;

	fView.Close();
}

U32
WinFileBitmap::SourceWidth() const
{
	return WinBitmap::Width();
}

U32
WinFileBitmap::SourceHeight() const
{
	return WinBitmap::Height();
}

U32
WinFileBitmap::Width() const
{
	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceWidth() : UprightWidth() );
}

U32
WinFileBitmap::Height() const
{
	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceHeight() : UprightHeight() );
}

U32
WinFileBitmap::UprightWidth() const
{
	S32 angle = DegreesToUpright();
	return ( 0 == angle || 180 == Abs( angle ) ) ? SourceWidth() : SourceHeight();
}

U32
WinFileBitmap::UprightHeight() const
{
	S32 angle = DegreesToUpright();
	return ( 0 == angle || 180 == Abs( angle ) ) ? SourceHeight() : SourceWidth();
}

bool
WinFileBitmap::IsProperty( PropertyMask mask ) const
{
	return IsPropertyInternal( mask );
}

void
WinFileBitmap::SetProperty( PropertyMask mask, bool newValue )
{
	if ( ! Super::IsPropertyReadOnly( mask ) )
	{
		const U8 p = fProperties;
		const U8 propertyMask = (U8)mask;
		fProperties = ( newValue ? p | propertyMask : p & ~propertyMask );
	}

	switch ( mask )
	{
		case kIsBitsFullResolution:
			fScale = ( newValue ? -1.0f : CalculateScale() );
			break;
		case kIsBitsAutoRotated:
			break;
		default:
			break;
	}
}


// ----------------------------------------------------------------------------

WinFileGrayscaleBitmap::WinFileGrayscaleBitmap( const char *inPath, Rtt_Allocator &context )
	: WinFileBitmap(context)
{
#ifdef Rtt_DEBUG
	fPath.Set( inPath );
#endif

	U8* data = LoadBitmapData<U8>( FileFromPath( inPath ), context, LoadMaskDataFromStream, inPath, &fWidth, &fHeight );

	if ( NULL == data )
	{
		return;
	}

	if ( fWidth && fHeight )
	{
		U32 pitch = fWidth;
		U32 delta = fWidth % kBytePackingAlignment;
		if (delta > 0)
			pitch += kBytePackingAlignment - delta;

		// --------------------------------------------------------------------------------------------------------
		// Microsoft GDI cannot create a grayscaled bitmap that OpenGL needs for masking.
		// GDI can only create 8-bit bitmaps with color palettes. So we have to create the bitmap binary ourselves.
		// --------------------------------------------------------------------------------------------------------
		int byteCount = pitch * fHeight;
		U8* bitmapBuffer = new U8[byteCount];

		if (pitch > fWidth)
		{
			memset( bitmapBuffer, 0, pitch * fHeight );
		}

		U8*out = bitmapBuffer;
		const U8* colors = data;
		U32 rowBase = 0;

		for ( int yIndex = 0; yIndex < fHeight; ++yIndex )
		{
			for ( int xIndex = 0; xIndex < fWidth; ++xIndex )
			{
				// Adapted from http://www.songho.ca/dsp/luminance/luminance.html
				// Originally, the RGB factors and denominator were: 2, 5, 1, and 8, respectively.
				// These numbers (2/8, 5/8, 1/8) were hoisted up into higher denominators (e.g. 4/16, 10/16, 2/16),
				// and the intervals around them searched: e.g. 4/16 += 1/16, 4/16 += 2/16, etc. and compared against
				// the ground truth values (0.30, 0.59, 0.11); with a 64 denominator the end results only differ by
				// 1 (out of 255) at most, and never for solid black or white. It seems to be stuck with this slight
				// imperfection. :)
				out[rowBase + xIndex] = ( 19 * colors[2] + 38 * colors[1] + 7 * colors[0] ) / 64;

				colors += 4;
 			}

			rowBase += pitch;
		}

		fData = bitmapBuffer;

		// Set the image width to the pitch in case it is larger. Otherwise it will not be rendered correctly.
		// Ideally, you shouldn't do this because it will make the DisplayObject wider than expected by at
		// most 3 pixels (assuming the packing alignment is 4 bytes), but until the DisplayObject can compensate
		// for pitch then this will have to do for now.
		fWidth = pitch;
	}

	Rtt_FREE( data );
}

WinFileGrayscaleBitmap::~WinFileGrayscaleBitmap()
{
	if (fData)
	{
		delete fData;
		fData = nullptr;
	}
}

void
WinFileGrayscaleBitmap::FreeBits() const
{
	// Do not delete the grayscale bitmap until this object's destructor has been called.
	// This improves hit-test performance in "Rtt_PlatformBitmap.cpp" which tests a pixel's transparency value.
}

U32
WinFileGrayscaleBitmap::SourceWidth() const
{
	return fWidth;
}

U32
WinFileGrayscaleBitmap::SourceHeight() const
{
	return fHeight;
}

PlatformBitmap::Format 
WinFileGrayscaleBitmap::GetFormat() const
{
	return kMask;
}

#ifdef Rtt_USE_GDIPLUS_RENDERER

WinTextBitmap::WinTextBitmap(
	Interop::RuntimeEnvironment& environment, const char str[], const WinFont& font,
	int width, int height, const Interop::Graphics::HorizontalAlignment& alignment, Real& baselineOffset)
{
	// Initialize width and height to invalid values.
	// Will set to valid values when bitmap has been successfull generated.
	fWidth = 0;
	fHeight = 0;
	auto fontAscender = .0f;
	// Attempt to load the requested font.
	Interop::Graphics::FontSettings fontSettings;
	fontSettings.CopyFrom(font, Gdiplus::UnitPixel);
	auto gdiFontPointer = environment.GetFontServices().LoadUsing(fontSettings);
	if (!gdiFontPointer)
	{
		Rtt_ASSERT(0);
		return;
	}
	
	// Convert the given UTF-8 text to UTF-16.
	WinString stringConverter;
	stringConverter.SetUTF8(str);

	// Set up the text format and alignment.
	// Note: Removing the LineLimit flag causes text to be clipped if there is not enough height.
	Gdiplus::StringFormat textFormat(Gdiplus::StringFormat::GenericTypographic());
	textFormat.SetAlignment(alignment.GetGdiPlusStringAlignmentId());
	INT formatFlags = textFormat.GetFormatFlags();
	formatFlags &= ~Gdiplus::StringFormatFlagsLineLimit;
	if (width <= 0)
	{
		formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
	}

	// Needs to set the "Gdiplus::StringFormatFlagsMeasureTrailingSpaces" flag.
	// Without this flag Windows trims trailing characters at the end of string.
	formatFlags |= Gdiplus::StringFormatFlagsMeasureTrailingSpaces;

	textFormat.SetFormatFlags(formatFlags);

	// sometimes this makes last character invisible because the measured width less than width required to draw the string
#if 0
	// Append a Unicode "Zero-Width Space" (0x200B) to the end of every line in the given string.
	// This allows GDI+ to render trailing spaces without issue.
	// Note: Do not use the "Gdiplus::StringFormatFlagsMeasureTrailingSpaces" flag.
	//       It causes '\n' characters to be rendered as spaces and mismeasures text with line wrapping.
	if (!stringConverter.IsEmpty())
	{
		stringConverter.Replace(L"\r\n", L"\n");
		stringConverter.Replace(L"\n", L"\x200B\n");
	}
	stringConverter.Append(L'\x200B');
#endif

	// Windows ignores last '\n', so needs a space after last '\n' to force 
	if (stringConverter.EndsWith("\n") || stringConverter.EndsWith("\r\n"))
	{
		stringConverter.Append(L'\n');
	}
	
	// Measure the string width and height and store it to "boundingBox".
	Gdiplus::RectF boundingBox;
	Gdiplus::Status status;
	{
		// Set up a temporary device context to measure text rendering with.
		HDC screenDC = ::CreateDCW(L"DISPLAY", nullptr, nullptr, 0);
		Gdiplus::Graphics screenGraphics(screenDC);
		screenGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

		// Measure the string's text output.
		if (width > 0)
		{
			Gdiplus::RectF layoutBox(0, 0, (Gdiplus::REAL)width, (Gdiplus::REAL)height);
			status = screenGraphics.MeasureString(
						stringConverter.GetUTF16(), stringConverter.GetLength(),
						gdiFontPointer.get(), layoutBox, &textFormat, &boundingBox);
		}
		else
		{
			Gdiplus::PointF pointF(0.0f, 0.0f);
			status = screenGraphics.MeasureString(
						stringConverter.GetUTF16(), stringConverter.GetLength(),
						gdiFontPointer.get(), pointF, &textFormat, &boundingBox);
			boundingBox.X = 0;
			boundingBox.Y = 0;
		}

		// Remove the line height from the bottom line, if applied.
		auto fontFamilyPointer = environment.GetFontServices().GetFamilyFrom(*gdiFontPointer.get());
		if (fontFamilyPointer)
		{
			auto fontStyle = gdiFontPointer->GetStyle();
			auto emHeight = fontFamilyPointer->GetEmHeight(fontStyle);
			if (emHeight > 0)
			{
				auto emScale = gdiFontPointer->GetSize() / (Gdiplus::REAL)emHeight;
				auto ascentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellAscent(fontStyle) * emScale;
				auto descentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellDescent(fontStyle) * emScale;
				auto fontHeight = ascentHeight + descentHeight;
				if (boundingBox.Height > fontHeight)
				{
					auto totalHeight = (Gdiplus::REAL)fontFamilyPointer->GetLineSpacing(fontStyle) * emScale;
					auto lineHeight = totalHeight - fontHeight;
					if ((lineHeight > 0) && (lineHeight < boundingBox.Height))
					{
						boundingBox.Height -= lineHeight;
					}
				}
				fontAscender = ascentHeight;
			}
		}

		// Delete the temporary device context.
		::DeleteDC(screenDC);
	}

	// Constrain the text's bounding box if needed.
	int maxTextureSize = Rtt::Display::GetMaxTextureSize();
	if (width <= 0)
	{
		width = (int)std::ceil(boundingBox.Width);			// Round up.
	}
	if (width < 4)
	{
		width = 4;
	}
	if ((width & 0x3) != 0)
	{
		width = (width + 3) & -4;
	}
	if (height <= 0)
	{
		height = (int)std::ceil(boundingBox.Height);		// Round up.
	}
	if (height < 1)
	{
		height = 1;
	}
	
	// Constrain the bitmap width and height to OpenGL's max texture bounds.
	// This will clip the text if the bounding box exceeds the bitmap, which is the intent.
	width = std::min(width, maxTextureSize);
	height = std::min(height, maxTextureSize);
	baselineOffset = height * 0.5f - fontAscender;

	// Draw the text to a bitmap.
	Gdiplus::Bitmap bitmap(width, height, PixelFormat24bppRGB);
	Gdiplus::Graphics graphics(&bitmap);
	Gdiplus::SolidBrush solidBrush(Gdiplus::Color::White);
	graphics.Clear(Gdiplus::Color::Black);
	if (fontSettings.GetPixelSize() > 20.0f)
	{
		// Render text with GDI+ anti-aliasing for large font sizes. (Don't use for small sizes. Will look blurry.)
		// Must be grid-fitted in order to pixel align characters. (Prevents blurry edges on random characters.)
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
	}
	else
	{
		// Render text with Microsoft ClearType for small font sizes. Looks best with small font sizes.
		// Note: Do not use for large font sizes or else the text will be pixelated in the vertical direction.
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
	}

	// use whole bitmap as canvas for drawString
	boundingBox.X = 0;
	boundingBox.Y = 0;
	boundingBox.Width = (float) width; 
	boundingBox.Height = (float) height;

	graphics.DrawString(
			stringConverter.GetUTF16(), stringConverter.GetLength(),
			gdiFontPointer.get(), boundingBox, &textFormat, &solidBrush);
	
	// Convert the 24-bit color bitmap to an 8-bit grayscaled alpha mask.
	if (bitmap.GetLastStatus() == Gdiplus::Ok)
	{
		Gdiplus::BitmapData srcData;
		Gdiplus::Rect rect(0, 0, width, height);
		status = bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &srcData);
		if (Gdiplus::Ok == status)
		{
			fData = new char[width * height];
			if (fData)
			{
				BYTE *dst = (BYTE*)fData;
				const BYTE *src = (const BYTE*)srcData.Scan0;
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
					{
						int grayscaledColor =
								(11 * (int)src[0]) +	// Blue
								(59 * (int)src[1]) +	// Green
								(30 * (int)src[2]);		// Red
						grayscaledColor /= 100;
						*dst = (BYTE)grayscaledColor;
						dst++;
						src += 3;
					}
				}
				fWidth = width;
				fHeight = height;
			}
			status = bitmap.UnlockBits(&srcData);
		}
	}
}
#else

WinTextBitmap::WinTextBitmap(
	Interop::RuntimeEnvironment& environment, const char str[], const WinFont& font,
	int width, int height, const Interop::Graphics::HorizontalAlignment& alignment, Real& baselineOffset)
	: fData(NULL)
	, fWidth(0)
	, fHeight(0)
{
	// Convert the given UTF-8 text to UTF-16.
	WinString stringConverter;
	stringConverter.SetUTF8(str);

	auto fontAscender = .0f;

	// Attempt to load the requested font.
	Interop::Graphics::FontSettings fontSettings;
	fontSettings.CopyFrom(font, Gdiplus::UnitPixel);
	auto gdiFontPointer = environment.GetFontServices().LoadUsing(fontSettings);
	if (!gdiFontPointer)
	{
		Rtt_ASSERT(0);
		return;
	}

	// create in-memory HDC
	HDC hdc = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hdc);
	Rtt_ASSERT(hMemDC != NULL);

//	const char* fontName = font.Name();
//	int nBold = font.IsBold() ? 700 : 400;		// see details https://msdn.microsoft.com/en-us/library/windows/desktop/dd183499(v=vs.85).aspx
//	HFONT hFont = CreateFontA(ceilf(font.Size()*1.8), 0, 0, 0, nBold, font.IsItalic(), 0, 0, 0, 0, 0, PROOF_QUALITY, 0, fontName);

	// clone font
	Gdiplus::Graphics screenGraphics(hdc);
	LOGFONT a_stLogFont;
	gdiFontPointer->GetLogFontW(&screenGraphics, &a_stLogFont);
	a_stLogFont.lfQuality = PROOF_QUALITY;

	HFONT hFont = CreateFontIndirect(&a_stLogFont);
	HGDIOBJ hFontOld = SelectObject(hMemDC, hFont);

	UINT flags = DT_NOPREFIX | DT_WORDBREAK | DT_NOCLIP | DT_EDITCONTROL;
	const char* align = alignment.GetCoronaStringId();
	if (strcmp(align, "left") == 0)
	{
		flags |= DT_LEFT;
	}
	else
	if (strcmp(align, "right") == 0)
	{
		flags |= DT_RIGHT;
	}
	else
	{
		flags |= DT_CENTER;
	}

	// Measure bounding box if width or/and height not provided

	RECT boundingBox;
	SetRect(&boundingBox, 0, 0, width, height);
	int h = DrawTextW(hMemDC, stringConverter.GetUTF16(), stringConverter.GetLength(), &boundingBox, width > 0 ? flags : DT_CALCRECT | DT_NOPREFIX);
	if (h == 0)
	{
		Rtt_LogException("Failed to measure bounding box for: '%s'\n", stringConverter.GetUTF8());
		FreeBits();
		return;
	}
	boundingBox.bottom = h;

	// Remove the line height from the bottom line, if applied.
	auto fontFamilyPointer = environment.GetFontServices().GetFamilyFrom(*gdiFontPointer.get());
	if (fontFamilyPointer)
	{
		auto fontStyle = gdiFontPointer->GetStyle();
		auto emHeight = fontFamilyPointer->GetEmHeight(fontStyle);
		if (emHeight > 0)
		{
			auto emScale = gdiFontPointer->GetSize() / (Gdiplus::REAL)emHeight;
			auto ascentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellAscent(fontStyle) * emScale;
//			auto descentHeight = (Gdiplus::REAL)fontFamilyPointer->GetCellDescent(fontStyle) * emScale;
//			auto fontHeight = ascentHeight + descentHeight;
//			if (boundingBox.bottom > fontHeight)
//			{
//				auto totalHeight = (Gdiplus::REAL)fontFamilyPointer->GetLineSpacing(fontStyle) * emScale;
//				auto lineHeight = totalHeight - fontHeight;
//				if ((lineHeight > 0) && (lineHeight < boundingBox.bottom))
//				{
//					boundingBox.bottom -= lineHeight;
//				}
//			}
			fontAscender = ascentHeight;
		}
	}

	// Constrain the text's bounding box if needed.
	if (width <= 0)
	{
		width = (int) ceilf(boundingBox.right);			// Round up.
	}
	if (width < 4)
	{
		width = 4;
	}
	if ((width & 0x3) != 0)
	{
		width = (width + 3) & -4;
	}
	if (height <= 0)
	{
		height = (int) ceilf(boundingBox.bottom);		// Round up.
	}
	if (height < 1)
	{
		height = 1;
	}

	// Constrain the bitmap width and height to OpenGL's max texture bounds.
	// This will clip the text if the bounding box exceeds the bitmap, which is the intent.
	int maxTextureSize = Rtt::Display::GetMaxTextureSize();
	width = min(width, maxTextureSize);
	height = min(height, maxTextureSize);
	baselineOffset = height * 0.5f - fontAscender;

	fWidth = width;
	fHeight = height;

	BITMAPINFO bitmapInfo;
	memset(&bitmapInfo, 0, sizeof(BITMAPINFO));
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = -height;	//  top-down
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = 0;
	bitmapInfo.bmiHeader.biSizeImage = 0;

	void* m_pBits = NULL;
	HBITMAP hBitmap = CreateDIBSection(hMemDC, &bitmapInfo, DIB_RGB_COLORS, &m_pBits, NULL, NULL);
	Rtt_ASSERT(hBitmap != NULL);

	auto hBitmapOld = SelectObject(hMemDC, hBitmap);

	SetBkMode(hMemDC, TRANSPARENT);
	SetBkColor(hMemDC, RGB(0, 0, 0));
	SetTextColor(hMemDC, 0x00FFFFFF);

	SetRect(&boundingBox, 0, 0, width, height);
	h = DrawTextW(hMemDC, stringConverter.GetUTF16(), stringConverter.GetLength(), &boundingBox, flags);
	if (h != 0)
	{
		fData = new char[width * height];
		BYTE *dst = (BYTE*)fData;
		const BYTE *src = (const BYTE*)m_pBits;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				// rgb ==> grayscale
				*dst++ = 0.2989f * src[0] + 0.5870f * src[1] + 0.1140f * src[2];
				src += 4;
			}
		}
	}
	else
	{
		Rtt_LogException("Failed to draw text: '%s'\n", stringConverter.GetUTF8());
		FreeBits();
	}

	// restore selected objects and release new ones to delete
	SelectObject(hMemDC, hFontOld);
	SelectObject(hMemDC, hBitmapOld);

	DeleteObject(hBitmap);
	DeleteObject(hFont);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hdc);
}

#endif

WinTextBitmap::~WinTextBitmap()
{
	FreeBits();
}

void
WinTextBitmap::FreeBits() const
{
	delete fData;
	fData = NULL;
}

U32 
WinTextBitmap::Width() const
{
	return fWidth;
}

U32 
WinTextBitmap::Height() const
{
	return fHeight;
}

PlatformBitmap::Format 
WinTextBitmap::GetFormat() const
{
	return PlatformBitmap::kMask;
}

const void * 
WinTextBitmap::GetBits( Rtt_Allocator* context ) const
{
	return fData; 
}

} // namespace Rtt
