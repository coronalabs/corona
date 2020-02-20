//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneTemplate_H__
#define _Rtt_IPhoneTemplate_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneTemplate
{
	public:
		typedef enum _Property
		{
			kIsCoronaKit = 0x1,
		}
		Property;

	public:
		static void WillLoadMain();
		static bool IsProperty( Property key );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneTemplate_H__
