// ----------------------------------------------------------------------------
// 
// IDispatcher.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

/// <summary>Interface used to handle the main UI thread's message/event queue.</summary>
/// <remarks>
///  This dispatcher is expected to be used by other threads to post operations to be invoked on the main UI thread.
/// </remarks>
[Windows::Foundation::Metadata::WebHostHidden]
public interface class IDispatcher
{
	/// <summary>Processes all UI events in the dispatcher's queue.</summary>
	/// <remarks>This method blocks until all events in the queue have been processed.</remarks>
//	void ProcessEvents();

	/// <summary>Invokes the given delegate immediately on the main UI thread.</summary>
	/// <remarks>
	///  This method can be called from any thread. This method will block until the execution of the
	///  given delegate completes on the main UI thread.
	/// </remarks>
	/// <param name="handler">The delegate to be called on the main UI thread. Cannot be null.</param>
	void Invoke(Windows::UI::Core::DispatchedHandler^ handler);

	/// <summary>
	///  Adds the given delegate to the end of the event queue to be executed later by the main UI thread.
	/// </summary>
	/// <remarks>This is a non-blocking call.</remarks>
	/// <param name="handler">The delegate to be called on the main UI thread. Cannot be null.</param>
	void InvokeAsync(Windows::UI::Core::DispatchedHandler^ handler);
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
