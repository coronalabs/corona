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

#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include "PageProxy.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

interface class IDispatcher;

/// <summary>Interface to a user interface control.</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public interface class IControlAdapter
{
	#pragma region Events
	/// <summary>
	///  <para>Raised when the control has been added to the page and is about to be displayed.</para>
	///  <para>Handler is expected to finish initialization with this event before the control is rendered onscreen.</para>
	/// </summary>
	event Windows::Foundation::TypedEventHandler<IControlAdapter^, CoronaLabs::WinRT::EmptyEventArgs^>^ Loaded;

	/// <summary>
	///  <para>Raised when the control has been removed from the page and is no longer visible onscreen.</para>
	///  <para>Handler is expected to release any allocated resources that was created via the Loaded event.</para>
	/// </summary>
	event Windows::Foundation::TypedEventHandler<IControlAdapter^, CoronaLabs::WinRT::EmptyEventArgs^>^ Unloaded;

	/// <summary>Raised when the width and/or height of the control has changed.</summary>
	event Windows::Foundation::TypedEventHandler<IControlAdapter^, CoronaLabs::WinRT::EmptyEventArgs^>^ Resized;

	#pragma endregion


	#pragma region Methods/Properties
	/// <summary>Gets or sets the width of the control in pixels.</summary>
	/// <value>
	///  <para>The width of the control in pixels.</para>
	///  <para>Must be set to a value greater than or equal to zero.</para>
	///  <para>Changing this property will cause a "Resized" event to be raised.</para>
	/// </value>
	property int Width;

	/// <summary>Gets or sets the height of the control in pixels.</summary>
	/// <value>
	///  <para>The height of the control in pixels.</para>
	///  <para>Must be set to a value greater than or equal to zero.</para>
	///  <para>Changing this property will cause a "Resized" event to be raised.</para>
	/// </value>
	property int Height;

	/// <summary>Gets a dispatcher used to queue operations to be executed on the main UI thread.</summary>
	/// <value>The main UI thread dispatcher this control is associated with.</value>
	property IDispatcher^ Dispatcher { IDispatcher^ get(); }

	/// <summary>Gets a proxy to the page that is hosting this control.</summary>
	/// <value>
	///  <para>Gets the page that is hosting this control.</para>
	///  <para>
	///   Note that the returned proxy will not be null when the control is removed from the page.
	///   In this case, the proxy will safely no-op when calling its methods and provide the last
	///   known property values from this control's previously assigned page.
	///  </para>
	/// </value>
	property PageProxy^ ParentPageProxy { PageProxy^ get(); }

	/// <summary>Gets the control that this adapter wraps.</summary>
	/// <remarks>
	///  Provided as a generic object reference to be cross-platform between Windows Phone and
	///  Windows Universal applications which use different UI frameworks.
	/// </remarks>
	/// <value>
	///  <para>The control that this adapter references.</para>
	///  <para>Will be of type "System.Windows.UIElement" for Windows Phone applications.</para>
	///  <para>Will be of type "Windows.UI.Xaml.UIElement" for Universal WinRT applications.</para>
	///  <para>
	///   Will be null if this adapter no longer references a control, which can happen after calling
	///   the <see cref="ReleaseReferencedControl"/> method.
	///  </para>
	/// </value>
	property Platform::Object^ ReferencedControl { Platform::Object^ get(); }

	/// <summary>
	///  <para>Releases the control that this adapter wraps.</para>
	///  <para>This adapter's properties and methods will no longer operate on the control once released.</para>
	/// </summary>
	void ReleaseReferencedControl();

	#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
