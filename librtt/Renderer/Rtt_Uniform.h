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
			kUserData0,
			kUserData1,
			kUserData2,
			kUserData3,
			kNumBuiltInVariables,
		}
		Name;

	public:
		Uniform( Rtt_Allocator* allocator, DataType type );
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
