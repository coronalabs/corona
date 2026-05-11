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
			kOtherFamily, // shadow formats, atomic_uint
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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Texture_H__
