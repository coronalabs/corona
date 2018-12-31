// ----------------------------------------------------------------------------
// 
// EmptyEventArgs.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
	#error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace WinRT {

/// <summary>Event arguments class to be used by component class events that do not provide any arguments.</summary>
/// <remarks>
///  <para>This class provides the .NET equivalent of the System.EventArgs.Empty static method.</para>
///  <para>
///   You cannot create instances of this class. Instead, you fetch a pre-allocated instance via the static
///   <see cref="Instance">EmptyEventArgs.Instance</see> property.
///  </para>
/// </remarks>
public ref class EmptyEventArgs sealed
{
	private:
		/// <summary>Creates a new EmptyEventArgs instance.</summary>
		EmptyEventArgs();

	public:
		/// <summary>Provides a pre-allocated empty event arguments objects to be used component class events.</summary>
		/// <remarks>
		///  This property provides a pre-allocated empty event args object. This object should be used by component class
		///  events that do not need to provide any event arguments.
		///  This is the equivalent to a .NET System.EventArgs.Empty object.
		/// </remarks>
		/// <value>An empty event arguments object that can be used by events.</value>
		static property EmptyEventArgs^ Instance { EmptyEventArgs^ get(); }
};

} }	// namespace CoronaLabs::WinRT
