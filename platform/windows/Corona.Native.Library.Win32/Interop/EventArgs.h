//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
