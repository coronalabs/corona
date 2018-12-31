// ----------------------------------------------------------------------------
// 
// BaseEvent.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


namespace AL {

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

}	// namespace AL
