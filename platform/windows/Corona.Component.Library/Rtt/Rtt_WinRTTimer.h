// ----------------------------------------------------------------------------
// 
// Rtt_WinRTTimer.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_Build.h"
#	include "Rtt_PlatformTimer.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {
	interface class ITimerServices;
	interface class ITimer;
} } } }
namespace CoronaLabs { namespace WinRT {
	ref class EmptyEventArgs;
} }

#pragma endregion


namespace Rtt
{

class WinRTTimer : public PlatformTimer
{
	Rtt_CLASS_NO_COPIES(WinRTTimer)

	public:
		/// <summary>Creates a new Corona timer wrapping the platform specific timer implementation.</summary>
		/// <param name="callback">The callback to be invoked every time this timer elapses.</param>
		/// <param name="timerServices">Creates a WinRT timer that this Corona timer will wrap.</param>
		WinRTTimer(MCallback& callback, CoronaLabs::Corona::WinRT::Interop::ITimerServices^ timerServices);

		/// <summary>Destroys the WinRT timer owned by this object.</summary>
		virtual ~WinRTTimer();

		/// <summary>Starts this timer with the intveral given to the SetInterval() function.</summary>
		virtual void Start();

		/// <summary>Stops this timer if running.</summary>
		virtual void Stop();

		/// <summary>Sets the interval to be used by this timer.</summary>
		/// <remarks>Changing the interval while the timer is already running will cause it to be restarted.</remarks>
		/// <param name="milliseconds">
		///  <para>The amount of time in milliseconds this timer will raise Elapsed events.</para>
		///  <para>Cannot be set to a value less than 1 millisecond.</para>
		/// </param>
		virtual void SetInterval(U32 milliseconds);

		/// <summary>Determines if this timer is currently running.</summary>
		/// <returns>
		///  <para>Returns true if the timer is currently running.</para>
		///  <para>Returns false if stopped, meaning the given callback will no longer be invoked.</para>
		/// </returns>
		virtual bool IsRunning() const;

	private:
		ref class EventHandler sealed
		{
			internal:
				/// <summary>
				///  Creates a new handler which receives the WinRT timer's "Elapsed" events and invokes the
				///  Corona timer's callback.
				/// </summary>
				/// <param name="timerPointer">
				///  Pointer to the timer object that provides WinRT implementation of the timer.
				/// </param>
				EventHandler(WinRTTimer *timerPointer);
				
				/// <summary>Subscribes to the timer's "Elapsed" event.</summary>
				void Subscribe();

				/// <summary>Unsubscribes from the timer's "Elapsed" events.</summary>
				void Unsubscribe();

			private:
				/// <summary>Called when the WinRT implemented timer elapses. Invokes the Corona timer's callback.</summary>
				/// <param name="sender">The timer that raised this event.</param>
				/// <param name="args">Empty event arguments.</param>
				void OnTimerElapsed(CoronaLabs::Corona::WinRT::Interop::ITimer^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

				WinRTTimer* fTimerPointer;
				Windows::Foundation::EventRegistrationToken fEventToken;
		};

		CoronaLabs::Corona::WinRT::Interop::ITimer^ fTimer;
		WinRTTimer::EventHandler^ fEventHandler;
};

}	// namespace Rtt
