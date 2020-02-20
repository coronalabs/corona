//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

// Need to include this so that we don't duplicate definitions
// See header for explanation
#include "Core/Rtt_Config.h"

// Either we don't inline (so Rtt_MATH_INLINE is not defined when this  file is compiled)
// or we do inline (so *both* Rtt_MATH_INLINE and Rtt_FIXED_INLINE are defined by the header)
#if !defined( Rtt_MATH_INLINE ) || ( defined( Rtt_MATH_INLINE ) && defined( Rtt_FIXED_INLINE ) )

// ----------------------------------------------------------------------------

#include "Core/Rtt_Build.h"

#include "Core/Rtt_Fixed.h"

// ----------------------------------------------------------------------------

Rtt_EXPORT_BEGIN

Rtt_FIXED_INLINE Rtt_Fixed
Rtt_IntToFixed( S32 a )
{
	return (((Rtt_Fixed)(a)) << 16);
}

Rtt_FIXED_INLINE S32
Rtt_FixedToInt( Rtt_Fixed a )
{
	return ( a + Rtt_FIXED_HALF ) >> 16;
}

Rtt_FIXED_INLINE Rtt_Fixed
Rtt_FloatToFixed( float a )
{
	return (Rtt_Fixed)(a * Rtt_FIXED_1 );
}

Rtt_FIXED_INLINE float
Rtt_FixedToFloat( Rtt_Fixed a )
{
	return ((float)a) / Rtt_FIXED_1;
}

Rtt_FIXED_INLINE Rtt_Fixed
Rtt_FixedMidpoint( Rtt_Fixed a, Rtt_Fixed b )
{
	return (a + b) >> 1;
}

Rtt_FIXED_INLINE Rtt_Fixed
Rtt_FixedMul2( Rtt_Fixed a )
{
	return a << 1;
}

Rtt_FIXED_INLINE Rtt_Fixed
Rtt_FixedDiv2( Rtt_Fixed a )
{
	return a >> 1;
}

Rtt_FIXED_INLINE Rtt_Fixed
Rtt_FixedDiv4( Rtt_Fixed a )
{
	return a >> 2;
}

Rtt_FIXED_INLINE Rtt_Fixed
Rtt_FixedDiv8( Rtt_Fixed a )
{
	return a >> 3;
}

Rtt_FIXED_INLINE Rtt_Fixed
Rtt_FixedDiv16( Rtt_Fixed a )
{
	return a >> 4;
}

// ----------------------------------------------------------------------------

Rtt_FIXED_INLINE
Rtt_Fixed Rtt_FixedAvg( Rtt_Fixed a, Rtt_Fixed b )
{
	return ( a + b ) >> 1;
}

Rtt_FIXED_INLINE
Rtt_Fixed Rtt_FixedAbs( Rtt_Fixed a )
{
	return ( a < 0 ? -a : a );
}

Rtt_FIXED_INLINE
Rtt_Fixed Rtt_FixedMin( Rtt_Fixed a, Rtt_Fixed b )
{
	return ( a < b ? a : b );
}

Rtt_FIXED_INLINE
Rtt_Fixed Rtt_FixedMax( Rtt_Fixed a, Rtt_Fixed b )
{
	return ( a > b ? a : b );
}

#ifdef Rtt_DEBUG
	extern double fabs( double );
#endif

Rtt_FIXED_INLINE 
Rtt_Fixed Rtt_FixedMul( Rtt_Fixed a, Rtt_Fixed b )
{
	Rtt_Fixed result;

	#ifdef Rtt_ARM_ASM

		#if defined( __ARMCC_VERSION )

			Rtt_Fixed r0, r1, r2;

			// The ARM compiler isn't doing a good job of register
			// allocation, so we add add'l MOV instructions to get
			// the following result:
			// SMULL    r1,r2,r0,r1
			// LSRS     r0,r1,#16
			// ADC      r0,r0,r2,LSL #16
			__asm
			{
				MOV r0, a
				MOV r1, b
				SMULL r1, r2, r0, r1
				MOVS r1, r1, LSR#16
				ADC r0, r1, r2, LSL#16
				MOV result, r0
			}

		#elif defined( __GNUC__ )

			// When inlining, the compiler needs more freedom for register
			// allocation, so we don't specify which registers to use.
			Rtt_Fixed r1, r2;
			asm( "smull %0, %1, %2, %3" : "=r"(r1), "=r"(r2) : "r"(a), "r"(b) );
			asm( "movs %0, %0, lsr#16" : "=r"(r1) : "0"(r1) );
			asm( "adc %0, %1, %2, lsl#16" : "=r"(result) : "r"(r1), "r"(r2) );
			/*	
			asm( "mov r0, %0" : : "r"(a) );
			asm( "mov r1, %0" : : "r"(b) );
			asm( "smull r1, r2, r0, r1" );
			asm( "movs r1, r1, lsr#16" );
			asm( "adc r0, r1, r2, lsl#16" );
			asm( "mov %0, r0" : "=r"(result) );
			*/
		#else

			#error Assembly syntax not supported
		
		#endif

	#else
		register S16 aHi = (S16)(a >> 16);
		register S16 bHi = (S16)(b >> 16);
		register U16 aLo = (U16)(a & 0xFFFF);
		register U16 bLo = (U16)(b & 0xFFFF);

		// Conceptually: result = (aHi + aLo) * (bHi + bLo)
		// Note we add 0.5 (0x8000) to round up
		result = (S32)((aLo * bLo + 0x8000u) >> 16) + (aHi * bLo) + (aLo * bHi) + ((aHi * bHi) << 16);

		// Ensure no overflow
		#ifdef Rtt_DEBUG
		{
			S32 cHi = (S32)aHi*(S32)bHi;
			if ( cHi < 0 ) { cHi = -cHi; }
			Rtt_ASSERT( (cHi & 0xFFFF0000) == 0 );
		}
		#endif // Rtt_DEBUG

	#endif

	#ifdef Rtt_DEBUG
	{
		double aDouble = (double)a / 65536.f;
		double bDouble = (double)b / 65536.f;
		double cDouble = aDouble*bDouble;
		double rDouble = (double)result / 65536.f;
		double delta = fabs( cDouble - rDouble );
		double eps = Rtt_FIXED_EPS / 65536.f;
		Rtt_ASSERT( delta <= eps );
	}
	#endif

	return result;
}

Rtt_FORCE_INLINE
Rtt_Fixed Rtt_FixedDivInternal( Rtt_Fixed a, Rtt_Fixed b )
{
	return ( b == Rtt_FIXED_1 ? a : (Rtt_Fixed)( (((S64)a) << 16) / b ) );
}

Rtt_FIXED_INLINE 
Rtt_Fixed Rtt_FixedDiv( Rtt_Fixed a, Rtt_Fixed b )
{
	if ( 0 == a
		 || Rtt_FIXED_INF == b || Rtt_FIXED_NEG_INF == b )
	{
		return 0;
	}
	else if ( 0 == b )
	{
		return ( a > 0 ? Rtt_FIXED_INF : Rtt_FIXED_NEG_INF );
	}
	else
	{
		return Rtt_FixedDivInternal( a, b );
	}
}	

Rtt_FIXED_INLINE
Rtt_Fixed Rtt_FixedDivNonZeroAB( Rtt_Fixed a, Rtt_Fixed b )
{
	if ( Rtt_FIXED_INF == b || Rtt_FIXED_NEG_INF == b )
	{
		return 0;
	}
	else
	{
		return Rtt_FixedDivInternal( a, b );
	}
}

Rtt_FIXED_INLINE
Rtt_Fixed Rtt_FixedDivNonZeroB( Rtt_Fixed a, Rtt_Fixed b )
{
	if ( 0 == a
		 || Rtt_FIXED_INF == b || Rtt_FIXED_NEG_INF == b )
	{
		return 0;
	}
	else
	{
		return Rtt_FixedDivInternal( a, b );
	}
}

// ----------------------------------------------------------------------------

Rtt_EXPORT_END

// ----------------------------------------------------------------------------

#endif // Rtt_MATH_INLINE

// ----------------------------------------------------------------------------
