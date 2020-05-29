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


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

interface class IControlAdapter;
interface class IMessageBox;

/// <summary>Interface used to create UI controls and wrapping them in cross-platform/language adapters.</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public interface class IUserInterfaceServices
{
	/// <summary>
	///  <para>Gets a fractional scale factor used to convert the UI's coordinates to pixels.</para>
	///  <para>This scale is intented to be multiplied against a UI coordinate value to convert it to pixels.</para>
	///  <para>
	///   For Windows Phone apps, this is retrieved from "System.Windows.Application.Current.Host.Content.ScaleFactor",
	///   which doesn't always match property "Windows.Graphics.Display.DisplayProperties.ResolutionScale".
	///  </para>
	/// </summary>
	/// <value>
	///  <para>A fractional scale used to convert the UI's coordinates to pixels.</para>
	///  <para>Returns 1.0 if the UI's coordinates are not scaled.</para>
	/// </value>
	property double ScaleFactor { double get(); }

	/// <summary>
	///  <para>Wraps the given UI control with an adapter.</para>
	///  <para>This makes it accessible to Corona on the C++/CX side in a cross-platform and cross-language manner.</para>
	/// </summary>
	/// <param name="control">
	///  <para>Reference to an existing control to be wrapped by a Corona adapter.</para>
	///  <para>For Windows Phone apps, this is expected to derive from the "System.Windows.FrameworkElement" class.</para>
	///  <para>For Windows Universal apps, this is expected to derive from the "Windows.UI.Xaml.FrameworkElement" class.</para>
	/// </param>
	/// <returns>
	///  <para>Returns an new adapter for the given control.</para>
	///  <para>Returns null if the given object is not a UI control object.</para>
	/// </returns>
	IControlAdapter^ CreateAdapterFor(Platform::Object^ control);

	/// <summary>
	///  <para>Creates an interface to a new modal message box dialog.</para>
	///  <para>Used by Corona's Lua native.showAlert() function.</para>
	/// </summary>
	/// <returns>Returns a new message box object to be configured and displayed by the caller.</returns>
	IMessageBox^ CreateMessageBox();

//TODO: Add methods for creating native UI controls here.
//	ITextBoxAdapter^ CreateTextBox();
//	IWebViewAdapter^ CreateWebView();
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
