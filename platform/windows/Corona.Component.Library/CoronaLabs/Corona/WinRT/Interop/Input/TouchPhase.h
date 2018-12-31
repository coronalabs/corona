// ----------------------------------------------------------------------------
// 
// TouchPhase.h
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
#pragma warning(disable: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)
#	include "Rtt_Build.h"
#	include "Rtt_Event.h"
#pragma warning(default: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

/// <summary>
///  Indicates if the touch input has began, moved/dragged, ended, or has been canceled.
///  Provides unique IDs matching Corona's string and integer constants for easy conversion.
/// </summary>
/// <remarks>
///  <para>
///   You cannot create instances of this class. Instead, you access pre-defined TouchPhase objects
///   via this class' static properties like you would with an enum.
///  </para>
///  <para>Instances of this class are immutable.</para>
/// </remarks>
public ref class TouchPhase sealed
{
	private:
		/// <summary>Creates a new touch phase object with the given identifier.</summary>
		/// <remarks>
		///  This is a private constructor that is used to create the predefined touch phase objects that
		///  this class makes available publicly via its static properties.
		/// </remarks>
		/// <param name="coronaIntegerId">Corona's unique integer ID for this touch phase such as kBegan, kMoved, etc.</param>
		TouchPhase(Rtt::TouchEvent::Phase coronaIntegerId);

	public:
		/// <summary>Gets Corona's unique integer ID for this touch phase type.</summary>
		/// <value>Corona's unique integer ID for this touch phase.</value>
		property int CoronaIntegerId { int get(); }

		/// <summary>Gets Corona's unique string ID for this touch phase type.</summary>
		/// <value>
		///  Corona's unique string ID for this touch phase type such as "began", "moved", "ended", etc.
		///  Used by touch events in Lua scripts.
		/// </value>
		property Platform::String^ CoronaStringId { Platform::String^ get(); }

		/// <summary>Gets Corona's unique string ID for this touch phase type.</summary>
		/// <returns>
		///  Returns Corona's unique string ID for this touch phase type such as "began", "moved", "ended", etc.
		///  Used by touch events in Lua scripts.
		/// </returns>
		Platform::String^ ToString();

		/// <summary>Indicates that touch input has began.</summary>
		static property TouchPhase^ Began { TouchPhase^ get(); }

		/// <summary>Indicates that the touch input pointer has changed coordinates.</summary>
		static property TouchPhase^ Moved { TouchPhase^ get(); }

		/// <summary>Indicates that touch input has ended.</summary>
		static property TouchPhase^ Ended { TouchPhase^ get(); }

		/// <summary>Indicates that the system has aborted tracking touch input.</summary>
		static property TouchPhase^ Canceled { TouchPhase^ get(); }

		/// <summary>Gets a read-only collection of all touch phases provided by this class.</summary>
		/// <value>Read-only collection of all touch phases provided by this class.</value>
		static property Windows::Foundation::Collections::IIterable<TouchPhase^>^ Collection
		{
			Windows::Foundation::Collections::IIterable<TouchPhase^>^ get();
		}

		/// <summary>Gets a touch phase matching the given Corona string ID.</summary>
		/// <param name="stringId">
		///  Unique string ID for the touch phase such as "began", "moved", "ended", or "canceled".
		///  This string ID is typically used by touch events in Lua.
		/// </param>
		/// <returns>
		///  <para>Returns the touch phase object matching the given string ID.</para>
		///  <para>Returns null if the given string ID is unknown.</para>
		/// </returns>
		static TouchPhase^ FromCoronaStringId(Platform::String^ stringId);

		/// <summary>Gets a touch phase matching the given unique Corona integer ID.</summary>
		/// <param name="value">Corona's unique integer ID for the touch phase.</param>
		/// <returns>
		///  <para>Returns the touch phase object matching the given integer ID.</para>
		///  <para>Returns null if the given integer ID is unknown.</para>
		/// </returns>
		static TouchPhase^ FromCoronaIntegerId(int value);

	internal:
		/// <summary>Gets Corona's unique integer ID matching a constant in the Rtt::TouchEvent::Phase enum.</summary>
		/// <value>Corona's unique integer ID for this touch phase.</value>
		property Rtt::TouchEvent::Phase CoronaTouchEventPhase { Rtt::TouchEvent::Phase get(); }

		/// <summary>Gets a touch phase matching the given Corona string ID.</summary>
		/// <param name="stringId">
		///  Unique string ID for the touch phase such as "began", "moved", "ended", or "canceled".
		///  This string ID is typically used by touch events in Lua.
		/// </param>
		/// <returns>
		///  <para>Returns the touch phase object matching the given string ID.</para>
		///  <para>Returns null if the given string ID is unknown.</para>
		/// </returns>
		static TouchPhase^ FromCoronaStringId(const char *stringId);

		/// <summary>Gets a touch phase matching the given unique Corona integer ID.</summary>
		/// <param name="value">Corona's unique integer ID for the touch phase.</param>
		/// <returns>
		///  <para>Returns the touch phase object matching the given integer ID.</para>
		///  <para>Returns null if the given integer ID is unknown.</para>
		/// </returns>
		static TouchPhase^ From(const Rtt::TouchEvent::Phase value);

	private:
		/// <summary>Gets a private mutable collection used to store all touch phase types provided by this class.</summary>
		/// <remarks>
		///  This is used by this class' constructor to automatically store all pre-defined touch phase objects
		///  to be used by this class' static From() methods to do the equivalent of a Java/.NET reflection lookup
		///  for pre-existing objects.
		/// </remarks>
		static property Platform::Collections::Vector<TouchPhase^>^ MutableCollection
		{
			Platform::Collections::Vector<TouchPhase^>^ get();
		}

		static const TouchPhase^ kBegan;
		static const TouchPhase^ kMoved;
		static const TouchPhase^ kEnded;
		static const TouchPhase^ kCanceled;

		Rtt::TouchEvent::Phase fCoronaTouchEventPhase;
		Platform::String^ fCoronaStringId;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
