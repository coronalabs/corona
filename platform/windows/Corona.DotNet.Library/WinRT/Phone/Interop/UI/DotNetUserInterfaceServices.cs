using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.UI
{
	/// <summary>
	///  Creates .NET UI controls and wraps them in Corona compatible adapters, making them accessible
	///  to the C++/CX side of the application.
	/// </summary>
	public class DotNetUserInterfaceServices : Corona.WinRT.Interop.UI.IUserInterfaceServices
	{
		/// <summary>
		///  <para>Gets a fractional scale factor used to convert the UI's coordinates to pixels.</para>
		///  <para>This scale is intented to be multiplied against a UI coordinate value to convert it to pixels.</para>
		///  <para>
		///   For Windows Phone apps, this is retrieved from "System.Windows.Application.Current.Host.Content.ScaleFactor",
		///   which doesn't always match property "Windows.Graphics.Display.DisplayProperties.ResolutionScale".
		///  </para>
		/// </summary>
		/// <value>
		///  <para>A fractional scale used to convert the UI's coordinates to pixels.</para>
		///  <para>Returns 1.0 if the UI's coordinates are not scaled.</para>
		/// </value>
		public double ScaleFactor
		{
			get
			{
				int fixedPointScale = System.Windows.Application.Current.Host.Content.ScaleFactor;
				if (fixedPointScale <= 0)
				{
					fixedPointScale = 100;
				}
				return (double)fixedPointScale / 100.0;
			}
		}

		/// <summary>
		///  <para>Wraps the given UI control with an adapter.</para>
		///  <para>This makes it accessible to Corona on the C++/CX side in a cross-platform and cross-language manner.</para>
		/// </summary>
		/// <param name="control">
		///  <para>Reference to an existing .NET control to be wrapped by a Corona adapter.</para>
		///  <para>Expected to be of type "System.Windows.FrameworkElement".</para>
		/// </param>
		/// <returns>
		///  <para>Returns a new adapter wrapping the given control.</para>
		///  <para>Returns null if the given object is not of type "System.Windows.FrameworkElement".</para>
		/// </returns>
		public Corona.WinRT.Interop.UI.IControlAdapter CreateAdapterFor(object control)
		{
			// Validate.
			if (control == null)
			{
				return null;
			}

			// Attempt to create and return an appropriate adapter for the given control.
			if (DotNetCoronaControlAdapter.CanWrap(control))
			{
				return new DotNetCoronaControlAdapter(control as System.Windows.FrameworkElement);
			}
			else if (control is System.Windows.FrameworkElement)
			{
				return new DotNetControlAdapter(control as System.Windows.FrameworkElement);
			}

			// The given object is not a control.
			return null;
		}

		/// <summary>
		///  <para>Creates an interface to a new modal message box dialog.</para>
		///  <para>Used by Corona's Lua native.showAlert() function.</para>
		/// </summary>
		/// <returns>Returns a new message box object to be configured and displayed by the caller.</returns>
		public Corona.WinRT.Interop.UI.IMessageBox CreateMessageBox()
		{
			return new CoronaMessageBox();
		}
	}
}
