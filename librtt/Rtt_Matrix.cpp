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

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Math.h"

#include "Rtt_Matrix.h"

#include "Renderer/Rtt_Geometry_Renderer.h"

#include <Box2D/Common/b2Math.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

Xform3D::Xform3D()
:	tx( Rtt_REAL_0 ),
	ty( Rtt_REAL_0 ),
	tz( Rtt_REAL_0 ),
	angle( Rtt_REAL_0 ),
	x( Rtt_REAL_0 ),
	y( Rtt_REAL_0 ),
	z( Rtt_REAL_0 ),
	sx( Rtt_REAL_1 ),
	sy( Rtt_REAL_1 )
{
}



/*
Matrix::Matrix( Real width, Real height, Real radius )
:	Super(),
	fWidth( width ),
	fHeight( height ),
	fRadius( radius )
{
}
*/
/*
bool
Matrix::IsSimple() const
{
	return IsIdentity()
			( || Rtt_RealIsZero( fValues[0][1] ) && Rtt_RealIsZero( fValues[1][0] ) );
}
*/

//typedef Real MatrixRow[3];
//typedef Real MatrixValues[2][3];

namespace Private
{

	Rtt_INLINE static Real
	MatrixRowDotVertex2( const Real* Rtt_RESTRICT row, const Vertex2& v )
	{
		Real result = row[2]; // In homogeneous coordinates, we v has an implied 1
		result += Rtt_RealMul( row[0], v.x );
		result += Rtt_RealMul( row[1], v.y );

		return result;
	}

	static void
	MatrixApply( Vertex2& v, const Real* Rtt_RESTRICT row0, const Real* Rtt_RESTRICT row1 )
	{
		Real x = MatrixRowDotVertex2( row0, v );
		Real y = MatrixRowDotVertex2( row1, v );
		v.x = x;
		v.y = y;
	}

	static void
	MatrixInitIdentityValues( Real row0[3], Real row1[3] )
	{
		row0[0] = Rtt_REAL_1;
		row0[1] = Rtt_REAL_0;
		row0[2] = Rtt_REAL_0;

		row1[0] = Rtt_REAL_0;
		row1[1] = Rtt_REAL_1;
		row1[2] = Rtt_REAL_0;
	}

} // Private

Matrix::Matrix( const Matrix& rhs )
{
	* this = rhs;
}


void
Matrix::SetSingular()
{
#if 0 //def Rtt_REAL_FIXED
	memset( & dst, 0, sizeof( Matrix ) );
#else
	fRow0[0] = Rtt_REAL_0;
	fRow0[1] = Rtt_REAL_0;
	fRow0[2] = Rtt_REAL_0;

	fRow1[0] = Rtt_REAL_0;
	fRow1[1] = Rtt_REAL_0;
	fRow1[2] = Rtt_REAL_0;
#endif
}

#if 0
void
Matrix::Append( Matrix& lhs, const Matrix& rhs )
{
	// [  lhs  ] = [  lhs  ] * [  rhs  ]
	// 
	//             [ a b c ]   [ u v w ]
	//           = [ d e f ] * [ x y z ]
	//             [ 0 0 1 ]   [ 0 0 1 ]
	if ( ! m.IsIdentity() )
	{
		Real* const Rtt_RESTRICT row0 = lhs.fRow0;
		Real* const Rtt_RESTRICT row1 = lhs.fRow1;

		if ( IsIdentity() )
		{
			Private::MatrixInitIdentityValues( row0, row1 );
		}

		const Real* Rtt_RESTRICT rhsRow0 = rhs.fRow0;
		const Real* Rtt_RESTRICT rhsRow1 = rhs.fRow1;
		/*
		Real u = rhs.fRow0[0];
		Real v = rhs.fRow0[1];
		Real w = rhs.fRow0[2];
		Real x = rhs.fRow1[0];
		Real y = rhs.fRow1[1];
		Real z = rhs.fRow1[2];
		*/
		Real a = row0[0];
		Real b = row0[1];
		row0[0] = Rtt_RealMul( a, rhsRow0[0] ) + Rtt_RealMul( b, rhsRow1[0] );
		row0[1] = Rtt_RealMul( a, rhsRow0[1] ) + Rtt_RealMul( b, rhsRow1[1] );
		row0[2] += Rtt_RealMul( a, rhsRow0[2] ) + Rtt_RealMul( b, rhsRow1[2] );

		Real d = row1[0];
		Real e = row1[1];
		row1[0] = Rtt_RealMul( d, rhsRow0[0] ) + Rtt_RealMul( e, rhsRow1[0] );
		row1[1] = Rtt_RealMul( d, rhsRow0[1] ) + Rtt_RealMul( e, rhsRow1[1] );
		row1[2] += Rtt_RealMul( d, rhsRow0[2] ) + Rtt_RealMul( e, rhsRow1[2] );
	}
}
#endif

void
Matrix::Apply( Rect& r ) const
{
	if ( ! IsIdentity() )
	{
		if ( r.IsEmpty() )
		{
			r.xMin = r.yMin = r.xMax = r.yMax = Rtt_REAL_0;
		}

		Real xMin, xMax, yMin, yMax;

		// Init points to be corners of the "main" diagonal, i.e. defined by Rect
		Real x0 = r.xMin;
		Real x1 = r.xMax;
		Real y0 = r.yMin;
		Real y1 = r.yMax;

		if ( PreservesOrientationInternal() )
		{
			// No cross terms
			Real a0 = fRow0[0];
			Real a2 = fRow0[2];
			xMin = Rtt_RealMul( x0, a0 ) + a2;
			xMax = Rtt_RealMul( x1, a0 ) + a2;
			if ( xMax < xMin ) { Swap( xMin, xMax ); }

			Real a1 = fRow1[1];
			a2 = fRow1[2];
			yMin = Rtt_RealMul( y0, a1 ) + a2;
			yMax = Rtt_RealMul( y1, a1 ) + a2;
		}
		else
		{
			// The two diagonals of the rect can be used to determine the 
			// x and y bounds of the bounding rectangle. The x-bounds determined
			// by one diagonal and the y-bounds by the other. For pure rotations,
			// we can determine which diagonal defines the x-bounds depends
			// on the sign of the sin/cos component of the rotation matrix.
			// When < 0, the "main" diagonal defines xMin,xMax; otherwise, the
			// other diagonal. The main diagonal is defined by the Rect's ivars.
			// 
			//        -         -         +         +        sign(sin)
			//   |=========|=========|=========|=========|
			//  -90   -   -45   +    0    +   45    -   90   sign(cos)
			//        +         -         +         -        sign(sin) * sign(cos)
			// 
			// Unfortunately, if the scale factors, sx and sy have different sign,
			// it's impossible to determine which diagonal is important. Thus,
			// we cannot use this technique. Instead, we determine which diagonal
			// to use for x-bounds. The opposite diagonal then defines the y-bounds.
			Real a0 = fRow0[0];
			Real a1 = fRow0[1];
			Real a2 = fRow0[2];

			// Main Diagonal (xMin,yMin) and (xMax,yMax)
			Real min1 = Rtt_RealMul( x0, a0 ) + Rtt_RealMul( y0, a1 ) + a2;
			Real max1 = Rtt_RealMul( x1, a0 ) + Rtt_RealMul( y1, a1 ) + a2;
			if ( min1 > max1 ) { Swap( min1, max1 ); }

			// Other Diagonal (xMin,yMax) and (xMax,yMin)
			Real min2 = Rtt_RealMul( x0, a0 ) + Rtt_RealMul( y1, a1 ) + a2;
			Real max2 = Rtt_RealMul( x1, a0 ) + Rtt_RealMul( y0, a1 ) + a2;
			if ( min2 > max2 ) { Swap( min2, max2 ); }

			Rtt_ASSERT( ( min1 <= min2 && max1 >= max2 ) || ( min2 <= min1 && max2 >= max1 ) );
			// The y-bounds always uses the other diagonal. If the x-bounds was
			// determined by the main diagonal, then swap y0, y1.  If it wasn't,
			// then the y-bounds is determined by the main diagonal, so no swap.
			if ( min1 < min2 )
			{
				Swap( y0, y1 );
				xMin = min1;
				xMax = max1;
			}
			else
			{
				xMin = min2;
				xMax = max2;
			}

			a0 = fRow1[0];
			a1 = fRow1[1];
			a2 = fRow1[2];

			yMin = Rtt_RealMul( x0, a0 ) + Rtt_RealMul( y0, a1 ) + a2;
			yMax = Rtt_RealMul( x1, a0 ) + Rtt_RealMul( y1, a1 ) + a2;
		}

		r.xMin = xMin;
		r.xMax = xMax;

		if ( yMax < yMin ) { Swap( yMin, yMax); }
		r.yMin = yMin;
		r.yMax = yMax;
	}
}

void
Matrix::Apply( Vertex2& v ) const
{
	if ( ! IsIdentity() )
	{
		Private::MatrixApply( v, fRow0, fRow1 );
	}
}

void
Matrix::Apply( Vertex2 vertices[], S32 numVertices ) const
{
	if ( ! IsIdentity() )
	{
		const Real* Rtt_RESTRICT row0 = fRow0;
		const Real* Rtt_RESTRICT row1 = fRow1;
		for ( S32 i = 0; i < numVertices; i++ )
		{
			Private::MatrixApply( vertices[i], row0, row1 );
		}
	}
}

void
Matrix::Apply( Geometry& geometry ) const
{
	Geometry::Vertex *data = geometry.GetVertexData();

	for ( int i = 0, iMax = geometry.GetVerticesUsed(); i < iMax; i++ )
	{
		Vertex2 *v = (Vertex2 *)(data + i);
		Apply( * v );
	}
}

void
Matrix::ApplyTranslation( Vertex2& v ) const
{
	if ( ! IsIdentity() )
	{
		v.x += fRow0[2];
		v.y += fRow1[2];
	}
}

void
Matrix::ApplyScale( Real& value ) const
{
	if ( ! IsIdentity() )
	{
		Rtt_ASSERT( fRow0[0] == fRow1[1] );

		value = Rtt_RealMul( value, fRow0[0] );
	}
}

// [this] = [translate] * [this]
void
Matrix::Translate( Real dx, Real dy )
{
	if ( Rtt_REAL_0 != dx || Rtt_REAL_0 != dy )
	{
		Real* Rtt_RESTRICT row0 = fRow0;
		Real* Rtt_RESTRICT row1 = fRow1;

		if ( IsIdentity() )
		{
			Private::MatrixInitIdentityValues( row0, row1 );
		}

		row0[2] += dx;
		row1[2] += dy;
	}
}

// [this] = [scale] * [this]
void
Matrix::Scale( Real sx, Real sy )
{
	const bool isSxNotOne = Rtt_REAL_1 != sx;
	const bool isSyNotOne = Rtt_REAL_1 != sy;

	if ( isSxNotOne || isSyNotOne )
	{
		Real* Rtt_RESTRICT row0 = fRow0;
		Real* Rtt_RESTRICT row1 = fRow1;

		if ( IsIdentity() )
		{
			Private::MatrixInitIdentityValues( row0, row1 );
			row0[0] = sx;
			row1[1] = sy;
		}
		else
		{
			if ( isSxNotOne )
			{
				row0[0] = Rtt_RealMul( row0[0], sx );
				row0[1] = Rtt_RealMul( row0[1], sx );
				row0[2] = Rtt_RealMul( row0[2], sx );
			}

			if ( isSyNotOne )
			{
				row1[0] = Rtt_RealMul( row1[0], sy );
				row1[1] = Rtt_RealMul( row1[1], sy );
				row1[2] = Rtt_RealMul( row1[2], sy );
			}
		}
	}
}

// Normalizes 'value' to the range [start,end)
static double
Normalize( const double value, const double start, const double end ) 
{
	const double width = end - start;
	const double offsetValue = value - start ; // value relative to start

	return ( value - ( floor( offsetValue / width ) * width ) );
}

// [ this ] = [ rotate     ] * [ this  ]
// 
//            [ cos -sin 0 ]   [ u v w ]
//            [ sin cos  0 ] * [ x y z ]
//            [ 0   0    1 ]   [ 0 0 1 ]
void
Matrix::Rotate( Real dTheta )
{
	// TODO: Optimize for multiples of 90, 180, 270, 360
	if ( Rtt_REAL_0 != dTheta )
	{
		double angle = Rtt_RealToFloat( dTheta );
		angle = Normalize( angle, 0., 360. );
		dTheta = Rtt_FloatToReal( angle );

		Real a, b, c, d;

		if ( Rtt_RealIsZero( dTheta ) )
		{
			a = Rtt_REAL_1; b = Rtt_REAL_0;
			c = Rtt_REAL_0; d = Rtt_REAL_1;
		}
		else if ( Rtt_RealEqual( dTheta, 180 ) )
		{
			a = -Rtt_REAL_1; b = Rtt_REAL_0;
			c = Rtt_REAL_0; d = -Rtt_REAL_1;
		}
		else if ( Rtt_RealEqual( dTheta, 90 ) )
		{
			a = Rtt_REAL_0; b = -Rtt_REAL_1;
			c = Rtt_REAL_1; d = Rtt_REAL_0;
		}
		else if ( Rtt_RealEqual( dTheta, 270 ) )
		{
			a = Rtt_REAL_0; b = Rtt_REAL_1;
			c = -Rtt_REAL_1; d = Rtt_REAL_0;
		}
		else
		{
			const Real kDegreesToRadians = Rtt_FloatToReal( M_PI / 180.0f );

			Real radians = Rtt_RealMul( dTheta, kDegreesToRadians );
			a = Rtt_RealCos( radians );
			d = a;
			c = Rtt_RealSin( radians );
			b = -c;
		}

		Real* Rtt_RESTRICT row0 = fRow0;
		Real* Rtt_RESTRICT row1 = fRow1;

		if ( IsIdentity() )
		{
			row0[0] = a;
			row0[1] = b;
			row0[2] = Rtt_REAL_0;

			row1[0] = c;
			row1[1] = d;
			row1[2] = Rtt_REAL_0;
		}
		else
		{
			// TODO: Optimize this
			Real u = row0[0];
			Real v = row0[1];
			Real w = row0[2];
			Real x = row1[0];
			Real y = row1[1];
			Real z = row1[2];

			row0[0] = Rtt_RealMul( a, u ) + Rtt_RealMul( b, x );
			row0[1] = Rtt_RealMul( a, v ) + Rtt_RealMul( b, y );
			row0[2] = Rtt_RealMul( a, w ) + Rtt_RealMul( b, z );

			row1[0] = Rtt_RealMul( c, u ) + Rtt_RealMul( d, x );
			row1[1] = Rtt_RealMul( c, v ) + Rtt_RealMul( d, y );
			row1[2] = Rtt_RealMul( c, w ) + Rtt_RealMul( d, z );
		}
	}
}


// [ this ] = [   m   ] * [ this  ]
// 
//            [ a b c ]   [ u v w ]
//            [ d e f ] * [ x y z ]
//            [ 0 0 1 ]   [ 0 0 1 ]
void
Matrix::Prepend( const Matrix& m )
{
	if ( ! m.IsIdentity() )
	{
		if ( IsIdentity() )
		{
			*this = m;
		}
		else
		{
			Real* const Rtt_RESTRICT row0 = fRow0;
			Real* const Rtt_RESTRICT row1 = fRow1;

			Real u = row0[0];
			Real v = row0[1];
			Real w = row0[2];
			Real x = row1[0];
			Real y = row1[1];
			Real z = row1[2];

			const Real* Rtt_RESTRICT mRow0 = m.fRow0;
			const Real a = mRow0[0];
			const Real b = mRow0[1];
			row0[0] = Rtt_RealMul( a, u ) + Rtt_RealMul( b, x );
			row0[1] = Rtt_RealMul( a, v ) + Rtt_RealMul( b, y );
			row0[2] = Rtt_RealMul( a, w ) + Rtt_RealMul( b, z ) + mRow0[2];

			const Real* Rtt_RESTRICT mRow1 = m.fRow1;
			const Real d = mRow1[0];
			const Real e = mRow1[1];
			row1[0] = Rtt_RealMul( d, u ) + Rtt_RealMul( e, x );
			row1[1] = Rtt_RealMul( d, v ) + Rtt_RealMul( e, y );
			row1[2] = Rtt_RealMul( d, w ) + Rtt_RealMul( e, z ) + mRow1[2];
		}
	}
}

// [ this ] = [ this  ] * [   m   ]
// 
//            [ a b c ]   [ u v w ]
//            [ d e f ] * [ x y z ]
//            [ 0 0 1 ]   [ 0 0 1 ]
void
Matrix::Concat( const Matrix& m )
{
	if ( ! m.IsIdentity() )
	{
		if ( IsIdentity() )
		{
			*this = m;
		}
		else
		{
			Real* const Rtt_RESTRICT row0 = fRow0;
			Real* const Rtt_RESTRICT row1 = fRow1;

			const Real* Rtt_RESTRICT mRow0 = m.fRow0;
			const Real* Rtt_RESTRICT mRow1 = m.fRow1;
			/*
			Real u = m.fRow0[0];
			Real v = m.fRow0[1];
			Real w = m.fRow0[2];
			Real x = m.fRow1[0];
			Real y = m.fRow1[1];
			Real z = m.fRow1[2];
			*/
			Real a = row0[0];
			Real b = row0[1];
			row0[0] = Rtt_RealMul( a, mRow0[0] ) + Rtt_RealMul( b, mRow1[0] );
			row0[1] = Rtt_RealMul( a, mRow0[1] ) + Rtt_RealMul( b, mRow1[1] );
			row0[2] += Rtt_RealMul( a, mRow0[2] ) + Rtt_RealMul( b, mRow1[2] );

			Real d = row1[0];
			Real e = row1[1];
			row1[0] = Rtt_RealMul( d, mRow0[0] ) + Rtt_RealMul( e, mRow1[0] );
			row1[1] = Rtt_RealMul( d, mRow0[1] ) + Rtt_RealMul( e, mRow1[1] );
			row1[2] += Rtt_RealMul( d, mRow0[2] ) + Rtt_RealMul( e, mRow1[2] );
		}
	}
}

void
Matrix::Invert( const Matrix& src, Matrix& dst )
{
	if ( src.IsIdentity() )
	{
		dst.SetIdentity();
	}
	// Non-diagonal terms are zero (0,1) and (1,0)
	else if ( src.fRow0[1] == Rtt_REAL_0 && src.fRow1[0] == Rtt_REAL_0 )
	{
		// If diagonals are zero, then this is a singular matrix
		if ( src.fRow0[0] == Rtt_REAL_0 || src.fRow1[1] == Rtt_REAL_0 )
		{
			dst.SetSingular();
		}
		else
		{
			//     [ a 0 x ]              [ 1/a 0   -x/a ]     
			// M = [ 0 d y ]     Inv(M) = [ 0   1/d -y/d ]
			//     [ 0 0 1 ]              [ 0   0   1    ]
			dst.fRow0[1] = dst.fRow1[0] = Rtt_REAL_0;

			// When the non-diagonal terms are zero, the determinant is just 
			// the product of the diagonals. Hence, the inverse of the
			// diagonal terms is their reciprocal. And the inverse of each 
			// translation term is the negative product of the term and its
			// corresponding inverse diagonal term.
			dst.fRow0[0] = Rtt_RealDivNonZeroAB( Rtt_REAL_1, src.fRow0[0] );
			dst.fRow0[2] = - Rtt_RealMul( dst.fRow0[0], src.fRow0[2] );
			dst.fRow1[1] = Rtt_RealDivNonZeroAB( Rtt_REAL_1, src.fRow1[1] );
			dst.fRow1[2] = - Rtt_RealMul( dst.fRow1[1], src.fRow1[2] );
		}
	}
	else
	{
		//     [ a b x ]                            [  d -b by-dx ]     
		// M = [ c d y ]     Inv(M) = (1/(ad-bc)) * [ -c  a cx-ay ]
		//     [ 0 0 1 ]                            [  0  0 ad-bc ]

		// determinant
		Real dd = Rtt_RealMul( src.fRow0[0], src.fRow1[1] ) - Rtt_RealMul( src.fRow0[1], src.fRow1[0] );

		if ( Rtt_RealIsZero( dd ) )
		{
			dst.SetSingular();
		}
		else
		{
			Real ddInverse = Rtt_RealDivNonZeroAB( Rtt_REAL_1, dd );

			Real a = src.fRow0[0];
			Real b = src.fRow0[1];
			Real x = src.fRow0[2];
			Real c = src.fRow1[0];
			Real d = src.fRow1[1];
			Real y = src.fRow1[2];

			// Need to use double if we don't want rounding errors.
			// 
			// double ddd = (double)a * (double)d - (double)b * (double)c;
			// dst.fRow0[2] = ((double)b*(double)y - (double)d*(double)x) / ddd;
			// dst.fRow1[2] = ((double)a*(double)d - (double)b*(double)c) / ddd;

			dst.fRow0[0] = Rtt_RealMul( ddInverse, d );
			dst.fRow0[1] = - Rtt_RealMul( ddInverse, b );
			dst.fRow0[2] = Rtt_RealMul( ddInverse, Rtt_RealMul( b, y ) - Rtt_RealMul( d, x ) );
			dst.fRow1[0] = - Rtt_RealMul( ddInverse, c );
			dst.fRow1[1] = Rtt_RealMul( ddInverse, a );
			dst.fRow1[2] = Rtt_RealMul( ddInverse, Rtt_RealMul( c, x ) - Rtt_RealMul( a, y ) );
		}
	}

	// Verify identity matrix
	#if defined( Rtt_DEBUG ) && 0
		Matrix tmp = src;
		tmp.Concat( dst );
		tmp.Trace( "src * srcInv" );
		Rtt_ASSERT( tmp.TestIdentity() );

		tmp = dst;
		tmp.Concat( src );
		tmp.Trace( "srcInv * src" );
		Rtt_ASSERT( tmp.TestIdentity() );
	#endif
}

void
Matrix::ToGLMatrix( Rtt_Real m[16] ) const
{
	// TODO: Consider memset'ing m[] to 0 and only touching the non-zero values
	// Why? Possible code size reduction

	// GL's matrices are column-major order
	if ( IsIdentity() )
	{
		m[0] = Rtt_REAL_1;	m[4] = Rtt_REAL_0;	m[8] = Rtt_REAL_0;	m[12] = Rtt_REAL_0;
		m[1] = Rtt_REAL_0;	m[5] = Rtt_REAL_1;	m[9] = Rtt_REAL_0;	m[13] = Rtt_REAL_0;
	}
	else
	{
		m[0] = fRow0[0];	m[4] = fRow0[1];	m[8] = Rtt_REAL_0;	m[12] = fRow0[2];
		m[1] = fRow1[0];	m[5] = fRow1[1];	m[9] = Rtt_REAL_0;	m[13] = fRow1[2];
	}

	m[2] = Rtt_REAL_0;	m[6] = Rtt_REAL_0;	m[10] = Rtt_REAL_1;	m[14] = Rtt_REAL_0;
	m[3] = Rtt_REAL_0;	m[7] = Rtt_REAL_0;	m[11] = Rtt_REAL_0;	m[15] = Rtt_REAL_1;
}

void
Matrix::ToGLMatrix3x3( Rtt_Real m[9] ) const
{
	// TODO: Consider memset'ing m[] to 0 and only touching the non-zero values
	// Why? Possible code size reduction

	// GL's matrices are column-major order
	if ( IsIdentity() )
	{
		m[0] = Rtt_REAL_1;	m[3] = Rtt_REAL_0;	m[6] = Rtt_REAL_0;
		m[1] = Rtt_REAL_0;	m[4] = Rtt_REAL_1;	m[7] = Rtt_REAL_0;
	}
	else
	{
		m[0] = fRow0[0];	m[3] = fRow0[1];	m[6] = fRow0[2];
		m[1] = fRow1[0];	m[4] = fRow1[1];	m[7] = fRow1[2];
	}

	m[2] = Rtt_REAL_0;	m[5] = Rtt_REAL_0;	m[8] = Rtt_REAL_1;
}

void
Matrix::Tob2Mat33( b2Mat33 &m ) const
{
	if ( IsIdentity() )
	{
		m.ex.x = 1.0f;
		m.ex.y = 0.0f;
		m.ex.z = 0.0f;

		m.ey.x = 0.0f;
		m.ey.y = 1.0f;
		m.ey.z = 0.0f;

		m.ez.x = 0.0f;
		m.ez.y = 0.0f;
		m.ez.z = 1.0f;
	}
	else
	{
		m.ex.x = fRow0[ 0 ];
		m.ex.y = fRow1[ 0 ];
		m.ex.z = 0.0f;

		m.ey.x = fRow0[ 1 ];
		m.ey.y = fRow1[ 1 ];
		m.ey.z = 0.0f;

		m.ez.x = fRow0[ 2 ];
		m.ez.y = fRow1[ 2 ];
		m.ez.z = 1.0f;
	}
}

#ifdef Rtt_DEBUG

bool
Matrix::TestIdentity() const
{
	Real a = fRow0[0];
	Real b = fRow0[1];
	Real x = fRow0[2];
	Real c = fRow1[0];
	Real d = fRow1[1];
	Real y = fRow1[2];

	bool result =
		IsIdentity()
		|| ( Rtt_RealEqual( Rtt_REAL_1, a ) && Rtt_RealIsZero( b ) && Rtt_RealIsZero( x )
			 && Rtt_RealIsZero( c ) && Rtt_RealEqual( Rtt_REAL_1, d ) && Rtt_RealIsZero( y ) );

	return result;
}

void
Matrix::Trace( const char header[] ) const
{
	const char kDefault[] = "Matrix";
	if ( ! header ) { header = kDefault; }
	Rtt_TRACE( ( "%s:\n", header ) );

	const float k0 = Rtt_RealToFloat( Rtt_REAL_0 );
	const float k1 = Rtt_RealToFloat( Rtt_REAL_1 );

	if ( IsIdentity() )
	{
		Rtt_TRACE( ( "\t[ %g %g %g ]\n", k1, k0, k0 ) );
		Rtt_TRACE( ( "\t[ %g %g %g ]\n", k0, k1, k0 ) );
	}
	else
	{
		Rtt_TRACE( ( "\t[ %g %g %g ]\n", Rtt_RealToFloat( fRow0[0] ), Rtt_RealToFloat( fRow0[1] ), Rtt_RealToFloat( fRow0[2] ) ) );
		Rtt_TRACE( ( "\t[ %g %g %g ]\n", Rtt_RealToFloat( fRow1[0] ), Rtt_RealToFloat( fRow1[1] ), Rtt_RealToFloat( fRow1[2] ) ) );
	}
	Rtt_TRACE( ( "\t[ %g %g %g ]\n", k0, k0, k1 ) );
}

#endif

bool
Matrix::operator==( const Matrix& rhs )
{
	if ( this != & rhs )
	{
		for ( U32 i = 0; i < ( sizeof(fRow0) / sizeof(fRow0[0]) ); i++ )
		{
			if ( fRow0[i] != rhs.fRow0[i] || fRow1[i] != rhs.fRow1[i] )
			{
				return false;
			}
		}
	}

	return true;
}

Matrix&
Matrix::operator=( const Matrix& rhs )
{
	if ( this != & rhs )
	{
		Real* const Rtt_RESTRICT dstRow0 = fRow0;
		const Real* const Rtt_RESTRICT srcRow0 = rhs.fRow0;
		dstRow0[0] = srcRow0[0];
		dstRow0[1] = srcRow0[1];
		dstRow0[2] = srcRow0[2];

		Real* const Rtt_RESTRICT dstRow1 = fRow1;
		const Real* const Rtt_RESTRICT srcRow1 = rhs.fRow1;
		dstRow1[0] = srcRow1[0];
		dstRow1[1] = srcRow1[1];
		dstRow1[2] = srcRow1[2];
	}

	return * this;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

