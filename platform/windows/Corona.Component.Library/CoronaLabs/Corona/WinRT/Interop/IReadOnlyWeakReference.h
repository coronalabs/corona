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


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {

/// <summary>Stores a weak reference to an object and only provides read-only access to it.</summary>
/// <remarks>
///  This interface allows a .NET weak reference implementation to be created to store .NET objects
///  and make them accessible to the C++/CX side of the application. This is needed because .NET objects
///  that do not implement a C++/CX side cannot be stored by Platform::WeakReference C++ objects.
/// </remarks>
public interface class IReadOnlyWeakReference
{
	/// <summary>Fetches the stored reference and returns it as of type "object", if still available.</summary>
	/// <returns>
	///  <para>Returns the stored reference as of type "object".</para>
	///  <para>Returns null if the weak referenced object has been garbage collected.</para>
	/// </returns>
	Platform::Object^ GetAsObject();
};

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop
