using System;


namespace CoronaLabs.Corona.WinRT.Interop.Logging
{
	/// <summary>
	///  Logs messages to Visual Studio's "Output" panel if the .NET "Managed" debugger is attached to the application.
	/// </summary>
	/// <remarks>
	///  You cannot create instances of this class. Instead, you must access a single instance of
	///  this class made available via the <see cref="Instance"/> property.
	/// </remarks>
	public class DotNetDebuggerLogger : ILogger
	{
		#region Constructors
		/// <summary>Creates a new logger which outputs to the .NET Managed debugger.</summary>
		/// <remarks>
		///  Constructor made private to force callers to use a single instance of this class made available
		///  via the static <see cref="Instance"/> property.
		/// </remarks>
		private DotNetDebuggerLogger()
		{
		}

		#endregion


		#region Pre-allocated Instance
		/// <summary>Provides a pre-allocated instance of this class.</summary>
		public static readonly DotNetDebuggerLogger Instance = new DotNetDebuggerLogger();

		#endregion


		#region Public Methods/Properties
		/// <summary>Determines if this logger is able to output messages to Visual Studio's managed debugger.</summary>
		/// <value>
		///  <para>Returns true if this logger's Write() method is able to output messages.</para>
		///  <para>
		///   Returns false if this logger is unable to log anything.
		///   This occurs when the .NET Managed debugger is not attached to the application.
		///  </para>
		/// </value>
		public bool CanWrite
		{
			get { return System.Diagnostics.Debugger.IsLogging(); }
		}

		/// <summary>
		///  <para>Outputs the given message to the logging system.</para>
		///  <para>Does nothing if this logger's <see cref="CanWrite"/> property is false.</para>
		/// </summary>
		/// <param name="message">The message to be logged. Ignored if given a null or empty string.</param>
		[Windows.Foundation.Metadata.DefaultOverload]
		public void Write(string message)
		{
			if (string.IsNullOrEmpty(message) == false)
			{
				System.Diagnostics.Debugger.Log(0, "Corona", message);
			}
		}

		/// <summary>
		///  <para>Outputs the given message to the logging system.</para>
		///  <para>Does nothing if this logger's <see cref="CanWrite"/> property is false.</para>
		/// </summary>
		/// <param name="message">The message to be logged. Ignored if given a null or empty array.</param>
		public void Write(char[] message)
		{
			if ((message != null) && (message[0] != '\0'))
			{
				System.Diagnostics.Debugger.Log(0, "Corona", new string(message));
			}
		}

		#endregion
	}
}
