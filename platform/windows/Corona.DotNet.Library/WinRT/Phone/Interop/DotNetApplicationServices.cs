using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop
{
	/// <summary>
	///  <para>Provides access to the currently running application's information and services.</para>
	///  <para>Services such as the ability to exit the app or enable/disable automatic screen lock when idle.</para>
	/// </summary>
	public class DotNetApplicationServices : CoronaLabs.Corona.WinRT.Interop.IApplicationServices
	{
		#region Constructors
		/// <summary>Creates a new object used to access the currently running application's information and services.</summary>
		public DotNetApplicationServices()
		{
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>
		///  <para>Enables/disables automatic screen lock when the device has been idle for a period of time.</para>
		///  <para>This property is typically invoked by Corona's system.setIdleTimer() function in Lua.</para>
		/// </summary>
		/// <value>
		///  <para>
		///   Set true to enable automatic screen lock. This means the screen will automatically turn off if the device
		///   has not been touched for a period of time. Note that this property will be ignored if this feature has
		///   not been enabled in the operating system's "Lock Screen" settings screen too.
		///  </para>
		///  <para>
		///   Set false to prevent the screen from turning off when the device hasn't been touched for a period of time.
		///   This is useful for applications that display media content for several minutes or for applications that
		///   use the accelerometer or gyroscope for control input.
		///  </para>
		/// </value>
		public bool LockScreenWhenIdleEnabled
		{
			get
			{
				var idleMode = Microsoft.Phone.Shell.PhoneApplicationService.Current.UserIdleDetectionMode;
				return (idleMode == Microsoft.Phone.Shell.IdleDetectionMode.Enabled);
			}
			set
			{
				var appServices = Microsoft.Phone.Shell.PhoneApplicationService.Current;
				if (value)
				{
					appServices.UserIdleDetectionMode = Microsoft.Phone.Shell.IdleDetectionMode.Enabled;
				}
				else
				{
					appServices.UserIdleDetectionMode = Microsoft.Phone.Shell.IdleDetectionMode.Disabled;
				}
			}
		}

		#endregion
	}
}
