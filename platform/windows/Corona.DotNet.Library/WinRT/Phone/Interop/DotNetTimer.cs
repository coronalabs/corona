using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop
{
	/// <summary>Timer which provides "Elapsed" events on the main thread via the UI's dispatcher queue.</summary>
	public class DotNetTimer : WinRT.Interop.ITimer
	{
		#region Private Member Variables
		/// <summary>Set true if running and will elapse at regular intervals. Set false if the timer is disabled.</summary>
		private bool fIsRunning;

		/// <summary>The amount of time this timer will wait until it elapses.</summary>
		private TimeSpan fInterval;

		/// <summary>
		///  <para>The next scheduled time in system ticks (measured in milliseconds) that this timer will elapse.</para>
		///  <para>
		///   Warning! This value can overflow. Use the CompareTicks() method to compare tick values which handles
		///   overflowed values correctly.
		///  </para>
		/// </summary>
		private long fNextIntervalTimeInTicks;

		/// <summary>
		///  <para>Delegate referencing this class' private OnRequestingUpdate() method.</para>
		///  <para>Intended to be passed into a Dispatcher class' BeginInvoke() method.</para>
		/// </summary>
		private RequestingUpdateHandler fRequestingUpdateHandler;

		#endregion


		#region Delegates
		/// <summary>
		///  Private delegate used to have this class' OnRequestingUpdate() method called by the XAML dispatcher.
		/// </summary>
		private delegate void RequestingUpdateHandler();

		#endregion


		#region Events
		/// <summary>Raised every time the timer's interval has elapsed.</summary>
		public event Windows.Foundation.TypedEventHandler<WinRT.Interop.ITimer, CoronaLabs.WinRT.EmptyEventArgs> Elapsed;

		#endregion


		#region Constructors
		/// <summary>Creates a new timer.</summary>
		public DotNetTimer()
		{
			fIsRunning = false;
			fInterval = TimeSpan.FromSeconds(1.0);
			fNextIntervalTimeInTicks = 0;
			fRequestingUpdateHandler = new RequestingUpdateHandler(OnRequestingUpdate);
		}

		#endregion


		#region Event Handlers
		/// <summary>
		///  <para>Called by XAML's dispatcher system at regular intervals.</para>
		///  <para>Determines if this timer's interval has elapsed, raises an event, and then re-schedules.</para>
		/// </summary>
		/// <remarks>
		///  This method implements the "RequestingUpdateHandler" delegate and is intended to be pushed into
		///  the XAML dispatcher queue via the Dispatcher.BeginInvoke() method. This provides much more accurate
		///  timer intervals compared to XAML's DispatcherTimer class, which is especially needed since Corona's
		///  runtime timer needs to be as accurate as possible to implement a good rendering framerate.
		/// </remarks>
		private void OnRequestingUpdate()
		{
			// Do not continue if this timer has been disabled.
			if (fIsRunning == false)
			{
				return;
			}

			// Determine if we have reached the scheduled time.
			long currentTicks = GetCurrentTicks();
			if (CompareTicks(currentTicks, fNextIntervalTimeInTicks) >= 0)
			{
				// Raise the timer's event.
				if (this.Elapsed != null)
				{
					this.Elapsed(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
				}

				// Schedule the next interval time.
				currentTicks = GetCurrentTicks();
				long intervalInMilliseconds = (long)fInterval.TotalMilliseconds;
				for (; CompareTicks(currentTicks, fNextIntervalTimeInTicks) >= 0; fNextIntervalTimeInTicks += intervalInMilliseconds);
			}

			// Have this method called the next time the dispatcher queue gets pumped.
			System.Windows.Deployment.Current.Dispatcher.BeginInvoke(fRequestingUpdateHandler);
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets or sets the interval to be used by this timer.</summary>
		/// <remarks>Setting the interval while the timer is running will cause it to be restarted.</remarks>
		/// <value>
		///  <para>The amount of time this timer will take until it elapses.</para>
		///  <para>Cannot be set to less than 1 millisecond.</para>
		/// </value>
		public TimeSpan Interval
		{
			get
			{
				return fInterval;
			}
			set
			{
				// Do not continue if the interval hasn't changed.
				if (fInterval == value)
				{
					return;
				}

				// Stop the timer if it is currently running.
				bool isRunning = this.IsRunning;
				if (isRunning)
				{
					this.Stop();
				}

				// Update the timer's interval.
				fInterval = value;

				// Restart the timer if it was previously running.
				if (isRunning)
				{
					this.Start();
				}
			}
		}

		/// <summary>Determines if this timer has been started and currently running.</summary>
		/// <value>
		///  <para>Returns true if this timer is currently running and will provided "Elapsed" events.</para>
		///  <para>Returns false if this timer is not running.</para>
		/// </value>
		public bool IsRunning
		{
			get { return fIsRunning; }
		}

		/// <summary>Determines if this timer has been stopped or never started.</summary>
		/// <value>
		///  <para>Returns true if this timer is not running, meaning "Elapsed" events will not be raised.</para>
		///  <para>Returns false if this timer is currently running.</para>
		/// </value>
		public bool IsNotRunning
		{
			get { return !fIsRunning; }
		}

		/// <summary>Starts this timer with the duration provided to the "Interval" property.</summary>
		/// <remarks>
		///  <para>Once started, this timer will provide "Elapsed" events every time an interval has been completed.</para>
		///  <para>This method will do nothing if this timer is already running.</para>
		/// </remarks>
		public void Start()
		{
			// Do not continue if this timer is already running.
			if (fIsRunning)
			{
				return;
			}

			// Enable this timer and schedule the next time it will elapse.
			fIsRunning = true;
			fNextIntervalTimeInTicks = GetCurrentTicks() + (long)fInterval.TotalMilliseconds;
			System.Windows.Deployment.Current.Dispatcher.BeginInvoke(fRequestingUpdateHandler);
		}

		/// <summary>Stops this timer if currently running.</summary>
		/// <remarks>Stopping the timer will cause it to stop providing "Elapsed" events.</remarks>
		public void Stop()
		{
			fIsRunning = false;
		}

		#endregion


		#region Private Methods
		/// <summary>
		///  Gets the current system ticks, which is a system timer which always counts up and is unaffected by clock changes.
		/// </summary>
		/// <remarks>
		///  Warning! System ticks can overflow and become negative. You should use the CompareTicks() method when
		///  comparing tick values to correctly handle the overflow condition.
		/// </remarks>
		/// <returns>Returns the current system ticks, which is measured in milliseconds.</returns>
		private long GetCurrentTicks()
		{
			long value;

			if (System.Diagnostics.Stopwatch.IsHighResolution)
			{
				// Convert CPU ticks to milliseconds.
				value = (System.Diagnostics.Stopwatch.GetTimestamp() * 1000L) / System.Diagnostics.Stopwatch.Frequency;
			}
			else
			{
				// Use the less accurate system ticks value. (This is already in milliseconds.)
				value = System.Environment.TickCount;
			}
			return value;
		}

		/// <summary>Compares the given system tick values retrieved via the GetCurrentTicks() method.</summary>
		/// <param name="x">The ticks value to be compared with argument "y".</param>
		/// <param name="y">The ticks value to be compared with argument "x"</param>
		/// <returns>
		///  <para>Returns a positive value if x is greater than y.</para>
		///  <para>Returns a negative value if y is less than x.</para>
		///  <para>Returns zero if x and y are equal.</para>
		/// </returns>
		private long CompareTicks(long x, long y)
		{
			// We can't flip the sign of an integer at it's minimum value.
			// So, add one millisecond so that we can apply the subtraction down below
			if (y == long.MinValue)
			{
				y++;
			}

			// Compare the given tick values via subtraction. Overflow for this subtraction operation is okay.
			long deltaTime = x - y;
			if (deltaTime < 0)
			{
				return -1;
			}
			else if (0 == deltaTime)
			{
				return 0;
			}
			return 1;
		}

		#endregion
	}
}
