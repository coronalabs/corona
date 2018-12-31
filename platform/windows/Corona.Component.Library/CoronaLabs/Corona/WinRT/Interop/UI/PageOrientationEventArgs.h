// ----------------------------------------------------------------------------
// 
// PageOrientationEventArgs.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "CoronaLabs\Corona\WinRT\Interop\UI\PageOrientation.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

/// <summary>Provides information related to a page orientation event.</summary>
public ref class PageOrientationEventArgs sealed
{
	public:
		/// <summary>Creates a new object for storing an orientation event's information.</summary>
		/// <param name="orientation">
		///  <para>The orientation associated with the event.</para>
		///  <para>Cannot be set to null or else an exception will be thrown.</para>
		/// </param>
		PageOrientationEventArgs(PageOrientation^ orientation);

		/// <summary>Gets the event's orientation.</summary>
		/// <value>The orientation provided by the event.</value>
		property PageOrientation^ Orientation { PageOrientation^ get(); }

	private:
		PageOrientation^ fOrientation;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
