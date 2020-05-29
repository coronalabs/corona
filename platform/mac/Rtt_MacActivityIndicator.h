//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacActivityIndicator_H__
#define _Rtt_MacActivityIndicator_H__

// ----------------------------------------------------------------------------

@class SPILDTopLayerView;

namespace Rtt
{
	
	// ----------------------------------------------------------------------------
	
class MacActivityIndicator
{
	public:
		MacActivityIndicator();
		virtual ~MacActivityIndicator();
		void ShowActivityIndicator();
		void HideActivityIndicator();
	
	protected:
		SPILDTopLayerView* fIndicatorView;
};
	
// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacActivityIndicator_H__
