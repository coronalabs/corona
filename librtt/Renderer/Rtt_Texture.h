//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Texture_H__
#define _Rtt_Texture_H__

#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Types.h"
#include <stdlib.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class Texture : public CPUResource
{
	public:
		typedef CPUResource Super;
		typedef Texture Self;

		typedef enum _FormatValue
		{
			kAlpha,
			kLuminance,
			kRGB,
			kRGBA,
			kBGRA,
			kABGR,
			kARGB,
			kLuminanceAlpha,
			kNumFormats
		}
		FormatValue;

		class Format {
		public:
			Format( FormatValue value = kRGBA );

			FormatValue GetValue() const;
			U32 GetBackingValue() const { return fValue; }
			void SetBackingValue( U32 value ) { fValue = value; }

		public:
			bool IsNonCore() const;
			
			static int BlockDimsID( U8 width, U8 height );
			static void GetBlockDims( int blockDimsID, U8& width, U8& height );
			static int GetCompressedSize( U16 w, U16 h, U8 blockWidth, U8 blockHeight, U8 blockSize );

		private:
			U32 fValue;
		};

		typedef enum _Filter
		{
			kNearest,
			kLinear,
			kNumFilters
		}
		Filter;

		typedef enum _Wrap
		{
			kClampToEdge,
			kRepeat,
			kMirroredRepeat,

			kNumWraps
		}
		Wrap;

		typedef enum _Unit
		{
			kFill0,
			kFill1,
			kMask0,
			kMask1,
			kMask2,
			kNumUnits
		}
		Unit;

		typedef enum _Target
		{
			k2D, // default
			k1D,
			k3D,
			kCube,
			kBuffer,
			kRectangle,
			kMultisample,
			kNumTargets
		}
		Target;

		typedef enum _TargetSubtype
		{
			kNormal, // default
			kArray,
			kImage,
			kImageArray,
			kNumTargetSubtypes
		}
		TargetSubtype;

		typedef enum _Family
		{
			kFloatingPoint, // default
			kSignedInteger,
			kUnsignedInteger,
			kOtherFamily, // depth formats / shadow samplers, atomic_uint
			kNumFamilies
		}
		Family;

	public:

		Texture( Rtt_Allocator* allocator );
		virtual ~Texture();

		virtual ResourceType GetType() const;
		virtual void Allocate();
		virtual void Deallocate();

		virtual U32 GetWidth() const = 0;
		virtual U32 GetHeight() const = 0;
		virtual Format GetFormat() const = 0;
		virtual Filter GetFilter() const = 0;
		virtual Wrap GetWrapX() const;
		virtual Wrap GetWrapY() const;
		virtual size_t GetSizeInBytes() const;
		virtual U8 GetByteAlignment() const;

		virtual const U8* GetData() const;
		virtual void ReleaseData();

		virtual void SetFilter( Filter newValue );
		virtual void SetWrapX( Wrap newValue );
		virtual void SetWrapY( Wrap newValue );
	
	public:
		void SetRetina( bool newValue ){ fIsRetina = newValue; }
		bool IsRetina() const { return fIsRetina; }
		void SetTarget( bool newValue ){ fIsTarget = newValue; }
		bool IsTarget() const { return fIsTarget; }

	private:
		bool fIsRetina;
		bool fIsTarget;
};

struct TextureFormatDescription
{
	enum : U8 {
		kIsDepthRelated = 1 << 0,
		kIsStencilRelated = 1 << 1,
		kIsRenderable1 = 1 << 2, // first or only possiblity
		kIsRenderable2 = 1 << 3, // for depth-stencil
		kHasLinearFiltering = 1 << 4,
		kIsWordPacked = 1 << 5,
		kIsReversed = 1 << 6
	};
	
	enum : U8 {
		kInputKindsMask = ( 1U << 3 ) - 1, // cf. CoronaTextureKind, CoronaGraphics.h (verified elsewhere)
		kFamiliesMask = ( 1U << 2 ) - 1, // cf. CoronaTextureFamily

		kInputKindsShift = 0,
		kFamiliesShift = 3
	};

	bool IsCompressed() const { return !IsWordPacked() && ( 0 != fBlockSize ); }
	bool IsWordPacked() const { return 0 != ( fFlags & kIsWordPacked ); }
	// TODO: IsDepthOrStencil()... 0 != (flags&...)
		// still needs some accompanying logic

	static TextureFormatDescription MakeStandard();
	static TextureFormatDescription MakeCompressed();
	static TextureFormatDescription MakeWordPacked();
	static TextureFormatDescription MakeDepthStencil();

	U16 fFormat;
	U16 fInternal; // mostly for GL, but e.g. Vulkan has some "large" format values
	U16 fDataType;
	U8 fFlags;
	U8 fInputInfo;
	union {
		struct {
			U8 fUnused[2];
			U8 fBytesPerComponent;
			U8 fNumComponents; // also visible to compressed types
		};
		struct {
			U8 fBlockWidth;
			U8 fBlockHeight;
			U8 fBlockSize;
		};
		U8 fSizes[4];
	};
};

class TextureList {
public:
	TextureList() : fFill0( NULL ), fFill1( NULL )
	{
	}
	
	void SetFill0( Texture* newValue ) { fFill0 = newValue; }
	void SetFill1( Texture* newValue ) { fFill1 = newValue; }

	Texture* GetFill0() const;
	Texture* GetFill1() const;

	void Clear() { *this = TextureList(); }
	void PointToArray( Texture** texArray, U32 count );
	bool IsArray() const;
	bool IsEmpty() const { return ( NULL == fFill0 ) && ( NULL == fFill1 ); }
	
// TODO: static method to "mark as fill" (assumes only 1 or 2 textures in first spots)
// then can relax + 2 rule
// in theory we can only have fill1 and not fill0 in the array, though seems odd :D
	// since we really should have at least 4-byte alignment, another bit should be fine
	// probably should double-check that WASM doesn't freak out over this pointer tagging :)
	
	Texture** GetPositionAfterFills() const;
	U32 GetCountAfterFills() const;
	
	const U8* GetNamesList() const;
	
	// these two assume we're using an array:
	U32 GetCount() const;
	Texture** GetArray() const;

private:
	Texture * fFill0; // n.b. might be texture array
	Texture * fFill1; // when fill0 is an array, has low bit set (assumes pointer alignment)
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Texture_H__
