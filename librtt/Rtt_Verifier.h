//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Verifier_H__
#define _Rtt_Verifier_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class Verifier
{
/*
	public:
		typedef enum _Type
		{
			kApplication = 0,
			kSubscription,

			kNumTypes
		}
		Type;
*/

	public:
		static bool IsValidApplication( const char* filename );
		static bool IsValidSubscription( const char* filename );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Verifier_H__
