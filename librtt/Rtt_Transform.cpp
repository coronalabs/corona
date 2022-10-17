//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

Transform::Transform()
:	fMatrix()
{
	fBits.matrix = NULL;
	SetIdentity();
}

void
Transform::SetIdentity()
{
	// fMatrix.SetIdentity(); // TODO: Is this line necessary?
	fX = Rtt_REAL_0;
	fY = Rtt_REAL_0;
	fRotation = Rtt_REAL_0;
	fScaleX = Rtt_REAL_1;
	fScaleY = Rtt_REAL_1;
	fMatrix.SetIdentity();

	SetUserMatrix( NULL );
	SetValid(); // Set valid bit *after* setting user matrix
}

bool
Transform::IsIdentity() const
{
	return ( IsValid() && fMatrix.IsIdentity() );
}

void
Transform::Translate( Real dx, Real dy )
{
	fX += dx;
	fY += dy;
	Invalidate();

	// TODO: This optimization does not work due to anchor/ref pts.
	// fMatrix.Translate( dx, dy );
}

void
Transform::Scale( Real sx, Real sy, bool isNewValue )
{
	if ( isNewValue )
	{
		fScaleX = sx;
		fScaleY = sy;
		Invalidate();
	}
	else
	{
		fScaleX = Rtt_RealMul( fScaleX, sx );
		fScaleY = Rtt_RealMul( fScaleY, sy );
		Invalidate();

		/*
		// This optimization does not work when you have ref points
		// TODO: If transform knew it did not have a ref point or not, it could
		// choose to do this optimization.
		if ( IsValid() )
		{
			Matrix t;

			t.Scale( sx, sy );
			fMatrix.Concat( t ); // Scale( sx, sy );
		}
		*/
	}
}


void
Transform::Rotate( Real dTheta )
{
	fRotation += dTheta;
	Invalidate();
}

void
Transform::SetProperty( GeometricProperty p, Real newValue )
{
	Real* dstValue = NULL;

	switch( p )
	{
		case kOriginX:
			dstValue = & fX;
			break;
		case kOriginY:
			dstValue = & fY;
			break;
		case kScaleX:
			if ( ! Rtt_RealIsZero( newValue ) )
			{
				dstValue = & fScaleX;
			}
			break;
		case kScaleY:
			if ( ! Rtt_RealIsZero( newValue ) )
			{
				dstValue = & fScaleY;
			}
			break;
		case kRotation:
			dstValue = & fRotation;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	if ( dstValue )
	{
		// Do not modify unless newValue differs.
		if ( ! Rtt_RealEqual( * dstValue, newValue ) )
		{
			* dstValue = newValue;
			Invalidate();
		}
	}
}

Real
Transform::GetProperty( GeometricProperty p ) const
{
	Real result = Rtt_REAL_0;

	switch( p )
	{
		case kOriginX:
			result = fX;
			break;
		case kOriginY:
			result = fY;
			break;
		case kScaleX:
			result = fScaleX;
			break;
		case kScaleY:
			result = fScaleY;
			break;
		case kRotation:
			result = fRotation;
			break;
		default:
			break;
	}

	return result;
}

void
Transform::SetUserMatrix( Matrix* newValue )
{
	Matrix* userMatrix = GetUserMatrix();
	if ( userMatrix != newValue )
	{
		Rtt_DELETE( userMatrix );

		// Implicitly invalidates which is what we want
		fBits.matrix = newValue;
	}
}

void
Transform::SetV1Compatibility( bool newValue )
{
	if ( newValue )
	{
		fBits.properties |= kIsV1Compatibility;
	}
	else
	{
		fBits.properties &= ~kIsV1Compatibility;
	}
}


void
Transform::Invalidate()
{
	fBits.properties &= kInvalidateMask;
}

// GetMatrix() returns a matrix which flattens the ops supported by the
// Transform instance. This matrix would apply transform ops to vertices 
// in the following order:
// * anchor offset (determines location of origin)
// * scale (about the origin)
// * rotation (about the origin)
// * user transform 
// * translation (of final positions)
Matrix&
Transform::GetMatrix( const Vertex2 *anchorOffset, const Vertex2 *deltas )
{
	if ( ! IsValid() )
	{
		// Fetch dx, dy *before* we reset the matrix
		Real dx = fX; // fMatrix.Tx();
		Real dy = fY; // fMatrix.Ty();

		fMatrix.SetIdentity();

		if ( anchorOffset )
		{
			// Anchor offset applied first b/c it determines registration pt
			fMatrix.Translate( anchorOffset->x, anchorOffset->y );
		}

		if ( deltas )
		{
			fMatrix.Translate( -deltas->x, -deltas->y );
		}

		fMatrix.Scale( fScaleX, fScaleY );
		fMatrix.Rotate( fRotation );

		Matrix* userMatrix = GetUserMatrix();
		if ( userMatrix )
		{
			fMatrix.Prepend( * userMatrix );
		}

		bool isV1 = IsV1Compatibility();
		if ( isV1 && anchorOffset )
		{
			dx -= anchorOffset->x; dy -= anchorOffset->y;
		}

		if ( deltas ) // n.b. unlike V2 anchors, these are NOT baked into the final position
		{
			dx += deltas->x; dy += deltas->y;
		}

		fMatrix.Translate( dx, dy );

		SetValid();
	}

	return fMatrix;
}

// Generates inverse by operating in opposite order of Self::GetMatrix()
void
Transform::CopyInverseMatrix( const Vertex2* refPt, Matrix& outMatrix ) const
{
	outMatrix.SetIdentity();

	Real dx = fX;
	Real dy = fY;

	if ( refPt ) { dx += refPt->x; dy += refPt->y; }
	outMatrix.Translate( dx, dy );

	Matrix* userMatrix = GetUserMatrix();
	if ( userMatrix )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	outMatrix.Rotate( fRotation );
	outMatrix.Scale( fScaleX, fScaleY );
	if ( refPt ) { outMatrix.Translate( -refPt->x, -refPt->y ); }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

