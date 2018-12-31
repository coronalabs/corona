using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Graphics
{
	/// <summary>Adapter which wraps a .NET "DrawingSurface" Xaml control and provides it a Direct3D content provider.</summary>
	public class DotNetDrawingSurfaceAdapter : IXamlRenderSurfaceAdapter
	{
		#region Private Member Varaibles
		/// <summary>Reference to the resizable .NET Xaml control that renders Direct3D content.</summary>
		private System.Windows.Controls.DrawingSurface fDrawingSurface;

		/// <summary>Dispatcher used to queue and execute operations on the main UI thread.</summary>
		private Corona.WinRT.Phone.Interop.UI.DotNetDispatcher fDispatcher;

		/// <summary>The C++/CX Direct3D surface adapter used to render content to the Xaml DrawingSurface control.</summary>
		private Direct3DSurfaceAdapter fDirect3DSurfaceAdapter;

		#endregion


		#region Events
		/// <summary>Raised when the surface's width and/or height has changed.</summary>
		public event Windows.Foundation.TypedEventHandler<IXamlRenderSurfaceAdapter, CoronaLabs.WinRT.EmptyEventArgs> Resized;

		#endregion


		#region Constructors
		/// <summary>Creates a new adapter used to wrap a .NET "DrawingSurface" Xaml control.</summary>
		/// <remarks>
		///  This adapter does not reference a DrawingSurface upon creation. The caller is expected to assign this
		///  adapter a reference to an existing surface via its "DrawingSurface" property after creation.
		/// </remarks>
		public DotNetDrawingSurfaceAdapter()
		{
			fDrawingSurface = null;
			fDispatcher = new Corona.WinRT.Phone.Interop.UI.DotNetDispatcher();
			fDirect3DSurfaceAdapter = new Direct3DSurfaceAdapter(this);
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets or sets a reference to a .NET "DrawingSurface" Xaml control that this adapter will wrap.</summary>
		/// <remarks>
		///  <para>
		///   Setting this property to an existing DrawingSurface control will make it pass its Direct3D content provider
		///   and manipulation handler by calling the control's SetContentProvider() and SetManipulationHandler() methods.
		///  </para>
		///  <para>
		///   Setting this property to null will make this adapter remove its Direct3D content provider and manipulation
		///   handler by passing null to the control's SetContentProvider() and SetManipulationHandler() methods.
		///  </para>
		/// </remarks>
		/// <value>
		///  <para>Reference to a DrawingSurface Xaml control that this adapter wraps and provide information/events for.</para>
		///  <para>Set to null if this adapter is not assigned a surface.</para>
		/// </value>
		public System.Windows.Controls.DrawingSurface DrawingSurface
		{
			get { return fDrawingSurface; }
			set
			{
				double lastWidth = 0;
				double lastHeight = 0;
				double newWidth = 0;
				double newHeight = 0;

				// Do not continue if this adapter already has a reference to the given control.
				if (fDrawingSurface == value)
				{
					return;
				}

				// Remove this adapter's event handlers and content provider from the previous control.
				if (fDrawingSurface != null)
				{
					// Unsubscribe from events.
					fDrawingSurface.SizeChanged -= OnSizeChanged;

					// Remove the Direct3D content provider.
					fDrawingSurface.SetContentProvider(null);
					fDrawingSurface.SetManipulationHandler(null);

					// Fetch the control's width and height.
					lastWidth = fDrawingSurface.ActualWidth;
					lastHeight = fDrawingSurface.ActualHeight;
				}

				// Store the given control reference.
				fDrawingSurface = value;

				// Add this adapter's event handlers and Direct3D content provider to the given control.
				if (fDrawingSurface != null)
				{
					// Subscribe to the control's events.
					fDrawingSurface.SizeChanged += OnSizeChanged;

					// Add the Direct3D content provider.
					fDrawingSurface.SetContentProvider(fDirect3DSurfaceAdapter.ContentProvider);
					fDrawingSurface.SetManipulationHandler(fDirect3DSurfaceAdapter);

					// Fetch the control's width and height.
					newWidth = fDrawingSurface.ActualWidth;
					newHeight = fDrawingSurface.ActualHeight;
				}

				// Raise a resize event if the given control has a different size than the previous control.
				// Note: A null reference to a control is considered to have zero width/height by this adapter.
				double deltaWidth = Math.Abs(newWidth - lastWidth);
				double deltaHeight = Math.Abs(newHeight - lastHeight);
				if ((deltaWidth > double.Epsilon) || (deltaHeight > double.Epsilon))
				{
					if (this.Resized != null)
					{
						this.Resized.Invoke(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
					}
				}
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

		/// <summary>Determines if this adapter renders fullscreen to the background or to a resizable control.</summary>
		/// <value>Always returns false since this adapter wraps a .NET Xaml "DrawingSurface" control.</value>
		public bool IsRenderingToBackground
		{
			get { return false; }
		}

		/// <summary>Gets the width of the surface in pixels, relative to the application's orientations.</summary>
		/// <value>
		///  <para>The surface's width in pixels.</para>
		///  <para>Returns zero if this adapter is not assigned a "DrawingSurface" control.</para>
		/// </value>
		public int WidthInPixels
		{
			get
			{
				// Return zero if this adapter does not have a control reference.
				if (fDrawingSurface == null)
				{
					return 0;
				}

				// Return the control's width in pixels.
				double scaleFactor = (double)System.Windows.Application.Current.Host.Content.ScaleFactor / 100.0;
				double pixelWidth = fDrawingSurface.ActualWidth * scaleFactor;
				return (int)Math.Round(pixelWidth, MidpointRounding.AwayFromZero);
			}
		}

		/// <summary>Gets the height of the surface in pixels, relative to the application's orientations.</summary>
		/// <value>
		///  <para>The surface's height in pixels.</para>
		///  <para>Returns zero if this adapter is not assigned a "DrawingSurface" control.</para>
		/// </value>
		public int HeightInPixels
		{
			get
			{
				// Return zero if this adapter does not have a control reference.
				if (fDrawingSurface == null)
				{
					return 0;
				}

				// Return the control's width in pixels.
				double scaleFactor = (double)System.Windows.Application.Current.Host.Content.ScaleFactor / 100.0;
				double pixelHeight = fDrawingSurface.ActualHeight * scaleFactor;
				return (int)Math.Round(pixelHeight, MidpointRounding.AwayFromZero);
			}
		}

		/// <summary>Gets the surface's orientation relative to the application's orientation.</summary>
		/// <value>
		///  Always returns "upright" for this DrawingSurface adapter.
		///  This means that the caller does not need to rotate the rendered content.
		/// </value>
		public Corona.WinRT.Interop.RelativeOrientation2D Orientation
		{
			get { return Corona.WinRT.Interop.RelativeOrientation2D.Upright; }
		}

		#endregion


		#region Event Handlers
		/// <summary>
		///  <para>Called when this adapter's DrawingSurface control has been resized.</para>
		///  <para>Relays the event to this adapter's event handlers.</para>
		/// </summary>
		/// <param name="sender">The DrawingSurface control that has been resized.</param>
		/// <param name="e">Provides the surface's updated width and height.</param>
		private void OnSizeChanged(object sender, System.Windows.SizeChangedEventArgs e)
		{
			if (this.Resized != null)
			{
				this.Resized.Invoke(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
			}
		}

		#endregion
	}
}
