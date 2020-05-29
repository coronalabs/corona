//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace WinRT {
	ref class CancelEventArgs;
} }


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

ref class PageOrientationEventArgs;
ref class PageOrientation;

/// <summary>Provides access to the application page's features and events.</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public interface class IPage
{
	#pragma region Events
	/// <summary>Raised when the application page's orientation has changed.</summary>
	event Windows::Foundation::TypedEventHandler<IPage^, PageOrientationEventArgs^>^ OrientationChanged;

	/// <summary>Raised when the end-user attempts to back out of the page.</summary>
	/// <remarks>
	///  If you want to prevent the end-user from navigating back, then you should set the event argument's
	///  Cancel property to true.
	/// </remarks>
	event Windows::Foundation::TypedEventHandler<IPage^, CoronaLabs::WinRT::CancelEventArgs^>^ NavigatingBack;

	#pragma endregion


	#pragma region Methods/Properties
	/// <summary>Gets the application page's current orientation.</summary>
	/// <value>The application page's current orientation such as portrait, landscape, etc.</value>
	property PageOrientation^ Orientation { PageOrientation^ get(); }

	/// <summary>Commands the page to navigate back to the previous page.</summary>
	/// <remarks>This method is expected to be called when Corona's native.requestExit() function gets called in Lua.</remarks>
	void NavigateBack();

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
	bool NavigateTo(Windows::Foundation::Uri^ uri);

	/// <summary>Determines if the given URI will work when passed to the NavigateTo() method.</summary>
	/// <remarks>This method is called by Corona's system.canOpenURL() Lua function.</remarks>
	/// <param name="uri">The URI/URL of the application page to navigate to.</param>
	/// <returns>
	///  <para>Returns true if able to display an application page for the given URI.</para>
	///  <para>Returns false if the given URI cannot open a page or is invalid.</para>
	/// </returns>
	bool CanNavigateTo(Windows::Foundation::Uri^ uri);

//TODO: Should add status bar support here. On WP8, this is controlled via the "Microsoft.Phone.Shell.SystemTray" class.

	#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
