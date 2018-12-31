using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.UI
{
	/// <summary>Provides access to a Windows Phone 8.0 .NET page's features and events.</summary>
	public class DotNetPageAdapter : Corona.WinRT.Interop.UI.IPage
	{
		#region Private Member Variables
		/// <summary>References to the application page that this adapter wraps.</summary>
		private Microsoft.Phone.Controls.PhoneApplicationPage fPage;

		#endregion


		#region Events
		/// <summary>Raised when the application page's orientation has changed.</summary>
		public event Windows.Foundation.TypedEventHandler<Corona.WinRT.Interop.UI.IPage, Corona.WinRT.Interop.UI.PageOrientationEventArgs> OrientationChanged;

		/// <summary>Raised when the end-user attempts to back out of the page.</summary>
		/// <remarks>
		///  If you want to prevent the end-user from navigating back, then you should set the event argument's
		///  Cancel property to true.
		/// </remarks>
		public event Windows.Foundation.TypedEventHandler<Corona.WinRT.Interop.UI.IPage, CoronaLabs.WinRT.CancelEventArgs> NavigatingBack;

		#endregion


		#region Constructors
		/// <summary>
		///  <para>Create a new adapter which does not initially reference a page.</para>
		///  <para>The caller is expected to assign this adapter a page reference via its "Page" property after creation.</para>
		/// </summary>
		public DotNetPageAdapter()
		{
			fPage = null;
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets or sets a page that this adapter will wrap and provide information and events for.</summary>
		/// <remarks>
		///  The owner of this adapter should set this property to null when it is done using the adapter.
		///  This will cause the adapter to remove its event handlers from the page, which in turn would allow
		///  this adapter to be collected by the garbage collector.
		/// </remarks>
		/// <value>
		///  <para>Reference to the page that this adapter wraps and provides information for.</para>
		///  <para>Set to null if this adapter is not assigned a page.</para>
		/// </value>
		public Microsoft.Phone.Controls.PhoneApplicationPage Page
		{
			get { return fPage; }
			set
			{
				// Do not continue if we already have a reference to the given page.
				if (value == fPage)
				{
					return;
				}

				// Unsubscribe from the previous page's events.
				if (fPage != null)
				{
					fPage.OrientationChanged -= OnOrientationChanged;
					fPage.BackKeyPress -= OnBackKeyPressed;
				}

				// Store the given page reference.
				fPage = value;

				// Subscribe to the given page's events.
				if (fPage != null)
				{
					fPage.OrientationChanged += OnOrientationChanged;
					fPage.BackKeyPress += OnBackKeyPressed;
				}
			}
		}

		/// <summary>Gets the application page's current orientation.</summary>
		/// <value>
		///  <para>The application page's current orientation such as portrait, landscape, etc.</para>
		///  <para>Returns an unknown orientation if this adapter has not been assigned a page reference.</para>
		/// </value>
		public Corona.WinRT.Interop.UI.PageOrientation Orientation
		{
			get
			{
				// If this adapter is not assigned a page reference, then return an "unknown" orientation.
				if (fPage == null)
				{
					return Corona.WinRT.Interop.UI.PageOrientation.Unknown;
				}

				// Return the page's orientation.
				return GetCoronaOrientationFrom(fPage.Orientation);
			}
		}

		/// <summary>Commands the page to navigate back to the previous page.</summary>
		/// <remarks>
		///  This method is expected to be called when Corona's native.requestExit() function gets called in Lua.
		/// </remarks>
		public void NavigateBack()
		{
			// Validate.
			if (fPage == null)
			{
				return;
			}

			// Navigate back to the previous page.
			if (fPage.NavigationService.CanGoBack)
			{
				// Go back to the previous page in this application.
				// Note: You cannot use this method exit out of the application. It's only for internal navigation.
				fPage.NavigationService.GoBack();
			}
			else
			{
				// Exit out of the application since it has no more pages to go back to.
				// Note: This is a hack to simulate the back-to-exit behavior since Terminate() exits the app immediately.
				//       Nulling out the page content will unload its controls, which happens when pressing the back button,
				//       and causes Corona to raise an "applicationSuspend" and "applicationExit" event in Lua.
				fPage.Content = null;
				fPage.Dispatcher.BeginInvoke(() =>
				{
					System.Windows.Application.Current.Terminate();
				});
			}
		}

		/// <summary>
		///  <para>Navigates to the application page that the given URI's scheme applies to.</para>
		///  <para>
		///   For example, an "http://" URI will launch the browser app and a "mailto://" scheme will launch the mail app.
		///  </para>
		/// </summary>
		/// <remarks>This method is called by Corona's system.openURL() Lua function.</remarks>
		/// <param name="uri">The URI/URL of the application page to navigate to.</param>
		/// <returns>
		///  <para>Returns true if able to display an application page for the given URI.</para>
		///  <para>Returns false if the given URI was not recognized and has failed to navigate to another page.</para>
		/// </returns>
		public bool NavigateTo(System.Uri uri)
		{
			// Do not continue if we cannot navigate to the given URL.
			if (this.CanNavigateTo(uri) == false)
			{
				return false;
			}

			// Attempt to navigate to the given URL.
			var asyncStatus = Windows.System.Launcher.LaunchUriAsync(uri);
			return true;
		}

		/// <summary>Determines if the given URI will work when passed to the NavigateTo() method.</summary>
		/// <remarks>This method is called by Corona's system.canOpenURL() Lua function.</remarks>
		/// <param name="uri">The URI/URL of the application page to navigate to.</param>
		/// <returns>
		///  <para>Returns true if able to display an application page for the given URI.</para>
		///  <para>Returns false if the given URI cannot open a page or is invalid.</para>
		/// </returns>
		public bool CanNavigateTo(System.Uri uri)
		{
			// Validate argument.
			if (uri == null)
			{
				return false;
			}

			// Do not continue if given a URL without a scheme.
			if ((uri.Scheme == null) || (uri.Scheme.Length <= 0))
			{
				return false;
			}

			// Do not continue if given a path to a file.
			// Note: It might not be possible to open an app's sandboxed file in another app.
			if (uri.IsFile)
			{
				return false;
			}

			// Assume that the system can open the given URL.
			// Note: WP8 Silverlight does not have an API to verify if URL schemes are installed/registered.
			//       If a scheme is not registered, then the system will show an option to search the app store
			//       which will list all apps matching the given scheme.
			return true;
		}

		#endregion


		#region Public Static Methods
		/// <summary>
		///  Fetches the page that is hosting the given UI element and returns it wrapped in a page adapter.
		/// </summary>
		/// <param name="element">The UI element to fetch the page from. Can be null.</param>
		/// <returns>
		///  <para>
		///   Returns a new adapter object which wraps the page that is hosting the given UI element.
		///   Note that you should null out the return adapter's "Page" property when you are done using it
		///   or else the adapter will live for the lifetime of the page.
		///  </para>
		///  <para>Returns null if the given UI element is not currently being hosted on a page.</para>
		/// </returns>
		public static DotNetPageAdapter FromParentOf(System.Windows.FrameworkElement element)
		{
			// Validate argument.
			if (element == null)
			{
				return null;
			}

			// Recursively fetch the given UI element's parent page and wrap it with an instance of this class if found.
			DotNetPageAdapter pageAdapter = null;
			var parentPage = element.Parent as Microsoft.Phone.Controls.PhoneApplicationPage;
			if (parentPage != null)
			{
				pageAdapter = new DotNetPageAdapter();
				pageAdapter.Page = parentPage;
			}
			else
			{
				pageAdapter = FromParentOf(element.Parent as System.Windows.FrameworkElement);
			}
			return pageAdapter;
		}

		#endregion


		#region Event Handlers
		/// <summary>Called when the page orientation has changed, such as portrait to landscape.</summary>
		/// <param name="sender">The page that raised this event.</param>
		/// <param name="e">Provides the new orientation that the page has been set to.</param>
		private void OnOrientationChanged(object sender, Microsoft.Phone.Controls.OrientationChangedEventArgs e)
		{
			if (this.OrientationChanged != null)
			{
				var coronaOrientation = GetCoronaOrientationFrom(e.Orientation);
				this.OrientationChanged(this, new Corona.WinRT.Interop.UI.PageOrientationEventArgs(coronaOrientation));
			}
		}

		/// <summary>Called when the back key has been pressed the system is about to navigate back out of the page.</summary>
		/// <param name="sender">The page that raised this event.</param>
		/// <param name="e">Provides a "Cancel" property, which if set true, will cancel/block navigation.</param>
		private void OnBackKeyPressed(object sender, System.ComponentModel.CancelEventArgs e)
		{
			// Do not continue if the "NavigatingBack" event has no subscribers.
			if (this.NavigatingBack == null)
			{
				return;
			}

			// Do not continue if a Corona custom message box is currently displayed on screen.
			// In this case, the message box should handle the back key event.
			if (CoronaMessageBox.ShownMessagBoxes.Count > 0)
			{
				return;
			}

			// Raise the event.
			var cancelEventArgs = new CoronaLabs.WinRT.CancelEventArgs();
			this.NavigatingBack(this, cancelEventArgs);

			// Cancel the navigation if flagged by an event handler.
			if (cancelEventArgs.Cancel)
			{
				e.Cancel = true;
			}
		}

		#endregion


		#region Private Methods
		/// <summary>Converts the given Microsoft orientation constant to Corona's equivalent orientation constant.</summary>
		/// <param name="orientation">The orientation to be converted.</param>
		/// <returns>Returns the Corona constant equiavlent for the given orientation.</returns>
		private Corona.WinRT.Interop.UI.PageOrientation GetCoronaOrientationFrom(
			Microsoft.Phone.Controls.PageOrientation orientation)
		{
			Corona.WinRT.Interop.UI.PageOrientation coronaOrientation;

			switch (orientation)
			{
				case Microsoft.Phone.Controls.PageOrientation.LandscapeRight:
					// Landscape right on Windows Phone is where the top part of the phone is on the right side.
					// This is the opposite of what Corona and iOS documents it to be. So, flip it here.
					coronaOrientation = Corona.WinRT.Interop.UI.PageOrientation.LandscapeLeft;
					break;

				case Microsoft.Phone.Controls.PageOrientation.LandscapeLeft:
					// Landscape left on Windows Phone is where the top part of the phone is on the left side.
					// This is the opposite of what Corona and iOS documents it to be. So, flip it here.
					coronaOrientation = Corona.WinRT.Interop.UI.PageOrientation.LandscapeRight;
					break;

				case Microsoft.Phone.Controls.PageOrientation.PortraitDown:
					coronaOrientation = Corona.WinRT.Interop.UI.PageOrientation.PortraitUpsideDown;
					break;

				case Microsoft.Phone.Controls.PageOrientation.PortraitUp:
				default:
					coronaOrientation = Corona.WinRT.Interop.UI.PageOrientation.PortraitUpright;
					break;
			}
			return coronaOrientation;
		}

		#endregion
	}
}
