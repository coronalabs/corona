// ----------------------------------------------------------------------------
// 
// NativeThreadBlocker.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#ifdef CORONALABS_CORONA_API_EXPORT
	#define CORONALABS_CORONA_API __declspec( dllexport )
#else
	#define CORONALABS_CORONA_API __declspec( dllimport )
#endif

#include <atomic>
#include <chrono>


namespace CoronaLabs { namespace WinRT {

/// <summary>
///  <para>Blocks a thread for a given duration or until another thread unblocks by disabling it.</para>
///  <para>This class can be used to synchronize communications between 2 threads.</para>
///  <para>
///   Intended to replace Win32's CreateEvent() and WaitForSingleObject() functions which have about 10 millisecond overhead.
///  </para>
/// </summary>
class CORONALABS_CORONA_API NativeThreadBlocker
{
	public:
		#pragma region Public Result Class
		/// <summary>
	    ///  <para>Result returned by a NativeThreadBlocker::BlockUntil() function.</para>
		///  <para>Indicates if unblocked by another thread or if timed-out, indicating no response from the other thread.</para>
		/// </summary>
		class CORONALABS_CORONA_API Result
		{
			public:
				/// <summary>Determines if the NativeThreadBlocker was unblocked/disabled by another thread.</summary>
				/// <returns>
				///  <para>Returns true if the NativeThreadBlocker was unblocked/disabled by another thread.</para>
				///  <para>Returns false if there was no response from another thread.</para>
				/// </returns>
				bool WasUnblockedManually() const;

				/// <summary>
				///  <para>Determines if the NativeThreadBlocker was unblocked due to a timeout.</para>
				///  <para>This would indicate whether or not another thread responded in time.</para>
				/// </summary>
				/// <returns>
				///  <para>Returns true if other threads did not unblock/disable this NativeThreadBlocker in time.</para>
				///  <para>Returns false if the NativeThreadBlocker was unlbocked/disabled by another thread.</para>
				/// </returns>
				bool HasTimedOut() const;

				/// <summary>
				///  Pre-allocated result object indicating that a NativeThreadBlocker was unblocked/disabled
				///  by another thread.
				/// </summary>
				static const Result kUnblockedManually;

				/// <summary>
				///  <para>Pre-allocated result object indicating that a NativeThreadBlocker was unblocked due to a timeout.</para>
				///  <para>This would indicate whether or not another thread responded in time.</para>
				/// </summary>
				static const Result kTimedOut;

			private:
				/// <summary>Creates a new result object.</summary>
				/// <remarks>Constructor made private to force the caller to use pre-allocated instances of this class.</remarks>
				/// <param name="wasUnblockedManually">
				///  <para>Set true if the NativeThreadBlocker was unblocked/disabled by another thread.</para>
				///  <para>If this parameter is set true, then parameter "hasTimedOut" must be set false.</para>
				/// </param>
				/// <param name="hasTimedOut">
				///  <para>Set true if th NativeThreadBlocker timed-out due to no response from the other thread.</para>
				///  <para>If this parameter is set true, then parameter "wasUnblockedManually" must be set false.</para>
				/// </param>
				Result(bool wasUnblockedManually, bool hasTimedOut);

				/// <summary>Determines if the NativeThreadBlocker was unblocked/disabled by another thread.</summary>
				bool fWasUnblockedManually;

				/// <summary>Determins if the NativeThreadBlocker was unblocked due to a timeout.</summary>
				bool fHasTimedOut;
		};
		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new thread blocking object.</summary>
		NativeThreadBlocker();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>
		///   Blocks the current thread infinitely until another thread unblocks it by calling this object's
		///   Disable() method.
		///  </para>
		///  <para>This method will immediately return if this blocker object is currently disabled.</para>
		/// </summary>
		void Block();

		/// <summary>
		///  <para>
		///   Blocks the current thread until another thread unblocks it by calling this object's Disable() method
		///   or until the given timeout has been reached.
		///  </para>
		///  <para>This method will immediately return if this blocker object is currently disabled.</para>
		/// </summary>
		/// <param name="duration">
		///  Maximum amount of time this object should wait for another thread to unblock it via the Disable() method.
		/// </param>
		/// <returns>
		///  Returns a result indicating if the calling thread was unblocked by another thread or if the timeout was reached.
		/// </returns>
		Result BlockUntil(const std::chrono::milliseconds &duration);
		
		/// <summary>
		///  <para>
		///   Blocks the current thread until another thread unblocks it by calling this object's Disable() method
		///   or until the given timeout has been reached.
		///  </para>
		///  <para>This method will immediately return if this blocker object is currently disabled.</para>
		/// </summary>
		/// <param name="duration">
		///  Maximum amount of time this object should wait for another thread to unblock it via the Disable() method.
		/// </param>
		/// <returns>
		///  Returns a result indicating if the calling thread was unblocked by another thread or if the timeout was reached.
		/// </returns>
		Result BlockUntil(const std::chrono::seconds &duration);

		/// <summary>
		///  <para>
		///   Blocks the current thread until another thread unblocks it by calling this object's Disable() method
		///   or until the given timeout has been reached.
		///  </para>
		///  <para>This method will immediately return if this blocker object is currently disabled.</para>
		/// </summary>
		/// <param name="milliseconds">
		///  The number of milliseconds this object should wait for another thread to unblock it via the Disable() method.
		/// </param>
		/// <returns>
		///  Returns a result indicating if the calling thread was unblocked by another thread or if the timeout was reached.
		/// </returns>
		Result BlockUntilDurationInMilliseconds(long long milliseconds);

		/// <summary>Enables this object to block the current thread when calling this object's Block() methods.</summary>
		void Enable();

		/// <summary>
		///  <para>
		///   Disables this blocker so that its Block() methods will immediately return, preventing this object
		///   from blocking the calling thread.
		///  </para>
		///  <para>
		///   If a thread is currently being blocked by this object's Block() method, then it will immediately unblock
		///   and return out when another thread calls this method.
		///  </para>
		///  <para>Intended to be called by another thread to unblock the thread that called this object's Block() method.</para>
		/// </summary>
		void Disable();

		/// <summary>Determines if this object is set up to block the thread that calls its Block() methods.</summary>
		/// <returns>Returns true if blocking is enabled. Returns false if not.</returns>
		bool IsEnabled() const;

		/// <summary>Determines if this object is set up to block the thread that calls its Block() methods.</summary>
		/// <returns>Returns true if blocking is disabled. Returns false if enabled.</returns>
		bool IsDisabled() const;

		/// <summary>Determines if this object is currently blocking a thread via one of its Block() methods.</summary>
		/// <returns>Returns true if at least 1 thread is being blocked by this object. Returns false if not.</returns>
		bool IsBlocking() const;
		
		/// <summary>Determines if this object is not currently blocking a thread via one of its Block() methods.</summary>
		/// <returns>
		///  <para>Returns true if this object is not blocking any threads.</para>
		///  <para>Returns false if at least 1 thread is being blocked.</para>
		/// </returns>
		bool IsNotBlocking() const;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>
		///  <para>Compares the given tick values returned by the Win32 ::GetTickCount64() function.</para>
		///  <para>
		///   Correctly handles tick overflow where large negative numbers are considered greater than large positive numbers.
		///  </para>
		/// </summary>
		/// <param name="x">The tick value to be compared with parameter "y".</param>
		/// <param name="y">The tick value to be compared with parameter "x".</param>
		/// <returns>
		///  <para>Returns a positive value if "x" is greater than "y".</para>
		///  <para>Returns zero if "x" is equal to "y".</para>
		///  <para>Returns a negative value if "x" is less than "y".</para>
		/// </returns>
		static int CompareTicks(long long x, long long y);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Set true if this object is set up to block a thread that calls this object's Block() methods.</summary>
		std::atomic_bool fAtomicIsEnabled;

		/// <summary>
		///  <para>Keeps a count of the number of threads currently being blocked by this object.</para>
		///  <para>Set to zero if no threads are currently being blocked.</para>
		/// </summary>
		std::atomic_int fAtomicBlockOperationCount;

		#pragma endregion
};

} }	// namespace CoronaLabs::WinRT
