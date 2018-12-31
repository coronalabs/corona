using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.UI
{
	public class DotNetDispatcher : Corona.WinRT.Interop.UI.IDispatcher
	{
		#region Private Member Variables
		/// <summary>
		///  <para>Set true if it is okay to use the system's dispatche objects.</para>
		///  <para>Set false if not, which typically happens while the application is exiting.</para>
		/// </summary>
		private static bool sIsDispatcherValid = true;

		#endregion


		#region Constructors
		/// <summary>Static constructor.</summary>
		static DotNetDispatcher()
		{
			System.Windows.Application.Current.Exit += OnApplicationExiting;
		}

		/// <summary>Creates a new UI thread task dispatcher.</summary>
		public DotNetDispatcher()
		{
		}

		#endregion


		#region Public Methods
		public void Invoke(Windows.UI.Core.DispatchedHandler handler)
		{
			// Validate argument.
			if (handler == null)
			{
				throw new NullReferenceException("handler");
			}

			// Invoke the given handler on the main UI thread now. (This is a block call.)
			System.Windows.Threading.Dispatcher dispatcher = System.Windows.Deployment.Current.Dispatcher;
			if (dispatcher.CheckAccess())
			{
				handler.Invoke();
			}
			else if (sIsDispatcherValid)
			{
				var waitEvent = new System.Threading.AutoResetEvent(false);
				var operation = dispatcher.BeginInvoke(() =>
				{
					handler.Invoke();
					waitEvent.Set();
				});
				waitEvent.WaitOne();
			}
		}

		public void InvokeAsync(Windows.UI.Core.DispatchedHandler handler)
		{
			// Validate argument.
			if (handler == null)
			{
				throw new NullReferenceException("handler");
			}

			// Do not continue if dispatchers can no longer be used, which happen while the application is exiting.
			if (sIsDispatcherValid == false)
			{
				return;
			}

			// Queue the given handler to be invoked on the main UI thread later.
			System.Windows.Deployment.Current.Dispatcher.BeginInvoke(handler, null);
		}

		#endregion


		#region Event Handlers
		/// <summary>Called when the application is about to exit out.</summary>
		/// <param name="sender">The application object that is raising this event.</param>
		/// <param name="e">Empty event arguments.</param>
		private static void OnApplicationExiting(object sender, EventArgs e)
		{
			sIsDispatcherValid = false;
		}

		#endregion


		#region Private SynchronousOperation Class
		private void OnDispatchedOperationInvoked(object state)
		{
			var handler = state as Windows.UI.Core.DispatchedHandler;
			if (handler != null)
			{
				handler.Invoke();
			}
		}

		#endregion
	}
}
