using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop
{
	/// <summary>
	///  Creates .NET implemented timers that are compatible with Corona and WinRT component libraries.
	/// </summary>
	/// <remarks>An instance of this class is expected to be given to a CoronaRuntimeSettings object on startup.</remarks>
	public class DotNetTimerServices : WinRT.Interop.ITimerServices
	{
		/// <summary>Creates a new provider used to create .NET implemented timer objects.</summary>
		public DotNetTimerServices()
		{
		}

		/// <summary>Creates a new timer object.</summary>
		/// <returns>
		///  <para>Returns a new .NET implemented timer that runs on the main thread via the UI's dispatcher queue.</para>
		///  <para>
		///   The returned timer is not running by default. The caller is expected to give it an
		///   interval, set up an Elapsed event handler, and start it manually.
		///  </para>
		/// </returns>
		public WinRT.Interop.ITimer CreateTimer()
		{
			return new DotNetTimer();
		}
	}
}
