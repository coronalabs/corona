//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_EmitterSIMD_H__
#define _Rtt_EmitterSIMD_H__

#include <math.h>
#include <stdint.h>

// ----------------------------------------------------------------------------

// A minimal 4-lane float vector for the emitter's particle kernels.
//
// Every operation here is exact IEEE single precision (add, subtract, multiply,
// divide, square root, compare, select), so a kernel that performs the same
// operations in the same order as its scalar version gets bit-identical results.
// Anything else (trigonometry, degrees to radians, epsilon tests) stays scalar.
//
// Define Rtt_EMITTER_NO_SIMD to use the scalar kernels only.

#if defined( Rtt_EMITTER_NO_SIMD )
	// Scalar only.
#elif defined( __SSE2__ ) || defined( _M_X64 ) || ( defined( _M_IX86_FP ) && _M_IX86_FP >= 2 )
	#define Rtt_EMITTER_SIMD_SSE2 1
	#include <emmintrin.h>
#elif defined( __aarch64__ ) || defined( _M_ARM64 )
	// Not ARMv7: its NEON has no vector divide or square root, and flushes
	// denormals to zero, so results would differ from the scalar code.
	#define Rtt_EMITTER_SIMD_NEON 1
	#include <arm_neon.h>
#elif defined( __wasm_simd128__ )
	#define Rtt_EMITTER_SIMD_WASM 1
	#include <wasm_simd128.h>
#endif

#if defined( Rtt_EMITTER_SIMD_SSE2 ) || defined( Rtt_EMITTER_SIMD_NEON ) || defined( Rtt_EMITTER_SIMD_WASM )
	#define Rtt_EMITTER_SIMD 1
#endif

#ifdef Rtt_EMITTER_SIMD

// ----------------------------------------------------------------------------

namespace Rtt
{

namespace EmitterSIMD
{

// ----------------------------------------------------------------------------

#if defined( Rtt_EMITTER_SIMD_SSE2 )

typedef __m128 V4;

inline V4 Load( const float *p ) { return _mm_loadu_ps( p ); }
inline void Store( float *p, V4 v ) { _mm_storeu_ps( p, v ); }
inline V4 Splat( float x ) { return _mm_set1_ps( x ); }

inline V4 Add( V4 a, V4 b ) { return _mm_add_ps( a, b ); }
inline V4 Sub( V4 a, V4 b ) { return _mm_sub_ps( a, b ); }
inline V4 Mul( V4 a, V4 b ) { return _mm_mul_ps( a, b ); }
inline V4 Div( V4 a, V4 b ) { return _mm_div_ps( a, b ); }
inline V4 Sqrt( V4 a ) { return _mm_sqrt_ps( a ); }
inline V4 Neg( V4 a ) { return _mm_xor_ps( a, _mm_set1_ps( -0.0f ) ); }

// Masks: all bits set in lanes where the comparison holds.
inline V4 Equal( V4 a, V4 b ) { return _mm_cmpeq_ps( a, b ); }
inline V4 NotEqual( V4 a, V4 b ) { return _mm_cmpneq_ps( a, b ); } // true for NaN, like a C truth test
inline V4 Less( V4 a, V4 b ) { return _mm_cmplt_ps( a, b ); }
inline V4 Greater( V4 a, V4 b ) { return _mm_cmpgt_ps( a, b ); }
inline V4 Or( V4 a, V4 b ) { return _mm_or_ps( a, b ); }
inline V4 And( V4 a, V4 b ) { return _mm_and_ps( a, b ); }
inline V4 AndNot( V4 a, V4 b ) { return _mm_andnot_ps( b, a ); } // a & ~b
inline V4 Select( V4 mask, V4 a, V4 b ) { return _mm_or_ps( _mm_and_ps( mask, a ), _mm_andnot_ps( mask, b ) ); }
inline bool Any( V4 mask ) { return 0 != _mm_movemask_ps( mask ); }
inline V4 MaskFromBools( bool a, bool b, bool c, bool d ) { return _mm_castsi128_ps( _mm_set_epi32( -(int)d, -(int)c, -(int)b, -(int)a ) ); }

// Lane i of the result is ( r & 0xFF ) | ( g & 0xFF ) << 8 | ..., where each
// channel is truncated to an integer, as a C cast does.
inline void PackBytes( uint32_t out[4], V4 r, V4 g, V4 b, V4 a )
{
	const __m128i kByte = _mm_set1_epi32( 0xFF );
	__m128i ri = _mm_and_si128( _mm_cvttps_epi32( r ), kByte );
	__m128i gi = _mm_and_si128( _mm_cvttps_epi32( g ), kByte );
	__m128i bi = _mm_and_si128( _mm_cvttps_epi32( b ), kByte );
	__m128i ai = _mm_and_si128( _mm_cvttps_epi32( a ), kByte );
	__m128i packed = _mm_or_si128( _mm_or_si128( ri, _mm_slli_epi32( gi, 8 ) ), _mm_or_si128( _mm_slli_epi32( bi, 16 ), _mm_slli_epi32( ai, 24 ) ) );
	_mm_storeu_si128( (__m128i *)out, packed );
}

typedef __m128i V4i;

inline V4 Xor( V4 a, V4 b ) { return _mm_xor_ps( a, b ); }
inline V4i TruncateToInt( V4 a ) { return _mm_cvttps_epi32( a ); }
inline V4 IntToFloat( V4i a ) { return _mm_cvtepi32_ps( a ); }
inline V4i SplatInt( int x ) { return _mm_set1_epi32( x ); }
inline V4i AddInt( V4i a, V4i b ) { return _mm_add_epi32( a, b ); }
inline V4i SubInt( V4i a, V4i b ) { return _mm_sub_epi32( a, b ); }
inline V4i AndInt( V4i a, V4i b ) { return _mm_and_si128( a, b ); }
inline V4 EqualInt( V4i a, V4i b ) { return _mm_castsi128_ps( _mm_cmpeq_epi32( a, b ) ); }
inline V4 BitsToFloat( V4i a ) { return _mm_castsi128_ps( a ); }
template < int kShift > inline V4i ShiftLeftInt( V4i a ) { return _mm_slli_epi32( a, kShift ); }

#elif defined( Rtt_EMITTER_SIMD_NEON )

typedef float32x4_t V4;

inline V4 Load( const float *p ) { return vld1q_f32( p ); }
inline void Store( float *p, V4 v ) { vst1q_f32( p, v ); }
inline V4 Splat( float x ) { return vdupq_n_f32( x ); }

inline V4 Add( V4 a, V4 b ) { return vaddq_f32( a, b ); }
inline V4 Sub( V4 a, V4 b ) { return vsubq_f32( a, b ); }
inline V4 Mul( V4 a, V4 b ) { return vmulq_f32( a, b ); }
inline V4 Div( V4 a, V4 b ) { return vdivq_f32( a, b ); }
inline V4 Sqrt( V4 a ) { return vsqrtq_f32( a ); }
inline V4 Neg( V4 a ) { return vnegq_f32( a ); }

inline V4 FromMask( uint32x4_t m ) { return vreinterpretq_f32_u32( m ); }
inline uint32x4_t ToMask( V4 m ) { return vreinterpretq_u32_f32( m ); }

inline V4 Equal( V4 a, V4 b ) { return FromMask( vceqq_f32( a, b ) ); }
inline V4 NotEqual( V4 a, V4 b ) { return FromMask( vmvnq_u32( vceqq_f32( a, b ) ) ); }
inline V4 Less( V4 a, V4 b ) { return FromMask( vcltq_f32( a, b ) ); }
inline V4 Greater( V4 a, V4 b ) { return FromMask( vcgtq_f32( a, b ) ); }
inline V4 Or( V4 a, V4 b ) { return FromMask( vorrq_u32( ToMask( a ), ToMask( b ) ) ); }
inline V4 And( V4 a, V4 b ) { return FromMask( vandq_u32( ToMask( a ), ToMask( b ) ) ); }
inline V4 AndNot( V4 a, V4 b ) { return FromMask( vbicq_u32( ToMask( a ), ToMask( b ) ) ); }
inline V4 Select( V4 mask, V4 a, V4 b ) { return vbslq_f32( ToMask( mask ), a, b ); }
inline bool Any( V4 mask ) { return 0 != vmaxvq_u32( ToMask( mask ) ); }
inline V4 MaskFromBools( bool a, bool b, bool c, bool d )
{
	uint32_t m[4] = { a ? ~0U : 0U, b ? ~0U : 0U, c ? ~0U : 0U, d ? ~0U : 0U };
	return FromMask( vld1q_u32( m ) );
}

inline void PackBytes( uint32_t out[4], V4 r, V4 g, V4 b, V4 a )
{
	const uint32x4_t kByte = vdupq_n_u32( 0xFF );
	uint32x4_t ri = vandq_u32( vreinterpretq_u32_s32( vcvtq_s32_f32( r ) ), kByte );
	uint32x4_t gi = vandq_u32( vreinterpretq_u32_s32( vcvtq_s32_f32( g ) ), kByte );
	uint32x4_t bi = vandq_u32( vreinterpretq_u32_s32( vcvtq_s32_f32( b ) ), kByte );
	uint32x4_t ai = vandq_u32( vreinterpretq_u32_s32( vcvtq_s32_f32( a ) ), kByte );
	uint32x4_t packed = vorrq_u32( vorrq_u32( ri, vshlq_n_u32( gi, 8 ) ), vorrq_u32( vshlq_n_u32( bi, 16 ), vshlq_n_u32( ai, 24 ) ) );
	vst1q_u32( out, packed );
}

typedef int32x4_t V4i;

inline V4 Xor( V4 a, V4 b ) { return FromMask( veorq_u32( ToMask( a ), ToMask( b ) ) ); }
inline V4i TruncateToInt( V4 a ) { return vcvtq_s32_f32( a ); }
inline V4 IntToFloat( V4i a ) { return vcvtq_f32_s32( a ); }
inline V4i SplatInt( int x ) { return vdupq_n_s32( x ); }
inline V4i AddInt( V4i a, V4i b ) { return vaddq_s32( a, b ); }
inline V4i SubInt( V4i a, V4i b ) { return vsubq_s32( a, b ); }
inline V4i AndInt( V4i a, V4i b ) { return vandq_s32( a, b ); }
inline V4 EqualInt( V4i a, V4i b ) { return FromMask( vceqq_s32( a, b ) ); }
inline V4 BitsToFloat( V4i a ) { return vreinterpretq_f32_s32( a ); }
template < int kShift > inline V4i ShiftLeftInt( V4i a ) { return vshlq_n_s32( a, kShift ); }

#elif defined( Rtt_EMITTER_SIMD_WASM )

typedef v128_t V4;

inline V4 Load( const float *p ) { return wasm_v128_load( p ); }
inline void Store( float *p, V4 v ) { wasm_v128_store( p, v ); }
inline V4 Splat( float x ) { return wasm_f32x4_splat( x ); }

inline V4 Add( V4 a, V4 b ) { return wasm_f32x4_add( a, b ); }
inline V4 Sub( V4 a, V4 b ) { return wasm_f32x4_sub( a, b ); }
inline V4 Mul( V4 a, V4 b ) { return wasm_f32x4_mul( a, b ); }
inline V4 Div( V4 a, V4 b ) { return wasm_f32x4_div( a, b ); }
inline V4 Sqrt( V4 a ) { return wasm_f32x4_sqrt( a ); }
inline V4 Neg( V4 a ) { return wasm_f32x4_neg( a ); }

inline V4 Equal( V4 a, V4 b ) { return wasm_f32x4_eq( a, b ); }
inline V4 NotEqual( V4 a, V4 b ) { return wasm_f32x4_ne( a, b ); }
inline V4 Less( V4 a, V4 b ) { return wasm_f32x4_lt( a, b ); }
inline V4 Greater( V4 a, V4 b ) { return wasm_f32x4_gt( a, b ); }
inline V4 Or( V4 a, V4 b ) { return wasm_v128_or( a, b ); }
inline V4 And( V4 a, V4 b ) { return wasm_v128_and( a, b ); }
inline V4 AndNot( V4 a, V4 b ) { return wasm_v128_andnot( a, b ); }
inline V4 Select( V4 mask, V4 a, V4 b ) { return wasm_v128_bitselect( a, b, mask ); }
inline bool Any( V4 mask ) { return wasm_v128_any_true( mask ); }
inline V4 MaskFromBools( bool a, bool b, bool c, bool d ) { return wasm_i32x4_make( -(int)a, -(int)b, -(int)c, -(int)d ); }

inline void PackBytes( uint32_t out[4], V4 r, V4 g, V4 b, V4 a )
{
	const v128_t kByte = wasm_i32x4_splat( 0xFF );
	v128_t ri = wasm_v128_and( wasm_i32x4_trunc_sat_f32x4( r ), kByte );
	v128_t gi = wasm_v128_and( wasm_i32x4_trunc_sat_f32x4( g ), kByte );
	v128_t bi = wasm_v128_and( wasm_i32x4_trunc_sat_f32x4( b ), kByte );
	v128_t ai = wasm_v128_and( wasm_i32x4_trunc_sat_f32x4( a ), kByte );
	v128_t packed = wasm_v128_or( wasm_v128_or( ri, wasm_i32x4_shl( gi, 8 ) ), wasm_v128_or( wasm_i32x4_shl( bi, 16 ), wasm_i32x4_shl( ai, 24 ) ) );
	wasm_v128_store( out, packed );
}

typedef v128_t V4i; // The same type as V4, so these have their own names.

inline V4 Xor( V4 a, V4 b ) { return wasm_v128_xor( a, b ); }
inline V4i TruncateToInt( V4 a ) { return wasm_i32x4_trunc_sat_f32x4( a ); }
inline V4 IntToFloat( V4i a ) { return wasm_f32x4_convert_i32x4( a ); }
inline V4i SplatInt( int x ) { return wasm_i32x4_splat( x ); }
inline V4i AddInt( V4i a, V4i b ) { return wasm_i32x4_add( a, b ); }
inline V4i SubInt( V4i a, V4i b ) { return wasm_i32x4_sub( a, b ); }
inline V4i AndInt( V4i a, V4i b ) { return wasm_v128_and( a, b ); }
inline V4 EqualInt( V4i a, V4i b ) { return wasm_i32x4_eq( a, b ); }
inline V4 BitsToFloat( V4i a ) { return a; }
template < int kShift > inline V4i ShiftLeftInt( V4i a ) { return wasm_i32x4_shl( a, kShift ); }

#endif

// max( 0, x ) exactly as std::max( 0.0f, x ) evaluates it: ( 0 < x ) ? x : 0.
inline V4 MaxZero( V4 x )
{
	V4 zero = Splat( 0.0f );

	return Select( Greater( x, zero ), x, zero );
}


// Sine and cosine of each lane, with Cephes' single precision method: reduce x
// by multiples of pi / 4, then evaluate a sine or cosine polynomial. This is
// NOT bit-identical to sinf() / cosf(): errors are within a few units in the
// last place. Lanes beyond the reduction's accurate range, or not finite, use
// sinf() / cosf().
inline void SinCos( V4 x, V4 &sines, V4 &cosines )
{
	const V4 kSignBit = BitsToFloat( SplatInt( (int)0x80000000 ) );
	const float kMaxReduced = 8192.0f;

	V4 sinSign = And( x, kSignBit );
	V4 ax = AndNot( x, kSignBit ); // |x|

	// j = the even octant nearest above |x| * 4 / pi.
	V4i j = TruncateToInt( Mul( ax, Splat( 1.27323954473516f ) ) );

	j = AndInt( AddInt( j, SplatInt( 1 ) ), SplatInt( ~1 ) );

	V4 y = IntToFloat( j );

	// Sign of the results, and whether sine and cosine swap polynomials.
	sinSign = Xor( sinSign, BitsToFloat( ShiftLeftInt< 29 >( AndInt( j, SplatInt( 4 ) ) ) ) );

	V4 cosSign = BitsToFloat( ShiftLeftInt< 29 >( AndInt( SubInt( SplatInt( 4 ), AndInt( SubInt( j, SplatInt( 2 ) ), SplatInt( 4 ) ) ), SplatInt( 4 ) ) ) );
	V4 useSinPoly = EqualInt( AndInt( j, SplatInt( 2 ) ), SplatInt( 0 ) );

	// Extended precision reduction: |x| - j * pi / 4.
	V4 r = Sub( ax, Mul( y, Splat( 0.78515625f ) ) );

	r = Sub( r, Mul( y, Splat( 2.4187564849853515625e-4f ) ) );
	r = Sub( r, Mul( y, Splat( 3.77489497744594108e-8f ) ) );

	V4 z = Mul( r, r );

	V4 cosPoly = Splat( 2.443315711809948e-5f );

	cosPoly = Add( Mul( cosPoly, z ), Splat( -1.388731625493765e-3f ) );
	cosPoly = Add( Mul( cosPoly, z ), Splat( 4.166664568298827e-2f ) );
	cosPoly = Mul( Mul( cosPoly, z ), z );
	cosPoly = Add( Sub( cosPoly, Mul( z, Splat( 0.5f ) ) ), Splat( 1.0f ) );

	V4 sinPoly = Splat( -1.9515295891e-4f );

	sinPoly = Add( Mul( sinPoly, z ), Splat( 8.3321608736e-3f ) );
	sinPoly = Add( Mul( sinPoly, z ), Splat( -1.6666654611e-1f ) );
	sinPoly = Add( Mul( Mul( sinPoly, z ), r ), r );

	sines = Xor( Select( useSinPoly, sinPoly, cosPoly ), sinSign );
	cosines = Xor( Select( useSinPoly, cosPoly, sinPoly ), cosSign );

	// !( |x| < max ) is also true for infinities and NaNs.
	V4 isInRange = Less( ax, Splat( kMaxReduced ) );

	V4 zero = Splat( 0.0f );

	if ( Any( AndNot( Equal( zero, zero ), isInRange ) ) ) // any lane not in range
	{
		float xs[4], s[4], c[4];

		Store( xs, x );
		Store( s, sines );
		Store( c, cosines );

		for ( int i = 0; i < 4; ++i )
		{
			if ( !( fabsf( xs[i] ) < kMaxReduced ) )
			{
				s[i] = sinf( xs[i] );
				c[i] = cosf( xs[i] );
			}
		}

		sines = Load( s );
		cosines = Load( c );
	}
}

// ----------------------------------------------------------------------------

} // namespace EmitterSIMD

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_EMITTER_SIMD

#endif // _Rtt_EmitterSIMD_H__
