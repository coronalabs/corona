using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.UI
{
	/// <summary>Provides access to a Windows Phone 8.0 .NET Xaml control's features and events.</summary>
	public class DotNetControlAdapter : Corona.WinRT.Interop.UI.IControlAdapter
	{
		#region Private Member Variables
		/// <summary>Reference to the Xaml control that this adapter wraps.</summary>
		private System.Windows.FrameworkElement fControl;

		/// <summary>Provides access to the page that is hosting the referenced control.</summary>
		private Corona.WinRT.Interop.UI.PageProxy fPageProxy;

		/// <summary>Dispatcher used to queue operations to be executed on the main UI thread.</summary>
		private DotNetDispatcher fDispatcher;

		#endregion


		#region Events
		/// <summary>
		///  <para>Raised when the control has been added to the page and is about to be displayed.</para>
		///  <para>Handler is expected to finish initialization with this event before the control is rendered onscreen.</para>
		/// </summary>
		public event Windows.Foundation.TypedEventHandler<Corona.WinRT.Interop.UI.IControlAdapter, CoronaLabs.WinRT.EmptyEventArgs> Loaded;

		/// <summary>
		///  <para>Raised when the control has been removed from the page and is no longer visible onscreen.</para>
		///  <para>Handler is expected to release any allocated resources that was created via the Loaded event.</para>
		/// </summary>
		public event Windows.Foundation.TypedEventHandler<Corona.WinRT.Interop.UI.IControlAdapter, CoronaLabs.WinRT.EmptyEventArgs> Unloaded;

		/// <summary>Raised when the width and/or height of the control has changed.</summary>
		public event Windows.Foundation.TypedEventHandler<Corona.WinRT.Interop.UI.IControlAdapter, CoronaLabs.WinRT.EmptyEventArgs> Resized;

		#endregion


		#region Constructors
		/// <summary>Creates a new adapter which wraps the given .NET Xaml control.</summary>
		/// <param name="control">
		///  <para>The control to be wrapped by this adapter.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		public DotNetControlAdapter(System.Windows.FrameworkElement control)
		{
			// Validate argument.
			if (control == null)
			{
				throw new NullReferenceException();
			}

			// Initialize member variables.
			fControl = control;
			fDispatcher = new DotNetDispatcher();
			fPageProxy = new Corona.WinRT.Interop.UI.PageProxy();
			fPageProxy.Page = DotNetPageAdapter.FromParentOf(fControl);

			// Add event handlers.
			fControl.Loaded += OnLoaded;
			fControl.Unloaded += OnUnloaded;
			fControl.SizeChanged += OnSizeChanged;
		}

		#endregion


		#region Event Handlers
		/// <summary>Raised when the adapter's control has been added to a page and is about to be displayed.</summary>
		/// <param name="sender">The control that raised this event.</param>
		/// <param name="e">Event arguments providing the original source of this event.</param>
		private void OnLoaded(object sender, System.Windows.RoutedEventArgs e)
		{
			// Fetch this control's parent page.
			var parentPage = DotNetPageServices.FetchParentPageFrom(fControl);
			if (parentPage == null)
			{
				return;
			}

			// Do not continue if this control has already been loaded on the page.
			// Note: This can occur if the parent page was unloaded and re-loaded later, which happens when
			//       navigating to another page within the app and then navigating back to the parent page.
			var pageAdapter = fPageProxy.Page as Interop.UI.DotNetPageAdapter;
			if ((pageAdapter != null) && (pageAdapter.Page == parentPage))
			{
				return;
			}

			// Store a reference to the new parent page.
			if (pageAdapter != null)
			{
				pageAdapter.Page = null;
			}
			pageAdapter = new DotNetPageAdapter();
			pageAdapter.Page = parentPage;
			fPageProxy.Page = pageAdapter;

			// Relay the event.
			if (this.Loaded != null)
			{
				this.Loaded.Invoke(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
			}
		}

		/// <summary>Raised when the adapter's control has been removed from the page and is no longer visible onscreen.</summary>
		/// <param name="sender">The control that raised this event.</param>
		/// <param name="e">Event arguments providing the original source of this event.</param>
		private void OnUnloaded(object sender, System.Windows.RoutedEventArgs e)
		{
			// Do not continue if this control is still attached to its parent page.
			// This can happen when navigating to another page within the app, where the parent page gets unloaded instead.
			var parentPage = DotNetPageServices.FetchParentPageFrom(fControl);
			if (parentPage != null)
			{
				return;
			}

			// Remove the page reference that the control is no longer attached to.
			var pageAdapter = fPageProxy.Page as Interop.UI.DotNetPageAdapter;
			if (pageAdapter != null)
			{
				pageAdapter.Page = null;
			}
			fPageProxy.Page = null;

			// Relay the event.
			if (this.Unloaded != null)
			{
				this.Unloaded.Invoke(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
			}
		}

		/// <summary>Raised when the adapter's control width and/or height has changed.</summary>
		/// <param name="sender">The control that raised this event.</param>
		/// <param name="e">Event arguemnts providing the new width and height of the control.</param>
		private void OnSizeChanged(object sender, System.Windows.SizeChangedEventArgs e)
		{
			// Relay the event.
			if (this.Resized != null)
			{
				this.Resized.Invoke(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
			}
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets or sets the pixel width of the control.</summary>
		/// <value>
		///  <para>The width of the control in pixels.</para>
		///  <para>Must be set to a value greater than or equal to zero.</para>
		///  <para>Changing this property will cause a "Resized" event to be raised.</para>
		/// </value>
		public int Width
		{
			get
			{
				// Return zero if this adapter no longer references a control.
				if (fControl == null)
				{
					return 0;
				}

				// Convert the referenced control's width to pixels and return it.
				double scaleFactor = (double)System.Windows.Application.Current.Host.Content.ScaleFactor / 100.0;
				double pixelWidth = fControl.ActualWidth * scaleFactor;
				return (int)Math.Round(pixelWidth, MidpointRounding.AwayFromZero);
			}
			set
			{
				// Do nothing if this adapter no longer references a control.
				if (fControl == null)
				{
					return;
				}

				// Change the control's width.
				fControl.Width = value;
			}
		}

		/// <summary>Gets or sets the pixel height of the control.</summary>
		/// <value>
		///  <para>The height of the control in pixels.</para>
		///  <para>Must be set to a value greater than or equal to zero.</para>
		///  <para>Changing this property will cause a "Resized" event to be raised.</para>
		/// </value>
		public int Height
		{
			get
			{
				// Return zero if this adapter no longer references a control.
				if (fControl == null)
				{
					return 0;
				}

				// Convert the referenced control's height to pixels and return it.
				double scaleFactor = (double)System.Windows.Application.Current.Host.Content.ScaleFactor / 100.0;
				double pixelHeight = fControl.ActualHeight * scaleFactor;
				return (int)Math.Round(pixelHeight, MidpointRounding.AwayFromZero);
			}
			set
			{
				// Do nothing if this adapter no longer references a control.
				if (fControl == null)
				{
					return;
				}

				// Change the control's height.
				fControl.Height = value;
			}
		}

		/// <summary>Gets a dispatcher used to queue operations to be executed on the main UI thread.</summary>
		/// <value>The main UI thread dispatcher this control is associated with.</value>
		public Corona.WinRT.Interop.UI.IDispatcher Dispatcher
		{
			get { return fDispatcher; }
		}

		/// <summary>Gets a proxy to the page that is hosting this control.</summary>
		/// <value>
		///  <para>Gets the page that is hosting this control.</para>
		///  <para>
		///   Note that the returned proxy will not be null when the control is removed from the page.
		///   In this case, the proxy will safely no-op when calling its methods and provide the last
		///   known property values from this control's previously assigned page.
		///  </para>
		/// </value>
		public Corona.WinRT.Interop.UI.PageProxy ParentPageProxy
		{
			get { return fPageProxy; }
		}

		/// <summary>Gets the .NET Xaml control that this adapter wraps.</summary>
		/// <remarks>
		///  Provided as a generic object reference to be cross-platform between Windows Phone and
		///  Windows Universal applications which use different UI frameworks.
		/// </remarks>
		/// <value>
		///  <para>Reference to the "System.Windows.UIElement" derived control that this adapter wraps.</para>
		///  <para>
		///   Will be null if this adapter no longer references a control, which can happen after calling
		///   the <see cref="M:ReleaseReferencedControl"/> method.
		///  </para>
		/// </value>
		public object ReferencedControl
		{
			get { return fControl; }
		}

		/// <summary>
		///  <para>Releases the control that this adapter wraps.</para>
		///  <para>This adapter's properties and methods will no longer operate on the control once released.</para>
		/// </summary>
		public virtual void ReleaseReferencedControl()
		{
			// Remove event handlers.
			fControl.Loaded -= OnLoaded;
			fControl.Unloaded -= OnUnloaded;
			fControl.SizeChanged -= OnSizeChanged;

			// Release the control's parent page reference.
			var pageAdapter = fPageProxy.Page as Interop.UI.DotNetPageAdapter;
			if (pageAdapter != null)
			{
				pageAdapter.Page = null;
			}
			fPageProxy.Page = null;

			// Release the control that this adapter wraps.
			fControl = null;
		}

		#endregion
	}
}
