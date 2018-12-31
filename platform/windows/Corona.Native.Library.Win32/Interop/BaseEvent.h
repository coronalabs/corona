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

/// <summary>
///  <para>Abstract class from which all Events derive from.</para>
///  <para>
///   Since the main Event&lt;&gt; class is a template, this base class allows you to store a collection
///   of different templatized event types and handler types via polymorphism.
///  </para>
/// </summary>
class BaseEvent
{
	public:
		/// <summary>Destroys this event and its resources.</summary>
		virtual ~BaseEvent();

		#pragma region Handler Class
		/// <summary>
		///  <para>Stores a callback to be invoked when an event gets raised.</para>
		///  <para>
		///   This is an abstract class from which the FunctionHandler and MethodHandler classes derive from.
		///   Instances of those classes are expected to be created and passed into the Event::HandlerManager
		///   class' Add() method so that they'll be invoked when the event gets raised.
		///  </para>
		/// </summary>
		class Handler
		{
			protected:
				/// <summary>Constructor made private to prevent instances from being made.</summary>
				Handler();

				/// <summary>Constructor made private to prevent instances from being made.</summary>
				Handler(const Handler& event);

			public:
				/// <summary>Destroys this event handler.</summary>
				virtual ~Handler();
		};

		#pragma endregion


	protected:
		/// <summary>Constructor made private to prevent instances from being made.</summary>
		BaseEvent();

		/// <summary>Constructor made private to prevent instances from being made.</summary>
		BaseEvent(const BaseEvent& event);
};

}	// namespace Interop
