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

#include "Renderer/Rtt_Matrix_Renderer.h"

#include "Core/Rtt_Real.h"
#include <string.h>

// ----------------------------------------------------------------------------

namespace /*anonymous*/ 
{

// ----------------------------------------------------------------------------

const U32 ELEMENTS_PER_MAT3 = 9;
// NOT USED: const U32 ELEMENTS_PER_MAT4 = 16;

void MakeIdentity3x3( Rtt::Real* matrix )
{
	memset( matrix, 0, ELEMENTS_PER_MAT3 * sizeof(Rtt::Real) );
	matrix[0] = 1.0;
	matrix[4] = 1.0;
	matrix[8] = 1.0;
}

void Normalize( Rtt::Real* input, Rtt::Real* output )
{
	Rtt::Real length = sqrt( input[0] * input[0] + input[1] * input[1] + input[2] * input[2] );
	output[0] = input[0] / length;
	output[1] = input[1] / length;
	output[2] = input[2] / length;
}

Rtt::Real Dot( Rtt::Real* lhs, Rtt::Real* rhs )
{
	return lhs[0] * rhs[0] + 
		   lhs[1] * rhs[1] + 
		   lhs[2] * rhs[2];
}

void Cross( Rtt::Real* lhs, Rtt::Real* rhs, Rtt::Real* output )
{
	output[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
	output[1] = lhs[2] * rhs[0] - lhs[0] * rhs[2];
	output[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
}

// ----------------------------------------------------------------------------

} // namespace anonymous

namespace Rtt
{

// ----------------------------------------------------------------------------

void Translation3x3( Real x, Real y, Real* result )
{
	MakeIdentity3x3( result );
	#if COLUMN_MAJOR_MATRIX
		result[6] = x;
		result[7] = y;
	#else

	#endif
}

void Scale3x3( Real x, Real y, Real* result )
{
	MakeIdentity3x3( result );
	#if COLUMN_MAJOR_MATRIX
		result[0] = x;
		result[4] = y;
	#else

	#endif
}

void Rotation3x3( Real angle, Real* result )
{
	MakeIdentity3x3( result );
	Real radians = angle * M_PI / 180.0f;
	Real sinAngle = sin( radians );
	Real cosAngle = cos( radians );

	#if COLUMN_MAJOR_MATRIX
		result[0] = cosAngle;
		result[1] = sinAngle;
		result[3] = -sinAngle;
		result[4] = cosAngle;
	#else

	#endif
}

void Multiply3x3( const Real* m0, const Real* m1, Real* result )
{
	#if COLUMN_MAJOR_MATRIX
		result[0] = m0[0]*m1[0] + m0[3]*m1[1] + m0[6]*m1[2];
		result[1] = m0[1]*m1[0] + m0[4]*m1[1] + m0[7]*m1[2];
		result[2] = m0[2]*m1[0] + m0[5]*m1[1] + m0[8]*m1[2];
		result[3] = m0[0]*m1[3] + m0[3]*m1[4] + m0[6]*m1[5];
		result[4] = m0[1]*m1[3] + m0[4]*m1[4] + m0[7]*m1[5];
		result[5] = m0[2]*m1[3] + m0[5]*m1[4] + m0[8]*m1[5];
		result[6] = m0[0]*m1[6] + m0[3]*m1[7] + m0[6]*m1[8];
		result[7] = m0[1]*m1[6] + m0[4]*m1[7] + m0[7]*m1[8];
		result[8] = m0[2]*m1[6] + m0[5]*m1[7] + m0[8]*m1[8];
	#else

	#endif
}

void Multiply4x4( const Real* m0, const Real* m1, Real* result )
{
	#if COLUMN_MAJOR_MATRIX
		result[0] = m0[0]*m1[0] + m0[4]*m1[1] + m0[8]*m1[2] + m0[12]*m1[3];
		result[1] = m0[1]*m1[0] + m0[5]*m1[1] + m0[9]*m1[2] + m0[13]*m1[3];
		result[2] = m0[2]*m1[0] + m0[6]*m1[1] + m0[10]*m1[2] + m0[14]*m1[3];
		result[3] = m0[3]*m1[0] + m0[7]*m1[1] + m0[11]*m1[2] + m0[15]*m1[3];
		result[4] = m0[0]*m1[4] + m0[4]*m1[5] + m0[8]*m1[6] + m0[12]*m1[7];
		result[5] = m0[1]*m1[4] + m0[5]*m1[5] + m0[9]*m1[6] + m0[13]*m1[7];
		result[6] = m0[2]*m1[4] + m0[6]*m1[5] + m0[10]*m1[6] + m0[14]*m1[7];
		result[7] = m0[3]*m1[4] + m0[7]*m1[5] + m0[11]*m1[6] + m0[15]*m1[7];
		result[8] = m0[0]*m1[8] + m0[4]*m1[9] + m0[8]*m1[10] + m0[12]*m1[11];
		result[9] = m0[1]*m1[8] + m0[5]*m1[9] + m0[9]*m1[10] + m0[13]*m1[11];
		result[10] = m0[2]*m1[8] + m0[6]*m1[9] + m0[10]*m1[10] + m0[14]*m1[11];
		result[11] = m0[3]*m1[8] + m0[7]*m1[9] + m0[11]*m1[10] + m0[15]*m1[11];
		result[12] = m0[0]*m1[12] + m0[4]*m1[13] + m0[8]*m1[14] + m0[12]*m1[15];
		result[13] = m0[1]*m1[12] + m0[5]*m1[13] + m0[9]*m1[14] + m0[13]*m1[15];
		result[14] = m0[2]*m1[12] + m0[6]*m1[13] + m0[10]*m1[14] + m0[14]*m1[15];
		result[15] = m0[3]*m1[12] + m0[7]*m1[13] + m0[11]*m1[14] + m0[15]*m1[15];
	#else
		#error
	#endif
}

void MultiplyVec3Mat3( const Real* v, const Real* m, Real* result )
{
	Real x = v[0];
	Real y = v[1];
	Real z = v[2];

	#if COLUMN_MAJOR_MATRIX
		result[0] = m[0] * x + m[3] * y + m[6] * z;
		result[1] = m[1] * x + m[4] * y + m[7] * z;
		result[2] = m[2] * x + m[5] * y + m[8] * z; 
	#else

	#endif
}

void MultiplyVec4Mat4( const Real* v, const Real* m, Real* result )
{
	Real x = v[0];
	Real y = v[1];
	Real z = v[2];
	Real w = v[3];

	#if COLUMN_MAJOR_MATRIX
		result[0] = m[0] * x + m[4] * y + m[8] * z + m[12] * w;
		result[1] = m[1] * x + m[5] * y + m[9] * z + m[13] * w;
		result[2] = m[2] * x + m[6] * y + m[10] * z + m[14] * w;
		result[3] = m[3] * x + m[7] * y + m[11] * z + m[15] * w;
	#else

	#endif
}

void ClipToWindow( const Real* input, S32 viewportX, S32 viewportY, Real* result )
{
	result[0] = ((input[0] / input[3]) * 0.5f + 0.5f) * viewportX;
	result[1] = ((input[1] / input[3]) * 0.5f + 0.5f) * viewportY;
}

void CreateViewMatrix( Real ex, Real ey, Real ez, // Eye position.
	                   Real cx, Real cy, Real cz, // Target position.
					   Real ux, Real uy, Real uz, // Up vector.
					   Real* result )
{
	Real fwd[] = { cx - ex, cy - ey, cz - ez };
	Real eye[] = { ex, ey, ez };
	Real up[] = { ux, uy, uz };
	Real right[3];
	
	Real nf[3], nu[3], ns[3];
	Normalize( fwd, nf );
	Normalize( up, nu );

	Cross( nf, nu, right );
	Normalize( right, ns );
	Cross( ns, nf, nu );

	const U32 ELEMENTS_PER_MAT4 = 16;
	memset( result, 0, ELEMENTS_PER_MAT4 * sizeof( Real ) );

	result[0] = ns[0];
	result[4] = ns[1];
	result[8] = ns[2];
	result[1] = nu[0];
	result[5] = nu[1];
	result[9] = nu[2];
	result[2] = -nf[0];
	result[6] = -nf[1];
	result[10] = -nf[2];
	result[12] = -Dot( ns, eye );
	result[13] = -Dot( nu, eye );
	result[14] =  Dot( nf, eye );
	result[15] = 1.0f;
}

void CreateOrthoMatrix( Real left, Real right, Real bottom, Real top, Real zNear, Real zFar, Real* result )
{
	const U32 ELEMENTS_PER_MAT4 = 16;
	memset( result, 0, ELEMENTS_PER_MAT4 * sizeof( Real ) );

	result[0] = 2.0f / ( right - left );
	result[5] = 2.0f / ( top - bottom );
	result[10] = -2.0f / ( zFar - zNear );
	result[12] = -( right + left ) / ( right - left );
	result[13] = -( top + bottom ) / ( top - bottom );
	result[14] = -( zFar + zNear ) / ( zFar - zNear );
	result[15] = 1.0f;
}

void CreatePerspectiveMatrix( Real fovy, Real aspectRatio, Real zNear, Real zFar, Real* result )
{
	Real range = tan( fovy / 2.0f ) * zNear;
	Real left = -range * aspectRatio;
	Real right = range * aspectRatio;
	Real bottom = -range;
	Real top = range;

	const U32 ELEMENTS_PER_MAT4 = 16;
	memset( result, 0, ELEMENTS_PER_MAT4 * sizeof( Real ) );

	result[0] = ( 2.0f * zNear ) / ( right - left );
	result[5] = ( 2.0f * zNear ) / ( top - bottom );
	result[10] = -( zFar + zNear ) / ( zFar - zNear );
	result[11] = -1.0f;
	result[14] = -( 2.0f * zFar * zNear ) / ( zFar - zNear );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
