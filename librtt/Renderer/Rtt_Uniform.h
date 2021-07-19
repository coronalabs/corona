//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Uniform_H__
#define _Rtt_Uniform_H__

#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

// ----------------------------------------------------------------------------

class Uniform : public CPUResource
{
	public:
		typedef CPUResource Super;
		typedef Uniform Self;

		typedef enum _DataType
		{
			kScalar,
			kVec2,
			kVec3,
			kVec4,
			kMat3,
			kMat4,
		} 
		DataType;

		static DataType DataTypeForString( const char *value );

		typedef enum _Name
		{
			kViewProjectionMatrix,
			kMaskMatrix0,
			kMaskMatrix1,
			kMaskMatrix2,
			kTotalTime,
			kDeltaTime,
			kTexelSize,
			kContentScale,
			kContentSize, // used by Vulkan backend
			kUserData0,
			kUserData1,
			kUserData2,
			kUserData3,
			kNumBuiltInVariables,
		}
		Name;

	public:
		Uniform( Rtt_Allocator* allocator, DataType type );
		Uniform( DataType type );
		virtual ~Uniform();

		virtual ResourceType GetType() const;
		virtual void Allocate();
		virtual void Deallocate();

		DataType GetDataType() const;
		U32 GetNumValues() const;
		U32 GetSizeInBytes() const;
		
		// Convenience functions for some of the more common data types.
		// For others, especially large types, modify the data directly.
		void GetValue( Real& x );
		void GetValue( Real& x, Real& y );
		void GetValue( Real& x, Real& y, Real& z );
		void GetValue( Real& x, Real& y, Real& z, Real& w );

		void SetValue( Real x );
		void SetValue( Real x, Real y );
		void SetValue( Real x, Real y, Real z );
		void SetValue( Real x, Real y, Real z, Real w );

		// To avoid excessive copying, the data may be manipulated directly.
		// Invalidate() will result in the data being subloaded to the GPU.
		U8* GetData() { return fData; }
		const U8* GetData() const { return fData; }

	private:
		DataType fType;
		U8* fData;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Uniform_H__
