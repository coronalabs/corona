// ----------------------------------------------------------------------------
// 
// MessageBoxSettings.h
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

#include <collection.h>


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

/// <summary>Stores the configuration used to set up a Corona managed message box.</summary>
public ref class MessageBoxSettings sealed
{
	public:
		/// <summary>Creates a new message box configuration initialized to to empty strings.</summary>
		MessageBoxSettings();

		/// <summary>Copies the given settings to this object.</summary>
		/// <param name="settings">
		///  <para>Reference to the settings object to copy from.</para>
		///  <para>Will be ignored if set to null.</para>
		/// </param>
		void CopyFrom(MessageBoxSettings^ settings);

		/// <summary>Gets or sets the message box's optional title text.</summary>
		/// <value>
		///  <para>The title text to be displayed at the top of the message box.</para>
		///  <para>Set to empty string to not display a title in the message box.</para>
		/// </value>
		property Platform::String^ Title;

		/// <summary>Gets or sets the main text to be displayed in the message box.</summary>
		/// <value>
		///  <para>The main text to be displayed within the message box.</para>
		///  <para>Set to empty string to not display text within the message box.</para>
		/// </value>
		property Platform::String^ Message;

		/// <summary>Gets a modifiable collection of labels to be displayed as buttons in the message box.</summary>
		/// <value>
		///  <para>Modifiable collection of button labels to be displayed as button in the message box.</para>
		///  <para>
		///   Can be empty, which means that the message box will not display any buttons and can only be closed
		///   by the end-user via the back key.
		///  </para>
		/// </value>
		property Windows::Foundation::Collections::IVector<Platform::String^>^ ButtonLabels
		{
			Windows::Foundation::Collections::IVector<Platform::String^>^ get();
		}

	private:
		/// <summary>Collection of button labels/text to be turned into buttons in the displayed message box.</summary>
		Platform::Collections::Vector<Platform::String^>^ fButtonLabels;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
