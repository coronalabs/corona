//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


namespace Interop {

/// <summary>Provides information about an event to an event handler.</summary>
/// <remarks>
///  <para>
///   Events which need to pass additional information to their event handlers should derive from this class
///   and pass it as the 2nd argument to the event handler, much like how it works in the .NET framework.
///  </para>
///  <para>
///   An instance of this class is to be used by events that do not provide any event information.
///   In this case, you should pass a reference to this class' pre-allocated kEmpty object to the event handlers,
///   much like how you would use .NET's EventArgs.Empty property.
///  </para>
/// </remarks>
class EventArgs
{
	protected:
		/// <summary>Constructor made protected to prevent instances from being made publicly.</summary>
		EventArgs();

	public:
		/// <summary>Destroys this object.</summary>
		virtual ~EventArgs();

		/// <summary>Empty event arguments object to be used by events that do not provide any event information.</summary>
		static const EventArgs kEmpty;
};

}	// namespace Interop
