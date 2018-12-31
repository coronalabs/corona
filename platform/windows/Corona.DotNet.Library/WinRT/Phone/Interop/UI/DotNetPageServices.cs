using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.UI
{
	/// <summary>Provides useful methods for accessing .NET Xaml page information.</summary>
	/// <remarks>You cannot create instances of this class. Only its static methods/properties are made available.</remarks>
	public class DotNetPageServices
	{
		#region Constructors
		/// <summary>Constructor made private to prevent instances from being made.</summary>
		private DotNetPageServices()
		{
		}

		#endregion


		#region Public Static Methods
		/// <summary>Fetches the page that is hosting the given control.</summary>
		/// <param name="element">The UI element to fetch the parent page from. Can be null.</param>
		/// <returns>
		///  <para>Returns a reference to the page that is hosting the given control.</para>
		///  <para>Returns null if the given control is not assigned to a page or if given a null argument.</para>
		/// </returns>
		public static Microsoft.Phone.Controls.PhoneApplicationPage FetchParentPageFrom(System.Windows.FrameworkElement element)
		{
			// Validate argument.
			if (element == null)
			{
				return null;
			}

			// Recursively fetch the given UI element's parent page.
			var parentPage = element as Microsoft.Phone.Controls.PhoneApplicationPage;
			if (parentPage == null)
			{
				parentPage = element.Parent as Microsoft.Phone.Controls.PhoneApplicationPage;
				if (parentPage == null)
				{
					parentPage = FetchParentPageFrom(element.Parent as System.Windows.FrameworkElement);
				}
			}
			return parentPage;
		}

		#endregion
	}
}
