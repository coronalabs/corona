// ----------------------------------------------------------------------------
// 
// PageProxy.h
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

#include "IPage.h"
#include "PageOrientation.h"
#include "PageOrientationEventArgs.h"
#include "CoronaLabs\WinRT\CancelEventArgs.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

/// <summary>
///  Proxy which passes through all events, commands, and property accessors to an actual application page
///  used to host UI controls.
/// </summary>
/// <remarks>
///  The main intention of this proxy is to safely handle the case where a UI control can be removed from a page
///  and then added to the page (or a different page) later. In this case, the control's page reference would be
///  be null, meaning the system should no longer attempt to access that lost page's properties and should unsubsribe
///  from it events. However, with this class, the system can always safely/easily access the page's properties
///  through this proxy which serves as a pass-through and will provide default values when the page reference has
///  been lost.
/// </remarks>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class PageProxy sealed : public IPage
{
	public:
		#pragma region Events
		/// <summary>Raised when the application page's orientation has changed.</summary>
		virtual event Windows::Foundation::TypedEventHandler<IPage^, PageOrientationEventArgs^>^ OrientationChanged;

		/// <summary>Raised when the end-user attempts to back out of the page.</summary>
		/// <remarks>
		///  If you want to prevent the end-user from navigating back, then you should set the event argument's
		///  Cancel property to true.
		/// </remarks>
		virtual event Windows::Foundation::TypedEventHandler<IPage^, CoronaLabs::WinRT::CancelEventArgs^>^ NavigatingBack;

		/// <summary>Raised when this proxy has been assigned a page.</summary>
		event Windows::Foundation::TypedEventHandler<PageProxy^, CoronaLabs::WinRT::EmptyEventArgs^>^ ReceivedPage;

		/// <summary>Raised when this proxy's assigned page is about to be lost.</summary>
		event Windows::Foundation::TypedEventHandler<PageProxy^, CoronaLabs::WinRT::EmptyEventArgs^>^ LosingPage;

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new proxy used to safely reference one page.</summary>
		PageProxy();

		#pragma endregion


		#pragma region Public Methods/Properties
		/// <summary>Gets or sets the application page that this proxy references.</summary>
		/// <value>
		///  <para>The application page that this proxy references.</para>
		///  <para>Set to null if the proxy does not currently reference a page.</para>
		/// </value>
		property IPage^ Page { IPage^ get(); void set(IPage^ page); }

		/// <summary>Gets the application page's current orientation.</summary>
		/// <value>The application page's current orientation such as portrait, landscape, etc.</value>
		virtual property PageOrientation^ Orientation { PageOrientation^ get(); }

		/// <summary>Commands the page to navigate back to the previous page.</summary>
		/// <remarks>This method is expected to be called when Corona's native.requestExit() function gets called in Lua.</remarks>
		virtual void NavigateBack();

		/// <summary>
		///  <para>Navigates to the application page that the given URI's scheme applies to.</para>
		///  <para>
		///   For example, an "http://" URI will launch the browser app and a "mailto://" scheme will launch the mail app.
		///  </para>
		/// </summary>
		/// <remarks>This method is called by Corona's system.openURL() Lua function.</remarks>
		/// <param name="uri">The URI/URL of the application page to navigate to.</param>
		/// <returns>
		///  <para>Returns true if able to display an application page for the given URI.</para>
		///  <para>Returns false if the given URI was not recognized and has failed to navigate to another page.</para>
		/// </returns>
		virtual bool NavigateTo(Windows::Foundation::Uri^ uri);

		/// <summary>Determines if the given URI will work when passed to the NavigateTo() method.</summary>
		/// <remarks>This method is called by Corona's system.canOpenURL() Lua function.</remarks>
		/// <param name="uri">The URI/URL of the application page to navigate to.</param>
		/// <returns>
		///  <para>Returns true if able to display an application page for the given URI.</para>
		///  <para>Returns false if the given URI cannot open a page or is invalid.</para>
		/// </returns>
		virtual bool CanNavigateTo(Windows::Foundation::Uri^ uri);

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Subscribes to the "fPage" member variable's events.</summary>
		void AddEventHandlers();

		/// <summary>Unsubscribes from the "fPage" member variable's events.</summary>
		void RemoveEventHandlers();

		/// <summary>
		///  <para>Called when this proxy's assigned page raises an "OrientationChanged" event.</para>
		///  <para>Relays the event to the handlers assigned to this proxy.</para>
		/// </summary>
		/// <param name="sender">The page that raised this event.</param>
		/// <param name="args">Provides the page's newly assigned orientation.</param>
		void OnOrientationChanged(IPage^ sender, PageOrientationEventArgs^ args);
		
		/// <summary>
		///  <para>Called when this proxy's assigned page raises a "NavigatingBack" event.</para>
		///  <para>Relays the event to the handlers assigned to this proxy.</para>
		/// </summary>
		/// <param name="sender">The page that raised this event.</param>
		/// <param name="args">Provides a Cancel argument that the event handler can set true to block navigation.</param>
		void OnNavigatingBack(IPage^ sender, CoronaLabs::WinRT::CancelEventArgs^ args);
		
		#pragma endregion


		#pragma region Private Member Variables.
		/// <summary>The page that this proxy currently references.</summary>
		IPage^ fPage;

		/// <summary>Last orientation received from the page.</summary>
		PageOrientation^ fLastOrientation;

		/// <summary>Token received when subscribing to the referenced page's "OrientationChanged" event.</summary>
		Windows::Foundation::EventRegistrationToken fOrientationChangedEventToken;

		/// <summary>Token received when subscribing to the referenced page's "NavigatingBack" event.</summary>
		Windows::Foundation::EventRegistrationToken fNavigatingBackEventToken;

		#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
