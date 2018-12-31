using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Graphics
{
	/// <summary>
	///  Adapter which provides a Direct3D content provider and manipulation handler for a
	///  .NET "DrawingSurfaceBackgroundGrid" Xaml control.
	/// </summary>
	public class DotNetBackgroundSurfaceAdapter : IXamlRenderSurfaceAdapter
	{
		#region Private Member Varaibles
		/// <summary>Reference to the page that is hosting the DrawingSurfaceBackgroundGrid.</summary>
		private UI.DotNetPageAdapter fPageAdapter;

		/// <summary>Dispatcher used to queue and execute operations on the main UI thread.</summary>
		private Corona.WinRT.Phone.Interop.UI.DotNetDispatcher fDispatcher;

		/// <summary>Last received orientation of the page. Used to determine if a resize event should be raised.</summary>
		private Corona.WinRT.Interop.UI.PageOrientation fLastReceivedOrientation;

		/// <summary>The C++/CX Direct3D surface adapter used to render content to the Xaml DrawingSurface control.</summary>
		private Direct3DSurfaceAdapter fDirect3DSurfaceAdapter;

		#endregion


		#region Events
		/// <summary>Raised when the surface's width and/or height has changed.</summary>
		public event Windows.Foundation.TypedEventHandler<IXamlRenderSurfaceAdapter, CoronaLabs.WinRT.EmptyEventArgs> Resized;

		#endregion


		#region Constructors
		/// <summary>Creates a new Corona surface adapter which wraps the given DrawingSurface Xaml control.</summary>
		public DotNetBackgroundSurfaceAdapter()
		{
			// Initialize member variables.
			fPageAdapter = new UI.DotNetPageAdapter();
			fDispatcher = new Corona.WinRT.Phone.Interop.UI.DotNetDispatcher();
			fLastReceivedOrientation = Corona.WinRT.Interop.UI.PageOrientation.Unknown;
			fDirect3DSurfaceAdapter = new Direct3DSurfaceAdapter(this);

			// Add event handlers.
			fPageAdapter.OrientationChanged += OnOrientationChanged;
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets or sets the page that is hosting the DrawingSurfaceBackgroundGrid.</summary>
		/// <remarks>
		///  The owner of this adapter should set this property to null when it is done using this adapter
		///  so that its event handlers can be removed from the page, allowing this object to be garbage collected.
		/// </remarks>
		/// <value>
		///  <para>
		///   Reference to the page that is hosting the .NET DrawingSurfaceBackgroundGrid Xaml control.
		///   This is needed to access the page's current orientation so that this adapter's Direct3D
		///   content provider will know how to rotate its content relative to the page's orientation.
		///  </para>
		///  <para>
		///   Set to null if this adapter has not been assigned a page.
		///   It's Direct3D content provider will not render content in this case.
		///  </para>
		/// </value>
		public Microsoft.Phone.Controls.PhoneApplicationPage Page
		{
			get { return fPageAdapter.Page; }
			set
			{
				// Do not continue if we already have a reference to the given page.
				if (value == fPageAdapter.Page)
				{
					return;
				}

				// Store the given page reference.
				fPageAdapter.Page = value;

				// Raise a "Resized" event if the given page's width and height differ from the previous page.
				// This is determined by orientation such as changing from portrait to landscape or vice-versa.
				OnOrientationChanged(fPageAdapter, new WinRT.Interop.UI.PageOrientationEventArgs(fPageAdapter.Orientation));
			}
		}

		/// <summary>Gets a dispatcher used to queue operations to be executed on the main UI thread.</summary>
		/// <value>The main UI thread's dispatcher.</value>
		public Corona.WinRT.Interop.UI.IDispatcher Dispatcher
		{
			get { return fDispatcher; }
		}

		/// <summary>Gets a C++/CX adapter used to render content to the DrawingSurface via Direct3D.</summary>
		/// <value>Adapter used to render content to a .NET Xaml DrawingSurface control via Direct3D.</value>
		public Direct3DSurfaceAdapter Direct3DSurfaceAdapter
		{
			get { return fDirect3DSurfaceAdapter; }
		}

		/// <summary>
		///  Gets the Direct3D content provider to be passed to a DrawingSurfaceBackgroundGrid object's
		///  SetBackgroundContentProvider() method.
		/// </summary>
		/// <value>The Direct3D content provider designed to render content to a DrawingSurfaceBackgroundGrid control.</value>
		public object BackgroundContentProvider
		{
			get { return fDirect3DSurfaceAdapter.ContentProvider; }
		}

		/// <summary>
		///  Gets the Direct3D manipulation host to be passed to a DrawingSurfaceBackgroundGrid object's
		///  SetBackgroundManipulationHander() method.
		/// </summary>
		/// <value>
		///  The Direct3D manipulation handler which receives touch input from a DrawingSurfaceBackgroundGrid control.
		/// </value>
		public object BackgroundManipulationHandler
		{
			get { return fDirect3DSurfaceAdapter; }
		}

		/// <summary>Determines if this adapter renders fullscreen to the background or to a resizable control.</summary>
		/// <value>Always returns true since this adapter wraps a Xaml DrawingSurfaceBackgroundGrid control.</value>
		public bool IsRenderingToBackground
		{
			get { return true; }
		}

		/// <summary>Gets the width of the surface in pixels, relative to the application's orientations.</summary>
		/// <value>
		///  <para>The surface's width in pixels.</para>
		///  <para>Returns zero if this adapter is not assigned a Page reference.</para>
		/// </value>
		public int WidthInPixels
		{
			get
			{
				// Return zero if this adapter no longer has a page reference.
				if (fPageAdapter.Page == null)
				{
					return 0;
				}

				// Return the background surface's width in pixels, relative to the page's current orientation.
				var content = System.Windows.Application.Current.Host.Content;
				double scaledLength = fPageAdapter.Orientation.IsPortrait ? content.ActualWidth : content.ActualHeight;
				double pixelWidth = (scaledLength * (double)content.ScaleFactor) / 100.0;
				return (int)Math.Round(pixelWidth, MidpointRounding.AwayFromZero);
			}
		}

		/// <summary>Gets the height of the surface in pixels, relative to the application's orientations.</summary>
		/// <value>
		///  <para>The surface's height in pixels.</para>
		///  <para>Returns zero if this adapter is not assigned a Page reference.</para>
		/// </value>
		public int HeightInPixels
		{
			get
			{
				// Return zero if this adapter no longer has a page reference.
				if (fPageAdapter.Page == null)
				{
					return 0;
				}

				// Return the background surface's height in pixels, relative to the page's current orientation.
				var content = System.Windows.Application.Current.Host.Content;
				double scaledLength = fPageAdapter.Orientation.IsPortrait ? content.ActualHeight : content.ActualWidth;
				double pixelHeight = (scaledLength * (double)content.ScaleFactor) / 100.0;
				return (int)Math.Round(pixelHeight, MidpointRounding.AwayFromZero);
			}
		}

		/// <summary>Gets the surface's orientation relative to the application's orientation.</summary>
		/// <value>
		///  <para>Returns Upright if the surface will render its content at the same orientation as the application.</para>
		///  <para>
		///   Returns SidewaysLeft, SidewaysRight, or UpsideDown if the surface is rotated compared
		///   to the applciation's current orientation. In this case, the caller is expected to rotate
		///   the content it renders via Direct3D so that is final content will be displayed upright
		///   relative to the application's current orientation.
		///  </para>
		///  <para>Returns Unknown if this adapter is not assigned a page reference.</para>
		/// </value>
		public Corona.WinRT.Interop.RelativeOrientation2D Orientation
		{
			get
			{
				var portraitOrientation = Corona.WinRT.Interop.UI.PageOrientation.PortraitUpright;
				return portraitOrientation.ToOrientationRelativeTo(fPageAdapter.Orientation);
			}
		}

		#endregion


		#region Event Handlers
		/// <summary>
		///  <para>Called when the page that is hosting the rendering surface has changed orientations.</para>
		///  <para>Raises a "Resized" event if the orientation has changed from portrait to landscape or vice-versa.</para>
		/// </summary>
		/// <param name="sender">The page that raised this event.</param>
		/// <param name="e">Event arguments providing the new page orientation.</param>
		private void OnOrientationChanged(
			Corona.WinRT.Interop.UI.IPage sender, Corona.WinRT.Interop.UI.PageOrientationEventArgs e)
		{
			// Do not continue if the orientation has not changed.
			// Note: This can happen if this adapter was once given a different page reference.
			if (fLastReceivedOrientation == e.Orientation)
			{
				return;
			}

			// Raise a "Resized" event if the orientation has changed from portrait to landscape or vice-versa.
			// This is because this adapter provides the surface width and height relative to the app's orientation.
			bool wasResized =
					(fLastReceivedOrientation == Corona.WinRT.Interop.UI.PageOrientation.Unknown) ||
					(e.Orientation == Corona.WinRT.Interop.UI.PageOrientation.Unknown) ||
					(fLastReceivedOrientation.IsLandscape != e.Orientation.IsLandscape);
			fLastReceivedOrientation = e.Orientation;
			if (wasResized && (this.Resized != null))
			{
				this.Resized(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
			}
		}

		#endregion
	}
}
