// ----------------------------------------------------------------------------
// 
// PageProxy.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "PageProxy.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

#pragma region Consructors/Destructors
PageProxy::PageProxy()
:	fPage(nullptr),
	fLastOrientation(PageOrientation::Unknown)
{
}

#pragma endregion


#pragma region Public Methods/Properties
IPage^ PageProxy::Page::get()
{
	return fPage;
}

void PageProxy::Page::set(IPage^ page)
{
	// Do not continue if this proxy has already been assigned the given page reference.
	if (page == fPage)
	{
		return;
	}

	// Handle this proxy's last assigned page.
	if (fPage)
	{
		// Notify the system that the currently assigned page is about to be lost.
		LosingPage(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);

		// Remove this proxy's event handlers from the currently assigned page.
		RemoveEventHandlers();

		// Store the page's last known orientation.
		fLastOrientation = fPage->Orientation;
	}

	// Store the given page reference.
	fPage = page;

	// Handle the newly given page reference, if one was assigned (not null).
	if (fPage)
	{
		// Add this proxy's event handlers to the newly assigned page.
		AddEventHandlers();

		// Notify the system that a new page has been assigned to this proxy.
		ReceivedPage(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);

		// Notify the system that the orientation has changed if the given page has a different orientation
		// than the last assigned page's orientation.
		auto currentOrientation = fPage->Orientation;
		if (currentOrientation != fLastOrientation)
		{
			fLastOrientation = currentOrientation;
			OrientationChanged(this, ref new PageOrientationEventArgs(currentOrientation));
		}
	}
}

PageOrientation^ PageProxy::Orientation::get()
{
	return fPage ? fPage->Orientation : fLastOrientation;
}

void PageProxy::NavigateBack()
{
	if (fPage)
	{
		fPage->NavigateBack();
	}
}

bool PageProxy::NavigateTo(Windows::Foundation::Uri^ uri)
{
	if (nullptr == fPage)
	{
		return false;
	}
	return fPage->NavigateTo(uri);
}

bool PageProxy::CanNavigateTo(Windows::Foundation::Uri^ uri)
{
	if (nullptr == fPage)
	{
		return false;
	}
	return fPage->CanNavigateTo(uri);
}

#pragma endregion


#pragma region Private Methods
void PageProxy::AddEventHandlers()
{
	// Do not continue if a page has not been assigned.
	if (!fPage)
	{
		return;
	}

	// Remove the last event handlers if still assigned.
	// Note: This should never be the case.
	RemoveEventHandlers();

	// Add event handlers to this proxy's currently assigned page.
	fOrientationChangedEventToken = fPage->OrientationChanged += ref new Windows::Foundation::TypedEventHandler<IPage^, PageOrientationEventArgs^>(this, &PageProxy::OnOrientationChanged);
	fNavigatingBackEventToken = fPage->NavigatingBack += ref new Windows::Foundation::TypedEventHandler<IPage^, CoronaLabs::WinRT::CancelEventArgs^>(this, &PageProxy::OnNavigatingBack);
}

void PageProxy::RemoveEventHandlers()
{
	// Do not continue if a page has not been assigned.
	if (!fPage)
	{
		return;
	}

	// Remove all event handlers from this proxy's currently assigned page.
	if (fOrientationChangedEventToken.Value)
	{
		fPage->OrientationChanged -= fOrientationChangedEventToken;
		fOrientationChangedEventToken.Value = 0;
	}
	if (fNavigatingBackEventToken.Value)
	{
		fPage->NavigatingBack -= fNavigatingBackEventToken;
		fNavigatingBackEventToken.Value = 0;
	}
}

void PageProxy::OnOrientationChanged(IPage^ sender, PageOrientationEventArgs^ args)
{
	// Store the page's last known orientation.
	fLastOrientation = args->Orientation;

	// Relay the event to this proxy's event handlers.
	OrientationChanged(this, args);
}

void PageProxy::OnNavigatingBack(IPage^ sender, CoronaLabs::WinRT::CancelEventArgs^ args)
{
	// Relay the event to this proxy's event handlers.
	NavigatingBack(this, args);
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
