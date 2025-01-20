//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Matrix_H__
#define _Rtt_Matrix_H__

#include "Core/Rtt_Geometry.h"
#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

struct b2Mat33;

namespace Rtt
{

class Geometry;
struct Vertex2;

// ----------------------------------------------------------------------------

struct Xform3D
{
	public:
		Xform3D();

	public:
		Real tx, ty, tz;
		Real angle, x, y, z;
		Real sx, sy;
};

class Matrix
{
	public:
		static void Invert( const Matrix& src, Matrix& dst );

	public:
		Matrix() { fRow0[0] = Rtt_REAL_INF; }
		Matrix( const Matrix& rhs );

		Rtt_FORCE_INLINE bool IsIdentity() const { return Rtt_REAL_INF == fRow0[0]; }
		Rtt_INLINE void SetIdentity() { fRow0[0] = Rtt_REAL_INF; }

		void SetSingular();

		//bool IsSimple() const; // Translation and scale only
		//void ApplySimple( Rect& r ) const;
		void Apply( Rect& r ) const;
		void Apply( Vertex2& v ) const;
		void Apply( Vertex2 vertices[], S32 numVertices ) const;
		void Apply( Geometry& geometry ) const;
		void ApplyTranslation( Vertex2& v ) const;
		void ApplyScale( Real& value ) const;

		void Translate( Real dx, Real dy );
		void Scale( Real sx, Real sy );
		void Rotate( Real dTheta );

		void Prepend( const Matrix& lhs );
		void Concat( const Matrix& rhs );
		void Invert() { Invert( * this, * this ); }

		void ToGLMatrix( Rtt_Real m[16] ) const;
		void ToGLMatrix3x3( Rtt_Real m[9] ) const;
		void Tob2Mat33( b2Mat33 &m ) const;

		#ifdef Rtt_DEBUG
			bool TestIdentity() const;
			void Trace( const char header[] ) const;
		#endif

		Real Sx() const { Rtt_ASSERT( PreservesOrientation() ); return ( IsIdentity() ? Rtt_REAL_1 : fRow0[0] ); }
		Real Sy() const { Rtt_ASSERT( PreservesOrientation() ); return ( IsIdentity() ? Rtt_REAL_1 : fRow1[1] ); }
		Real Tx() const { return ( IsIdentity() ? Rtt_REAL_0 : fRow0[2] ); }
		Real Ty() const { return ( IsIdentity() ? Rtt_REAL_0 : fRow1[2] ); }
		bool PreservesAspectRatio() const { return IsIdentity() || Rtt_RealEqual( fRow0[0], fRow1[1] ); }
		bool PreservesOrientation() const { return IsIdentity() || PreservesOrientationInternal(); }
		bool PreservesAspectRatioAndOrientation() const { return IsIdentity() || ( Rtt_RealEqual( fRow0[0], fRow1[1] ) && PreservesOrientationInternal() ); }
		bool PreservesSizeAndOrientation() const { return IsIdentity() || ( PreservesOrientationInternal() && Rtt_RealIsOne( fRow0[0] ) && Rtt_RealIsOne( fRow1[1] ) ); }

		void SetTx( Real newValue ) { fRow0[2] = newValue; }
		void SetTy( Real newValue ) { fRow1[2] = newValue; }

	protected:
		Rtt_INLINE bool PreservesOrientationInternal() const { return ( Rtt_RealIsZero( fRow0[1] ) && Rtt_RealIsZero( fRow1[0] ) ); }

	public:
		bool operator==( const Matrix& rhs );
		bool operator!=( const Matrix& rhs ) { return ! operator==( rhs ); }

		Matrix& operator=( const Matrix& rhs );

	public:
		const Real* Row0() const { return fRow0; }
		const Real* Row1() const { return fRow1; }

	private:
		// 2-D transform matrix (3 x 3 matrix where 3rd row is implicitly [ 0 0 1 ] 
		// [ a b tx ]
		// [ c d ty ]
		// [ 0 0 1  ]
		Real fRow0[3];
		Real fRow1[3];
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Matrix_H__
