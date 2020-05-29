//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_Traits_H
#define Rtt_Traits_H

#ifdef __cplusplus

#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Types.h"

// Macros
// ----------------------------------------------------------------------------

#define Rtt_TRAITCONSTANT( expression )				\
	Rtt_CLASSCONSTANT( bool, Value, expression )

#define Rtt_SPECIALIZETRAIT( Trait, T, expression )	\
	template < >									\
	struct Trait< T >								\
	{												\
		Rtt_TRAITCONSTANT( expression );			\
	}

#define Rtt_TRAIT_SPECIALIZE( Trait, T, expression )	\
	namespace Traits { Rtt_SPECIALIZETRAIT( Trait, T, expression ); }

// ----------------------------------------------------------------------------

namespace Rtt
{

namespace Traits
{

// ----------------------------------------------------------------------------

// IMPLEMENTATION NOTE:
// We put qualifiers behind T, b/c putting the const qualifier behind
// _any_ type turns that type into const.  If the type is _not_ a ref or ptr,
// then putting it in front, the type similarly is const.  For templates, 
// no distinction is made; however, to remove any confusion, we put qualifiers
// behind T.


// Type Transformations: RemoveConst< T >::Type 
// ----------------------------------------------------------------------------
//
//		RemoveConst			- removes top-level const qualifier	
//		RemoveVolatile		- removes top-level volatile qualifier
//		RemoveQualifiers	- removes top-level const and volatile qualifiers
//		RemovePointer		- removes top-level pointer
//		RemoveReference		- removes top-level reference
//		RemoveBounds		- removes top-level array bounds
//		RemoveAll			- recursively removes qualifiers, pointer, reference,
//								and arrays until none are left
// ----------------------------------------------------------------------------

template < typename T >	struct RemoveConst					{ typedef T Type; };
template < typename T >	struct RemoveConst< T const >		{ typedef T Type; };

template < typename T >	struct RemoveVolatile				{ typedef T Type; };
template < typename T >	struct RemoveVolatile< T volatile >	{ typedef T Type; };

template < typename T > struct RemoveQualifiers
{
	typedef typename RemoveConst< typename RemoveVolatile< T >::Type >::Type Type;
};

template < typename T > struct RemovePointer				{ typedef T Type; };
template < typename T >	struct RemovePointer< T* >			{ typedef T Type; };
template < typename T >	struct RemovePointer< T* const >	{ typedef T Type; };
template < typename T >	struct RemovePointer< T* volatile >	{ typedef T Type; };
template < typename T >	struct RemovePointer< T* const volatile >	{ typedef T Type; };

template < typename T >	struct RemoveReference				{ typedef T Type; };
template < typename T >	struct RemoveReference< T& >		{ typedef T Type; };

template < typename T >	struct RemoveBounds					{ typedef T Type; };
template < typename T >	struct RemoveBounds< T[] >				{ typedef T Type; };
template < typename T, size_t N > struct RemoveBounds< T[N] >	{ typedef T Type; };

template < typename T > struct RemoveAllBounds				{ typedef T Type; };
template < typename T >	struct RemoveAllBounds< T[] >				{ typedef typename RemoveAllBounds< T >::Type Type; };
template < typename T, size_t N > struct RemoveAllBounds< T[N] >	{ typedef typename RemoveAllBounds< T >::Type Type; };

// Private Helpers
// ----------------------------------------------------------------------------

namespace Private
{
	template < typename T >
	struct RemoveOne
	{
		typedef typename RemoveQualifiers<
			typename RemovePointer<
				typename RemoveReference<
					typename RemoveAllBounds< T >::Type
				>::Type
			>::Type
		>::Type Type;
	};

	template < typename T, typename U >
	struct RemoveAll
	{
		typedef typename RemoveAll<
			typename RemoveOne< T >::Type,
			typename RemoveOne< U >::Type
		>::Type Type;
	};

	template < typename T >
	struct RemoveAll< T, T >
	{
		typedef T	Type;
	};

} // Private


template < typename T >
struct RemoveAll
{
	typedef typename Private::RemoveAll<
		typename Private::RemoveOne< T >::Type,
		T
	>::Type	Type;
};


// Type Comparisons: IsSame< T, U >::Value
// ----------------------------------------------------------------------------
//
//		IsSame				- true if T and U are the same type
// ----------------------------------------------------------------------------

template< typename T, typename U > struct IsSame	{ Rtt_TRAITCONSTANT( false ); };
template< typename T > struct IsSame< T, T >		{ Rtt_TRAITCONSTANT( true ); };


// Type Queries: IsConst< T >::Value
// ----------------------------------------------------------------------------
//
//		IsConst				- true if type has top-level const
//		IsVolatile			- true if type has top-level volatile
//	
//		// Note: Types with no signedness will generate compiler errors
//		IsSigned			- true if type is signed
//		IsUnsigned			- true if type is not signed
// ----------------------------------------------------------------------------

template< typename T > struct IsConst		{ Rtt_TRAITCONSTANT( ! ( IsSame< T, typename RemoveConst< T >::Type >::Value ) ); };
template< typename T > struct IsVolatile	{ Rtt_TRAITCONSTANT( ! ( IsSame< T, typename RemoveVolatile< T >::Type >::Value ) ); };

template< typename T > struct IsSigned		{ Rtt_TRAITCONSTANT( T( -1 ) < T( 0 ) ); };
template< typename T > struct IsUnsigned	{ Rtt_TRAITCONSTANT( ! IsSigned< T >::Value ); };


// Type Classification: IsInteger< T >::Value
// ----------------------------------------------------------------------------
// Note: Following type queries are independent of qualifiers.  Only one of
// the 11 queries below will match a particular type.
//		IsInteger
//		IsFloat
//		IsVoid
//		IsPointer
//		IsReference
//		IsMemberPointer
//		IsArray
//		IsEnum			- requires compiler support or client specialization.
//		IsUnion			- requires compiler support or client specialization.
//		IsFunction		- requires compiler support or client specialization.
//		IsClass			- without compiler support or specialization,
//							enums, unions, and functions are matched as class type.
//
//		IsArithmetic	- true if type is integral or floating.
//		IsPrimitive		- true if type is arithmetic or void.
//		IsScalar		- true if type is arithmetic, pointer, member pointer or enum.
//		IsObject		- true for all types except void, reference, and functions.
// ----------------------------------------------------------------------------

// Private Helpers
// ----------------------------------------------------------------------------

namespace Private
{

	template < typename T > struct IsInteger { Rtt_TRAITCONSTANT( false ); };

	Rtt_SPECIALIZETRAIT( IsInteger, bool, true );
	Rtt_SPECIALIZETRAIT( IsInteger, char, true );
	Rtt_SPECIALIZETRAIT( IsInteger, signed char, true );
	Rtt_SPECIALIZETRAIT( IsInteger, unsigned char, true );
#if !defined( _MSC_VER )
	Rtt_SPECIALIZETRAIT( IsInteger, wchar_t, true );
#endif
	Rtt_SPECIALIZETRAIT( IsInteger, short int, true );
//	Rtt_SPECIALIZETRAIT( IsInteger, unsigned short int, true );
	Rtt_SPECIALIZETRAIT( IsInteger, int, true );
	Rtt_SPECIALIZETRAIT( IsInteger, unsigned int, true );
	Rtt_SPECIALIZETRAIT( IsInteger, long int, true );
	Rtt_SPECIALIZETRAIT( IsInteger, unsigned long int, true );

	Rtt_SPECIALIZETRAIT( IsInteger, long long, true );
	Rtt_SPECIALIZETRAIT( IsInteger, unsigned long long, true );

	// ------------------------------------------------------------------------

	template < typename T > struct IsFloat { Rtt_TRAITCONSTANT( false ); };

	Rtt_SPECIALIZETRAIT( IsFloat, float, true );
	Rtt_SPECIALIZETRAIT( IsFloat, double, true );
	Rtt_SPECIALIZETRAIT( IsFloat, long double, true );

	// ------------------------------------------------------------------------

	template < typename T > struct IsVoid { Rtt_TRAITCONSTANT( false ); };

	Rtt_SPECIALIZETRAIT( IsVoid, void, true );

	// ------------------------------------------------------------------------

	template < typename T > struct IsPointer
	{
		Rtt_TRAITCONSTANT( ! ( IsSame< T, typename RemovePointer< T >::Type >::Value ) );
	};

	// ------------------------------------------------------------------------

	template < typename T > struct IsReference
	{
		Rtt_TRAITCONSTANT( ! ( IsSame< T, typename RemoveReference< T >::Type >::Value ) );
	};

	// ------------------------------------------------------------------------

	template < class T > struct IsMemberPointer { Rtt_TRAITCONSTANT( false ); };

	template < class T, typename U > struct IsMemberPointer< U T::* >
	{             
		Rtt_TRAITCONSTANT( true );
	};

	// ------------------------------------------------------------------------

	template < typename T > struct IsArray
	{
		Rtt_TRAITCONSTANT( ! ( IsSame< T, typename RemoveBounds< T >::Type >::Value ) );
	};

	// ------------------------------------------------------------------------

	#if defined( __MWERKS__ ) && ! defined ( Rtt_SYMBIAN_ENV )

		#include <msl_utility>

		template < typename T > struct IsEnum { Rtt_TRAITCONSTANT( Metrowerks::details::is_enum_imp< T >::value ); };
		template < typename T > struct IsUnion { Rtt_TRAITCONSTANT( Metrowerks::details::is_union_imp< T >::value ); };
		template < typename T > struct IsFunction { Rtt_TRAITCONSTANT( Metrowerks::details::is_function_imp< T >::value ); };

	#else

		template < typename T > struct IsEnum { Rtt_TRAITCONSTANT( false ); };
		template < typename T > struct IsUnion { Rtt_TRAITCONSTANT( false ); };
		template < typename T > struct IsFunction { Rtt_TRAITCONSTANT( false ); };

	#endif

	// ------------------------------------------------------------------------

	template < typename T > struct IsClass
	{
		Rtt_TRAITCONSTANT( ! ( IsInteger< T >::Value
								|| IsFloat< T >::Value
								|| IsVoid< T >::Value
								|| IsPointer< T >::Value
								|| IsReference< T >::Value
								|| IsMemberPointer< T >::Value
								|| IsArray< T >::Value
								|| IsEnum< T >::Value
								|| IsUnion< T >::Value )
								|| IsFunction< T >::Value );
	};

} // Private


template < typename T > struct IsInteger
{
	Rtt_TRAITCONSTANT( Private::IsInteger< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsFloat
{
	Rtt_TRAITCONSTANT( Private::IsFloat< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsVoid
{
	Rtt_TRAITCONSTANT( Private::IsVoid< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsPointer
{
	Rtt_TRAITCONSTANT( Private::IsPointer< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsReference
{
	Rtt_TRAITCONSTANT( Private::IsReference< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsMemberPointer
{
	Rtt_TRAITCONSTANT( Private::IsMemberPointer< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsArray
{
	Rtt_TRAITCONSTANT( Private::IsArray< T >::Value );
};

template < typename T > struct IsEnum
{
	Rtt_TRAITCONSTANT( Private::IsEnum< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsUnion
{
	Rtt_TRAITCONSTANT( Private::IsUnion< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsFunction
{
	Rtt_TRAITCONSTANT( Private::IsFunction< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsClass
{
	Rtt_TRAITCONSTANT( Private::IsClass< typename RemoveQualifiers< T >::Type >::Value );
};

template < typename T > struct IsArithmetic
{
	Rtt_TRAITCONSTANT( IsInteger< T >::Value || IsFloat< T >::Value );
};

template < typename T > struct IsPrimitive
{
	Rtt_TRAITCONSTANT( IsArithmetic< T >::Value || IsVoid< T >::Value );
};

template < typename T > struct IsScalar
{
	Rtt_TRAITCONSTANT( IsArithmetic< T >::Value
						|| IsPointer< T >::Value
						|| IsMemberPointer< T >::Value
						|| IsEnum< T >::Value );
};

template < typename T > struct IsObject
{
	Rtt_TRAITCONSTANT( ! ( IsVoid< T >::Value
							|| IsReference< T >::Value
							|| IsFunction< T >::Value ) );
};


// BULLSHIT: fix this -- change to HasTrivialCopyConstructor

// Type Properties: IsInteger< T >::Value
// ----------------------------------------------------------------------------
//		IsBitCopyable	- true if type is primitive
//		IsSuperSub
// ----------------------------------------------------------------------------

template < typename T > struct IsBitCopyable
{
	Rtt_TRAITCONSTANT( IsScalar< T >::Value != 0 );
};

// ----------------------------------------------------------------------------

} // Traits

} // Rtt_

// ----------------------------------------------------------------------------

#endif // __cplusplus

#endif // Rtt_Traits_H


