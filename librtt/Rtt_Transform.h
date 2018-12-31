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

#ifndef _Rtt_Transform_H__
#define _Rtt_Transform_H__

#include "Core/Rtt_Geometry.h"
#include "Core/Rtt_Real.h"

#include "Rtt_Matrix.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// The Transform class simplifies 2-D affine transformations.
// 
// Typically, transformations involve manipulating a transform matrix.
// However, the typical operations are scaling the object about its center,
// rotating the object about its center, and translating the object's center
// relative to a parent.  
// 
// This is much more difficult to accomplish when you have to modify an
// existing transform matrix. The other extreme is to define 3 transforms
// in a rigid sequence, e.g. first scale, then rotate, and finally translate.
// This is simple to use, but limits the kind of transforms that can be applied.
// 
// The Transform class is a hybrid that attempts to balance that ease of use 
// with the flexibility of a fully definable matrix. It sandwiches a user-defined
// matrix between scale/rotate ops and the translate op.
// 
// GetMatrix() returns a matrix which flattens the ops supported by the
// Transform instance. This matrix would apply transform ops to vertices 
// in the following order:
// * scale (about the reference pt)
// * rotation (about the reference pt)
// * user transform 
// * translation (of final positions)
class Transform
{
	public:
		typedef Transform Self;

	public:
		Transform();

	public:
		void SetIdentity();
		bool IsIdentity() const;

	public:
		// Translate the location of origin relative to parent
		void Translate( Real dx, Real dy );

		// Scale about reference pt
		void Scale( Real sx, Real sy, bool isNewValue );

		// Rotate about reference pt
		void Rotate( Real dTheta );

		void SetProperty( GeometricProperty p, Real newValue );
		Real GetProperty( GeometricProperty p ) const;

		Real GetX() const { return fX; }
		Real GetY() const { return fY; }
		Real GetRotation() const { return fRotation; }
		Real GetSx() const { return fScaleX; }
		Real GetSy() const { return fScaleY; }

		Matrix* GetUserMatrix() const { return (Matrix*)( fBits.properties & kUserMatrixMask ); }
		void SetUserMatrix( Matrix* newValue );

	protected:
		enum Constants
		{
			kIsValid = 0x1,
			kIsV1Compatibility = 0x2,
			
			kInvalidateMask = ~(kIsValid),
			kUserMatrixMask = ~(kIsValid | kIsV1Compatibility),
		};

		void SetValid() { fBits.properties |= kIsValid; }

	public:
		void SetV1Compatibility( bool newValue );
		bool IsV1Compatibility() const { return (fBits.properties & kIsV1Compatibility) ? true : false; }

	public:
		bool IsValid() const { return (fBits.properties & kIsValid) ? true : false; }
		void Invalidate();

	public:
		// This should be the other way, the non-const GetMatrix() calling the 
		// the const one, but that required making fBits mutable --- 
		// and in every other method, the constness of fBits *should* be
		// enforced by the compiler.
		const Matrix& GetMatrix( const Vertex2 *anchorOffset ) const
		{
			return const_cast< Self* >( this )->GetMatrix( anchorOffset );
		}
		Matrix& GetMatrix( const Vertex2 *anchorOffset );

		void CopyInverseMatrix( const Vertex2* refPt, Matrix& outMatrix ) const;

	private:
		// Object's cached transform		
		Matrix fMatrix;

		// Transform properties
		Real fX;
		Real fY;
		Real fRotation;
		Real fScaleX;
		Real fScaleY;

// Rtt_USE_64BIT
//#ifdef 32bit
//	typedef U32 Properties;
//#elif 64bit
//	typedef U64 Properties;
//#endif
	typedef size_t Properties;
		Rtt_STATIC_ASSERT( sizeof( Matrix* ) == sizeof( Properties ) );
		typedef union Bits
		{
			Matrix* matrix;
			Properties properties;
		}
		Bits;

		Bits fBits;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Transform_H__
