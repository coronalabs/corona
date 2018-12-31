using System;


namespace CoronaLabs.Corona.WinRT.Phone
{
	/// <summary>
	///  Corona Xaml control used to execute a Corona project's Lua scripts and render its content via Direct3D.
	/// </summary>
	/// <remarks>
	///  <para>
	///   In order to have this control run a Corona project, you must access this control's <see cref="Runtime"/>
	///   property and call its <see cref="M:CoronaLabs.Corona.WinRT.CoronaRuntime.Run"/> method. By default,
	///   this method will attempt to find and execute a Corona "main.lua" or "resource.car" file under the application's
	///   "Assets\Corona" directory. If you are a "CoronaCards" developer, then you can provide a different path
	///   to one of these files via the <see cref="M:CoronaLabs.Corona.WinRT.CoronaRuntime.RunUsing"/> method.
	///  </para>
	///  <para>
	///   This control will automatically suspend the Corona runtime when the application becomes suspended and will
	///   automatically resume the Corona runtime when the application resumes.
	///  </para>
	///  <para>
	///   This control will automatically terminate the Corona runtime (i.e.: raise an "applicationExit" event in Lua),
	///   when the control has been removed from the application page or when the end-user backs out of the page
	///   that is hosting the Corona control.
	///  </para>
	///  <para>This control can render content in one of two ways:</para>
	///  <para>
	///   <list type="number">
	///    <item>
	///     <description>
	///      By default, content will be rendered within the bounds of the control. The advantage of this method is
	///      that the control and its content are resizable and can be displayed along with other controls on the page.
	///      The disadvantage of this method is that this has poor rendering performance because content is rendered
	///      to a Direct3D texture on every frame.
	///     </description>
	///    </item>
	///    <item>
	///     <description>
	///      For best performance (i.e.: highest framerate possible) you can render content to a Xaml
	///      DrawingSurfaceBackgroundGrid control, which renders content fullscreen in the background. You can do this
	///      by setting this control's <see cref="BackgroundRenderingEnabled"/> property to true and passing the objects
	///      returned by properties <see cref="BackgroundContentProvider"/> and <see cref="BackgroundManipulationHandler"/>
	///      to the DrawingSurfaceBackgroundGrid. By doing this, the Corona control's background becomes invisible,
	///      but it will still display its child controls placed inside of it (see below).
	///     </description>
	///    </item>
	///   </list>
	///  </para>
	///  <para>
	///   This Corona control can also be a container for other Xaml controls via the <see cref="Children"/> property.
	///   You can also drag and drop Xaml controls within this control via the Visual Studio designer. When creating
	///   controls via Corona's native APIs in Lua such as text fields and web views, these controls are placed within
	///   the Corona control. If you are rendering Corona content fullscreen via a DrawingSurfaceBackgroundGrid, then
	///   it is highly recommended to display this Corona control fullscreen because its child controls are positioned
	///   relative to the coordinates within the Corona control.
	///  </para>
	/// </remarks>
	[System.Windows.Markup.ContentProperty("Children")]
	public class CoronaPanel : System.Windows.Controls.UserControl
	{
		#region Member Variables
		/// <summary>Corona runtime which runs Lua scripts and renders to a Direct3D surface.</summary>
		private CoronaRuntime fRuntime;

		/// <summary>
		///  <para>
		///   Set true if the Corona runtime should automatically be started when this control has been loaded onto the page.
		///  </para>
		///  <para>Set false if the owner of this control must call the CoronaRuntime's Run() method manually.</para>
		/// </summary>
		private bool fAutoLaunchEnabled;

		/// <summary>
		///  The launch settings that will be passed into the CoronaRuntime's Run() method if it is being automatically
		///  launched when this control has been loaded.
		/// </summary>
		private CoronaRuntimeLaunchSettings fAutoLaunchSettings;

		/// <summary>Stores a reference to the page that is hosting this control.</summary>
		private Corona.WinRT.Interop.UI.PageProxy fPageProxy;

		/// <summary>
		///  <para>The URI/URL name assigned to this control's parent page.</para>
		///  <para>Will be null if this control is not currently assigned to a page.</para>
		///  <para>This URI is needed to identify this page in the application's navigation history.</para>
		/// </summary>
		private System.Uri fPageUri;

		/// <summary>
		///  <para>
		///   The grid assigned to this CoronaPanel's Content property, used to store all internal and public child controls.
		///  </para>
		///  <para>This CoronaPanel's Children property does not have access to this grid.</para>
		/// </summary>
		private System.Windows.Controls.Grid fRootGrid;

		/// <summary>
		///  <para>Grid which is overlaid on top of this CoronaPanel's DrawingSurface and other internal controls.</para>
		///  <para>The owner of this CoronaPanel adds its controls to this grid via the CoronaPanel.Children property</para>
		///  <para>Child controls added to the CoronaPanel via the Visual Studio UI designer will be added to this grid.</para>
		/// </summary>
		private System.Windows.Controls.Grid fOverlaidGrid;

		/// <summary>
		///  <para>Invisible rectangle used to work-around a Windows Phone bug with rendering text to a bitmap.</para>
		///  <para>See method OnTextRendererIsRendering() for more details.</para>
		/// </summary>
		private System.Windows.Shapes.Rectangle fInvisibleRectangle;

		/// <summary>The Direct3D drawing surface this Corona panel will use when not rendering to the background.</summary>
		private System.Windows.Controls.DrawingSurface fDrawingSurface;

		/// <summary>
		///  Proxy which allows this control to hot swap between rendering to the DrawingSurface and
		///  DrawingSurfaceBackgroundGrid Xaml controls. This proxy is what the CoronaRuntime renders to.
		/// </summary>
		private Corona.WinRT.Interop.Graphics.RenderSurfaceProxy fRenderSurfaceProxy;

		/// <summary>Provides a Direct3DSurfaceAdapter for a resizable .NET "DrawingSurface" Xaml control.</summary>
		private Interop.Graphics.DotNetDrawingSurfaceAdapter fDrawingSurfaceAdapter;

		/// <summary>Provides a Direct3DSurfaceAdapter for a .NET "DrawingSurfaceBackgroundGrid" Xaml control.</summary>
		private Interop.Graphics.DotNetBackgroundSurfaceAdapter fBackgroundSurfaceAdapter;

		/// <summary>
		///  Set true if rendering to a DrawingSurfaceBackgroundGrid. Set false if rendering to a resizable DrawingSurface.
		/// </summary>
		private bool fIsBackgroundRenderingEnabled;

		/// <summary>
		///  <para>Nullable boolean type that is only expected to be used by the "CanRenderToBackground" property.</para>
		///  <para>This improves performance by only validating the system upon the first call to that property.</para>
		/// </summary>
		private bool? fCanRenderToBackground;

		/// <summary>Provides input events such as taps, touches, and key events.</summary>
		private Interop.Input.DotNetInputDeviceServices fInputDeviceServices;

		#endregion


		#region Constructors
		/// <summary>Creates a new Corona panel.</summary>
		public CoronaPanel() : base()
		{
			// Add the root grid control to this CoronaPanel used to store all child controls.
			fRootGrid = new System.Windows.Controls.Grid();
			this.Content = fRootGrid;

			// Overaly another grid on top of this control.
			// The owner of this CoronaPanel adds child controls to this grid in the UI designer or "Childrens" property.
			fOverlaidGrid = new System.Windows.Controls.Grid();
			fRootGrid.Children.Add(fOverlaidGrid);

			// Initialize member variables used by the UI designer and at runtime.
			fCanRenderToBackground = null;

			// If this control is being displayed in the Visual Studio UI designer,
			// then display the Corona logo and do nothing else.
			if (System.ComponentModel.DesignerProperties.IsInDesignTool)
			{
				// Set the background color to orange when in the UI designer.
				fRootGrid.Background = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Orange);

				// Build a relative path to the Corona logo resource file.
				var stringBuilder = new System.Text.StringBuilder();
				stringBuilder.Append('/');
				stringBuilder.Append(this.GetType().Assembly.GetName().Name);
				stringBuilder.Append(";component/CoronaPanel.Designer.png");
				var uri = new Uri(stringBuilder.ToString(), UriKind.Relative);

				// Set up an image control to display the Corona logo image.
				var image = new System.Windows.Controls.Image();
				image.Source = new System.Windows.Media.Imaging.BitmapImage(uri);
				image.Stretch = System.Windows.Media.Stretch.None;
				fRootGrid.Children.Insert(0, image);
				return;
			}

			// Verify that at least one of Corona's required native libraries is installed.
			var coronaNativeLibraryPath = System.IO.Path.Combine(
					Windows.ApplicationModel.Package.Current.InstalledLocation.Path, "CoronaLabs.Corona.Component.dll");
			if (System.IO.File.Exists(coronaNativeLibraryPath) == false)
			{
				String message =
						"Failed to load 'CoronaPanel' control because Corona's native C/C++/CX libraries are missing.\n" +
						"This can happen if the application was built in Visual Studio with the 'Any CPU' platform which " +
						"will only includes .NET libraries. To solve this issue, rebuild your application using platform" +
						"'ARM', 'x86', or 'Mixed Platforms'.";
				throw new System.DllNotFoundException(message);
			}

			// Create a near invisible rectangle and add it to this control.
			// Note: This works-around a Windows Phone bug with text rendering.
			//       Also, this rectangle can't be completely invisible (alpha of zero) for this fix to work.
			//       See method OnTextRendererIsRendering() for more details.
			fInvisibleRectangle = new System.Windows.Shapes.Rectangle();
			fInvisibleRectangle.Width = 1.0;
			fInvisibleRectangle.Height = 1.0;
			fInvisibleRectangle.Fill =
					new System.Windows.Media.SolidColorBrush(System.Windows.Media.Color.FromArgb(1, 0, 0, 0));
			fRootGrid.Children.Add(fInvisibleRectangle);

			// Create an event handler which keeps a weak reference to this control.
			// Note: This allows this control to be garbage collected when adding strong referenced event handlers to
			//       a .NET object that is being referenced on the C++/CX side, such as the CoronaRuntime C++/CX object.
			var weakEventHandler = new CoronaPanel.WeakEventHandler(this);

			// Initialize member variables.
			fAutoLaunchEnabled = false;
			fAutoLaunchSettings = new CoronaRuntimeLaunchSettings();
			fPageProxy = new Corona.WinRT.Interop.UI.PageProxy();
			fPageUri = null;
			fRenderSurfaceProxy = new WinRT.Interop.Graphics.RenderSurfaceProxy();
			fDrawingSurfaceAdapter = new Interop.Graphics.DotNetDrawingSurfaceAdapter();
			fBackgroundSurfaceAdapter = new Interop.Graphics.DotNetBackgroundSurfaceAdapter();
			fIsBackgroundRenderingEnabled = false;
			fInputDeviceServices = new Interop.Input.DotNetInputDeviceServices();

			// Set up Corona's Lua print() and C/C++ Rtt_Log() functions to write to the Managed debugger, if available.
			// If the .NET Managed debugger is not attached, then the C/C++ side will attach a native logger later.
			var dotNetLogger = Corona.WinRT.Interop.Logging.DotNetDebuggerLogger.Instance;
			if (dotNetLogger.CanWrite)
			{
				var currentLogger = Corona.WinRT.Interop.Logging.LoggingServices.Logger;
				if ((currentLogger == null) || (currentLogger != dotNetLogger))
				{
					Corona.WinRT.Interop.Logging.LoggingServices.Logger = dotNetLogger;
				}
			}

			// Set up the resizable Direct3D drawing surface, but don't add it to this control yet.
			// To be added in the OnLoaded() method and only if background rendering is disabled.
			fDrawingSurface = new System.Windows.Controls.DrawingSurface();
			fDrawingSurface.Margin = new System.Windows.Thickness(0);
			fDrawingSurface.Loaded += OnDrawingSurfaceLoaded;
			fDrawingSurface.Unloaded += OnDrawingSurfaceUnloaded;

			// Create and set up the image services interop layer.
			var imageServices = new Interop.Graphics.DotNetImageServices();
			imageServices.CreatedTextRenderer += weakEventHandler.OnCreatedTextRenderer;

			// Create and set up the Corona runtime.
			var settings = new Corona.WinRT.Interop.CoronaInteropSettings();
			settings.ApplicationServices = new Interop.DotNetApplicationServices();
			settings.ImageServices = imageServices;
			settings.InputDeviceServices = fInputDeviceServices;
			settings.NetworkServices = new Interop.Networking.DotNetNetworkServices();
			settings.ResourceServices = Corona.WinRT.Interop.Storage.DotNetResourceServices.Instance;
			settings.TimerServices = new Interop.DotNetTimerServices();
			settings.UserInterfaceServices = new Interop.UI.DotNetUserInterfaceServices();
			settings.WeakCoronaControlReference = new Corona.WinRT.Interop.DotNetReadOnlyWeakReference(this);
			fRuntime = new CoronaRuntime(settings);

			// Add event handlers.
			this.Loaded += OnLoaded;
			this.Unloaded += OnUnloaded;
		}

		#endregion


		#region Event Handlers
		/// <summary>Called when this control has been added to a page and is about to be displayed.</summary>
		/// <param name="sender">Reference to this CoronaPanel control.</param>
		/// <param name="e">Event arguments indicating which object raised this event.</param>
		private void OnLoaded(object sender, System.Windows.RoutedEventArgs e)
		{
			// Fetch this control's parent page.
			// Note: Should never return null in a "Loaded" event, but check just in case.
			var parentPage = Interop.UI.DotNetPageServices.FetchParentPageFrom(this);
			if (parentPage == null)
			{
				OnPageLost();
				return;
			}

			// Check if this control was already assigned a page.
			// Note: This can happen when navigating forward to another page within the app.
			//       Navigating forward causes the page to be unloaded and navigating back causes it to be re-laoded.
			var pageAdapter = fPageProxy.Page as Interop.UI.DotNetPageAdapter;
			if (pageAdapter != null)
			{
				// Do not continue if the parent page hasn't change. Let the Corona runtime resume instead.
				if (pageAdapter.Page == parentPage)
				{
					return;
				}

				// The parent page has changed.
				// Terminate the Corona runtime and then initialize it with the new parent page down below.
				OnPageLost();
			}

			// Add static event handlers.
			System.Windows.Application.Current.Exit += OnApplicationExiting;
			PageMonitor.JournalEntryRemoved += OnJournalEntryRemoved;

			// Make this control's parent page available to Corona's interop layer.
			if (pageAdapter == null)
			{
				pageAdapter = new Interop.UI.DotNetPageAdapter();
			}
			pageAdapter.Page = parentPage;
			fPageProxy.Page = pageAdapter;
			fBackgroundSurfaceAdapter.Page = pageAdapter.Page;
			fPageUri = parentPage.NavigationService.Source;

			// Set up this conrol to render to the DrawingSurface or the background depending on what is currently enabled.
			UpdateRenderSurface();

			// Start the Corona runtime if auto-launch has been enabled.
			if (fAutoLaunchEnabled && (fRuntime != null))
			{
				if ((fRuntime.State == CoronaRuntimeState.NotStarted) ||
				    (fRuntime.State == CoronaRuntimeState.Terminated))
				{
					fRuntime.RunUsing(fAutoLaunchSettings);
				}
			}
		}

		/// <summary>
		///  <para>Called when this control is no longer displayed on screen.</para>
		///  <para>
		///   This can happen when the control has been removed from the page or when the page is no longer on screen.
		///  </para>
		/// </summary>
		/// <param name="sender">Reference to this CoronaPanel control.</param>
		/// <param name="e">Event arguments indicating which object raised this event.</param>
		private void OnUnloaded(object sender, System.Windows.RoutedEventArgs e)
		{
			// Do not continue if this control was never loaded to begin with.
			// Note: This should never happen, but check just in case.
			var pageAdapter = fPageProxy.Page as Interop.UI.DotNetPageAdapter;
			if ((pageAdapter == null) || (pageAdapter.Page == null))
			{
				return;
			}

			// Do not continue if this control is still attached to its parent page. Let Corona suspend instead.
			// Note: This handles the case where its parent page was unloaded.
			var parentPage = Interop.UI.DotNetPageServices.FetchParentPageFrom(this);
			if (parentPage != null)
			{
				// Do not terminate the Corona runtime if its parent page is still in the application's navigation history.
				var parentPageJournalEntry = PageMonitor.GetJournalEntryBy(parentPage);
				if (parentPageJournalEntry != null)
				{
					return;
				}

				// Fetch the journal entry of the first page in the application's navigation history.
				// If the end-user navigated forward, then this will be the parent page's journal entry.
				// If the end-user navigated backwards, then this will be null (no more pages) or a different page.
				System.Windows.Navigation.JournalEntry previousPageJournalEntry = null;
				foreach (var nextJournalEntry in parentPage.NavigationService.BackStack)
				{
					previousPageJournalEntry = nextJournalEntry;
					break;
				}

				// Check if the first page in the navigation history matches this control's parent page.
				if ((previousPageJournalEntry != null) && previousPageJournalEntry.Source.Equals(fPageUri))
				{
					// The page's URI matches this control's parent page URI. This means that the page classes are the same.
					// Next, we need to check if the previous page class instance matches this control's parent page instance.
					var previousPage = PageMonitor.GetPageBy(previousPageJournalEntry);
					if (previousPage == null)
					{
						// It's a match! Start monitoring this page and suspend out.
						PageMonitor.Add(parentPage, previousPageJournalEntry);
						return;
					}
					else if (previousPage == parentPage)
					{
						// It's a match! And we're already monitoring this page. Suspend out.
						return;
					}
				}
			}

			// This control has either been detached from its parent page or the application has released the parent page.
			// Terminate the Corona runtime and remove its reference to this page, allowing it to be garbage collected.
			OnPageLost();
		}

		/// <summary>Called when the application is about to exit out.</summary>
		/// <param name="sender">The application object that is raising this event.</param>
		/// <param name="e">Empty event arguments.</param>
		private void OnApplicationExiting(object sender, EventArgs e)
		{
			// Ensure that this control unloads itself and releases its native resources upon app exit.
			// This is needed because backing out of a WP8 app does not raise an Unloaded event.
			OnPageLost();
		}

		/// <summary>Called when a page has been removed from the application's navigation history.</summary>
		/// <remarks>
		///  This occurs when the end-user backs to a previous page within the app or when
		///  a page has been explicitly removed from the application's "BackStack" via code.
		/// </remarks>
		/// <param name="sender">Set to null since this event comes from the private inner PageMonitor class.</param>
		/// <param name="e">Provides the page that was removed from the application's navigation history.</param>
		private void OnJournalEntryRemoved(object sender, PageMonitor.PageEventArgs e)
		{
			// Validate.
			if ((e == null) || (e.Page == null))
			{
				return;
			}

			// Fetch this control's parent page, if it still has one.
			var pageAdapter = fPageProxy.Page as Interop.UI.DotNetPageAdapter;
			if (pageAdapter == null)
			{
				return;
			}
			var parentPage = pageAdapter.Page;
			if (parentPage == null)
			{
				return;
			}
			
			// If this control's page is th one that was removed, then terminate the Corona runtime
			// and let go of the parent page reference so that the page can be garbage collected.
			if (e.Page == parentPage)
			{
				OnPageLost();
			}
		}

		/// <summary>
		///  <para>This method is to be called by this control directly when it has lost its parent page reference.</para>
		///  <para>This typically happens during the "Unloaded" or "JournalEntryRemoved" events.</para>
		/// </summary>
		private void OnPageLost()
		{
			// Do not continue if Corona and this control has already been unloaded.
			if (fPageProxy.Page == null)
			{
				return;
			}

			// Remove the static event handlers. This allows this control to be garbage collected.
			System.Windows.Application.Current.Exit -= OnApplicationExiting;
			PageMonitor.JournalEntryRemoved -= OnJournalEntryRemoved;

			// Terminate the Corona runtime.
			if (fRuntime != null)
			{
				fRuntime.Terminate();
			}

			// Remove the page reference that this control is no longer attached to.
			var pageAdapter = fPageProxy.Page as Interop.UI.DotNetPageAdapter;
			if (pageAdapter != null)
			{
				pageAdapter.Page = null;
			}
			fPageProxy.Page = null;
			fBackgroundSurfaceAdapter.Page = null;

			// Disable rendering with the DrawingSurface control, if used.
			// Note: Setting this to null will set its Direct3D content provider to null too.
			fDrawingSurfaceAdapter.DrawingSurface = null;

			// Stop listening for UI input events.
			fInputDeviceServices.UIEventSource = null;
		}

		/// <summary>Called when "fDrawingSurface" is about to be displayed onscreen.</summary>
		/// <param name="sender">Reference to member variable "fDrawingSurface" that raised this event.</param>
		/// <param name="e">Event arguments indicating which object raised this event.</param>
		private void OnDrawingSurfaceLoaded(object sender, System.Windows.RoutedEventArgs e)
		{
			// Set up Corona's Direct3D adapter to render to the DrawingSurface control.
			fDrawingSurfaceAdapter.DrawingSurface = fDrawingSurface;
		}

		/// <summary>Called when "fDrawingSurface" has been removed from the screen.</summary>
		/// <param name="sender">Reference to member variable "fDrawingSurface" that raised this event.</param>
		/// <param name="e">Event arguments indicating which object raised this event.</param>
		private void OnDrawingSurfaceUnloaded(object sender, System.Windows.RoutedEventArgs e)
		{
			// Disable rendering to the DrawingSurface control.
			fDrawingSurfaceAdapter.DrawingSurface = null;
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets a collection of child UI elements/control to be displayed within the Corona panel's bounds.</summary>
		/// <remarks>
		///  The owner of this Corona panel control can use this property to add Xaml controls within Corona control.
		///  In the Visual Studio UI designer, dragging and dropping Xaml control within this Corona control causes
		///  them to be added to this property's collection.
		/// </remarks>
		/// <value>Collection of UI elements/controls to be displayed within the Corona panel's bounds.</value>
		public System.Windows.Controls.UIElementCollection Children
		{
			get { return fOverlaidGrid.Children; }
		}

		/// <summary>
		///  Sets whether or not this control will automatically start the Corona runtime when loaded onto the page.
		/// </summary>
		/// <value>
		///  <para>
		///   Set true to automatically start the Corona runtime when the control has been loaded onto the page.
		///   This control will use the <see cref="AutoLaunchSettings"/> property when launching the Corona runtime
		///   which you can customize.
		///  </para>
		///  <para>
		///   Set false (the default) to start this control's Corona runtime manually.
		///   In this case, you must call this control's <see cref="Runtime">Runtime</see> Run() method yourself.
		///  </para>
		/// </value>
		public bool AutoLaunchEnabled
		{
			get { return fAutoLaunchEnabled; }
			set { fAutoLaunchEnabled = value; }
		}

		/// <summary>
		///  <para>
		///   Gets the settings to be used when this control automatically starts up the Corona runtime when loaded
		///   onto the page.
		///  </para>
		///  <para>Only applicable when the <see cref="AutoLaunchEnabled"/> property has been set to true.</para>
		/// </summary>
		/// <value>Customizable settings to be used when automatically launching the Corona runtime.</value>
		public CoronaRuntimeLaunchSettings AutoLaunchSettings
		{
			get { return fAutoLaunchSettings; }
		}

		/// <summary>
		///  Gets or sets whether or not Corona will render content within this control's bounds
		///  or to a DrawingSurfaceBackgroundGrid Xaml control.
		/// </summary>
		/// <value>
		///  <para>
		///   Set to true if this Corona control is set up to render content to a DrawingSurfaceBackgroundGrid control which
		///   provides the best rendering performance. In this case, this Corona control's background will be made invisible
		///   and content will be rendered via this control's <see cref="BackgroundContentProvider"/> object which is
		///   expected to be passed into the DrawingSurfaceBackgroundGrid control's SetBackgroundContentProvider() method.
		///  </para>
		///  <para>
		///   Set to false (the default) if content will be rendered within the bounds of this Corona control. In this case,
		///   this control's <see cref="BackgroundContentProvider"/> will not provide content and do nothing.
		///  </para>
		/// </value>
		public bool BackgroundRenderingEnabled
		{
			get { return fIsBackgroundRenderingEnabled; }
			set
			{
				// Do not allow background rendering to be enabled if we can't do it reliably.
				// Note: This works-around Microsoft scaling bugs on Windows 10 Mobile.
				if (value && (this.CanRenderToBackground == false))
				{
					value = false;
				}

				// Do not continue if this property's value isn't changing.
				if (value == fIsBackgroundRenderingEnabled)
				{
					return;
				}

				// Store the given setting.
				fIsBackgroundRenderingEnabled = value;
				UpdateRenderSurface();
			}
		}

		/// <summary>
		///  Gets an object used to render content to a DrawingSurfaceBackgroundGrid Xaml control if the
		///  <see cref="BackgroundRenderingEnabled"/> property is set to true.
		/// </summary>
		/// <value>
		///  Object used to render content fullscreen for best performance. It is expected to be passed into a
		///  DrawingSurfaceBackgroundGrid control's SetBackgroundContentProvider() method. This object will only
		///  render content if this control's <see cref="BackgroundRenderingEnabled"/> property is set to true.
		/// </value>
		public object BackgroundContentProvider
		{
			get
			{
				if (this.CanRenderToBackground == false)
				{
					return null;
				}
				return fBackgroundSurfaceAdapter.BackgroundContentProvider;
			}
		}

		/// <summary>
		///  Gets an object used to receive touch input from a DrawingSurfaceBackgroundGrid Xaml control if the
		///  <see cref="BackgroundRenderingEnabled"/> property is set to true.
		/// </summary>
		/// <value>
		///  Object used to receive touch input from a DrawingSurfaceBackground control when passed into its
		///  SetBackgroundManipulationHandler() method. This object will only be used when this control's
		///  <see cref="BackgroundRenderingEnabled"/> property is set to true.
		/// </value>
		public object BackgroundManipulationHandler
		{
			get
			{
				if (this.CanRenderToBackground == false)
				{
					return null;
				}
				return fBackgroundSurfaceAdapter.BackgroundManipulationHandler;
			}
		}

		/// <summary>Gets the 3D rendering surface that the Corona runtime renders to.</summary>
		/// <value>Reference to the control's 3D rendering surface.</value>
		public Corona.WinRT.Interop.Graphics.IRenderSurface RenderSurface
		{
			get { return fRenderSurfaceProxy; }
		}

		/// <summary>Gets a Corona runtime used to run, suspend, resume, and terminate a Corona project.</summary>
		/// <value>Corona runtime used to run, suspend, resume, and terminate a Corona project.</value>
		public CoronaRuntime Runtime
		{
			get { return fRuntime; }
		}

		#endregion


		#region Private Methods
		/// <summary>
		///  Updates this control's surface rendering handling to either a DrawingSurface or
		///  DrawingSurfaceBackgroundGrid Xaml control.
		/// </summary>
		private void UpdateRenderSurface()
		{
			// Do not continue if this control is being rendered in the Visual Studio UI designer.
			if (System.ComponentModel.DesignerProperties.IsInDesignTool)
			{
				return;
			}

			// Set up this control to render to a DrawingSurface or DrawingSurfaceBackgroundGrid.
			if (fIsBackgroundRenderingEnabled)
			{
				fBackgroundSurfaceAdapter.Page = Interop.UI.DotNetPageServices.FetchParentPageFrom(this);
				fRenderSurfaceProxy.RenderSurface = fBackgroundSurfaceAdapter.Direct3DSurfaceAdapter;
				fInputDeviceServices.UIEventSource = fBackgroundSurfaceAdapter.Page;
				if (fDrawingSurface != null)
				{
					fRootGrid.Children.Remove(fDrawingSurface);
				}
			}
			else
			{
				fBackgroundSurfaceAdapter.Page = null;
				fRenderSurfaceProxy.RenderSurface = fDrawingSurfaceAdapter.Direct3DSurfaceAdapter;
				if ((fDrawingSurface != null) && (fRootGrid.Children.Contains(fDrawingSurface) == false))
				{
					fRootGrid.Children.Insert(0, fDrawingSurface);
				}
				fInputDeviceServices.UIEventSource = fDrawingSurface;
			}
		}

		/// <summary>
		///  <para>Determines if Corona can render to a "DrawingSurfaceBackgroundGrid" reliably.</para>
		///  <para>If returns false, then background rendering should never be allowed to be enabled.</para>
		///  <para>
		///   Used to work-around a Microsoft bug on some Windows 10 Mobile devices where the native APIs
		///   return the wrong pixel resolution and/or scale factors.
		///  </para>
		/// </summary>
		/// <value>
		///  <para>Returns true if background rendering can be performed reliably.</para>
		///  <para>
		///   Returns false if unable to render to a "DrawingSurfaceBackgroundGrid" reliably and at the correct
		///   pixel resolution. In this case, the "BackgroundRenderingEnabled" property should not be allowed
		///   to enabled because only rendering to a "DrawingSurface" can be done reliably/correctly.
		///  </para>
		/// </value>
		private bool CanRenderToBackground
		{
			get
			{
				// Verify if we can render to the background reliably, if not done already.
				if (fCanRenderToBackground.HasValue == false)
				{
					// Initialize the nullable boolean type to true.
					fCanRenderToBackground = true;

					// Do not allow background rendering if we're running on Windows 10 Mobile that has scaling bugs.
					// ----------------------------------------------------------------------------------------------------
					// Microsoft has scaling bugs on Windows 10 Mobile devices that have an onscreen keyboard.
					// The OS' APIs will return the wrong pixel resolution and scale factor in this case,
					// preventing Corona from rendering content at the correct pixel size via Direct3D.
					// 
					// The only known work-around is for Corona to render its content to this CoronaPanel view
					// since Microsoft Silverlight knows how to render content at the correct width/height.
					// ----------------------------------------------------------------------------------------------------
					var application = System.Windows.Application.Current;
					if ((Environment.OSVersion.Version.Major >= 10) && (application != null))
					{
						// We're running on a Windows 10 Mobile device. Check for scaling bugs.
						var content = application.Host.Content;
						if (content.ScaleFactor != (int)Windows.Graphics.Display.DisplayProperties.ResolutionScale)
						{
							// The above scale factor properties differ when they shouldn't.
							// This device has a scaling bug. Do not allow backround rendering.
							fCanRenderToBackground = false;
						}
						else if (content.ScaleFactor > 100)
						{
							// Both of Microsoft's scale properties are correct. Next, check the screen resolution.
							var screenSize = new System.Windows.Size(content.ActualWidth, content.ActualHeight);
							object result = null;
							Microsoft.Phone.Info.DeviceExtendedProperties.TryGetValue(
									"PhysicalScreenResolution", out result);
							if (result is System.Windows.Size)
							{
								screenSize = (System.Windows.Size)result;
								if ((screenSize.Width == content.ActualWidth) &&
								    (screenSize.Height == content.ActualHeight))
								{
									// The pixel and content screen sizes should not equal with a scale greater than 1.0.
									// This device has a scaling bug. Do not allow backround rendering.
									fCanRenderToBackground = false;
								}
							}
						}
					}
				}

				// Return whether or not we can render to the background reliably.
				return fCanRenderToBackground.Value;
			}
		}

		#endregion


		#region Private WeakEventHandler Class
		/// <summary>Handles events for a CoronaPanel which this handler keeps a weak reference to.</summary>
		/// <remarks>
		///  This class should be used when the CoronaPanel needs to add event handlers to a .NET objects that is referenced
		///  by a C++/CX object the CoronaPanel owns, such as the interop objects reference by the CoronaRuntime.
		///  This resolves a circular reference issue and allows the CoronaPanel to be garbage collected.
		/// </remarks>
		private class WeakEventHandler
		{
			#region Private Member Variables
			private WeakReference<CoronaPanel> fWeakCoronaPanelReference;

			#endregion


			#region Constructors
			/// <summary>Creates a new weak event handler for the given CoronaPanel object.</summary>
			/// <param name="panel">
			///  <para>Reference to the CoronaPanel that handler will keep a weak reference to.</para>
			///  <para>Cannot be null or else an exception will be thrown.</para>
			/// </param>
			public WeakEventHandler(CoronaPanel panel)
			{
				// Validate argument.
				if (panel == null)
				{
					throw new NullReferenceException();
				}

				// Keep a weak reference to the given CoronaPanel control.
				fWeakCoronaPanelReference = new WeakReference<CoronaPanel>(panel);
			}

			#endregion


			#region Public Methods/Properties
			/// <summary>Gets the CoronaPanel that this handler keeps a weak reference to.</summary>
			/// <returns>
			///  <para>Returns a reference to the CoronaPanel that this handler is associated with.</para>
			///  <para>Returns null if the CoronaPanel has been garbage collected.</para>
			/// </returns>
			public CoronaPanel GetCoronaPanel()
			{
				CoronaPanel coronaPanel = null;
				bool wasReceived = fWeakCoronaPanelReference.TryGetTarget(out coronaPanel);
				return wasReceived ? coronaPanel : null;
			}

			/// <summary>Called when a new text renderer has been created by the DotNetImageServices object.</summary>
			/// <param name="sender">Reference to the DotNetImageServices object that raised this event.</param>
			/// <param name="e">Provides the text renderer that was just created.</param>
			public void OnCreatedTextRenderer(object sender, Interop.Graphics.DotNetTextRendererEventArgs e)
			{
				// Handle the new text renderer's "Rendering" event.
				e.TextRenderer.Rendering += OnTextRendererIsRendering;
			}

			/// <summary>Called when a text renderer is about to render text to the provided bitmap.</summary>
			/// <param name="sender">The DotNetTextRenderer that raised this event.</param>
			/// <param name="e">Provides the bitmap that is about to be rendered to.</param>
			public void OnTextRendererIsRendering(object sender, Interop.Graphics.DotNetWriteableBitmapEventArgs e)
			{
				// Fetch the Corona panel, if still available.
				CoronaPanel coronaPanel = GetCoronaPanel();
				if (coronaPanel == null)
				{
					return;
				}

				// This is a hack...
				// ----------------------------------------------------------------------------------------------------
				// This works-around a Windows Phone bug where rendering a Xaml control (such as a TextBlock)
				// that is not attached to the page to a WriteableBitmap is very unreliable if the application
				// is using a DrawingSurfaceBackgroundGrid control for fast Direct3D rendering.
				// Drawing a Xaml control that is attached to the page makes bitmap drawing much more reliable.
				// Here, we draw a near invisible rectangle (alpha is 1 out of 255) which can't be seen on the bitmap.
				// ----------------------------------------------------------------------------------------------------
				e.WriteableBitmap.Render(coronaPanel.fInvisibleRectangle, null);
			}

			#endregion
		}

		#endregion


		#region Private PageMonitor Class
		/// <summary>
		///  Private class intended to monitor the parent pages of all Corona controls and to notify those controls
		///  when their pages have been removed from the application and its navigation history.
		/// </summary>
		/// <remarks>
		///  You cannot create instances of this class. Instead, you are expected to access its static methods and events.
		/// </remarks>
		private class PageMonitor
		{
			#region Public PageEventArgs Class
			/// <summary>Provides a reference to a page the event is associated with.</summary>
			public class PageEventArgs : EventArgs
			{
				/// <summary>Reference to the page that the event is associated with.</summary>
				private Microsoft.Phone.Controls.PhoneApplicationPage fPage;

				/// <summary>Creates a new event arguments object providing the given page reference.</summary>
				/// <param name="page">Reference to a page. Cannot be null or else an exception will be thrown.</param>
				public PageEventArgs(Microsoft.Phone.Controls.PhoneApplicationPage page)
				{
					if (page == null)
					{
						throw new ArgumentNullException();
					}
					fPage = page;
				}

				/// <summary>Gets a reference to the page that the event is associated with.</summary>
				/// <value>Reference to a page.</value>
				public Microsoft.Phone.Controls.PhoneApplicationPage Page
				{
					get { return fPage; }
				}
			}

			#endregion


			#region Private PageJournalEntry Class
			/// <summary>Stores a page reference and its associated journal entry.</summary>
			/// <remarks>Instances of this class are immutable.</remarks>
			private class PageJournalEntry
			{
				/// <summary>The page associated with the journal entry.</summary>
				private Microsoft.Phone.Controls.PhoneApplicationPage fPage;

				/// <summary>The journal entry associated with the page.</summary>
				private System.Windows.Navigation.JournalEntry fJournalEntry;

				/// <summary>Creates a new page and journal entry pair.</summary>
				/// <param name="page">
				///  <para>Reference to the page associated with the given journal entry.</para>
				///  <para>Cannot be null or else an exception will be thrown.</para>
				/// </param>
				/// <param name="entry">
				///  <para>The application's navigation history journal entry for the given page.</para>
				///  <para>Cannot be null or else an exception will be thrown.</para>
				/// </param>
				public PageJournalEntry(
					Microsoft.Phone.Controls.PhoneApplicationPage page, System.Windows.Navigation.JournalEntry entry)
				{
					if ((page == null) || (entry == null))
					{
						throw new ArgumentNullException();
					}
					fPage = page;
					fJournalEntry = entry;
				}

				/// <summary>Gets the page reference.</summary>
				/// <value>The page reference associated with the journal entry.</value>
				public Microsoft.Phone.Controls.PhoneApplicationPage Page
				{
					get { return fPage; }
				}

				/// <summary>Gets the application's navigation journal entry for the page.</summary>
				/// <value>Reference to the page's journal entry.</value>
				public System.Windows.Navigation.JournalEntry JournalEntry
				{
					get { return fJournalEntry; }
				}
			}

			#endregion


			#region Private Member Variables
			/// <summary>Collection of all page and journal entry pairs that this class is monitoring.</summary>
			private static System.Collections.Generic.List<PageJournalEntry> sPageJournalEntries;

			/// <summary>
			///  <para>
			///   Set false if this class has not added its "JournalEntryRemoved" event handler to the application yet.
			///  </para>
			///  <para>Set true if it has.</para>
			/// </summary>
			private static bool sHasAddedEventHandler;

			#endregion


			#region Events
			/// <summary>
			///  <para>Raised when a page has been removed from the application's navigation handler.</para>
			///  <para>This happens when the end-user backs out of a page or it has been removed from the app via code.</para>
			/// </summary>
			public static event EventHandler<PageEventArgs> JournalEntryRemoved;

			#endregion


			#region Constructors
			/// <summary>Initializes all static member variables.</summary>
			static PageMonitor()
			{
				sPageJournalEntries = new System.Collections.Generic.List<PageJournalEntry>();
				sHasAddedEventHandler = false;
			}

			/// <summary>Constructor made private to prevent instances from being made.</summary>
			private PageMonitor()
			{
			}

			#endregion


			#region Public Methods/Properties
			/// <summary>Adds the given page reference and its associated journal entry to the monitor.</summary>
			/// <param name="page">The page to be monitored.</param>
			/// <param name="entry">The journal entry assigned to the given page by the application.</param>
			public static void Add(
				Microsoft.Phone.Controls.PhoneApplicationPage page, System.Windows.Navigation.JournalEntry entry)
			{
				// Validate arguments.
				if ((page == null) || (entry == null))
				{
					return;
				}

				// Add event handlers, if not done already.
				if (sHasAddedEventHandler == false)
				{
					var rootVisual = System.Windows.Application.Current.RootVisual;
					var appFrame = rootVisual as Microsoft.Phone.Controls.PhoneApplicationFrame;
					if (appFrame != null)
					{
						appFrame.JournalEntryRemoved += OnJournalEntryRemoved;
						sHasAddedEventHandler = true;
					}
				}

				// Check if the given page and journal entry pair has already been added to this class.
				for (int index = sPageJournalEntries.Count - 1; index >= 0; index--)
				{
					// Get the next stored page/entry pair.
					var nextPair = sPageJournalEntries[index];

					// Do not continue if the given page/entry pair already exists in this class' collection.
					if ((nextPair.Page == page) && (nextPair.JournalEntry == entry))
					{
						return;
					}

					// If the given page has a new journal entry or vice-versa, then remove the pair from the collection.
					// A new pair will be added down below for the new page settings.
					// Note: This should never happen, but exists as a safety mechanism.
					if ((nextPair.Page == page) || (nextPair.JournalEntry == entry))
					{
						sPageJournalEntries.RemoveAt(index);
						break;
					}
				}

				// Add the given page/entry pair to the collection.
				sPageJournalEntries.Add(new PageJournalEntry(page, entry));
			}

			/// <summary>Fetches the page associated with the given journal entry.</summary>
			/// <param name="entry">The page's journal entry.</param>
			/// <returns>
			///  <para>Returns the page associated with the given journal entry.</para>
			///  <para>Returns null if the given entry was never added to this monitor or if given a null argument.</para>
			/// </returns>
			public static Microsoft.Phone.Controls.PhoneApplicationPage GetPageBy(System.Windows.Navigation.JournalEntry entry)
			{
				if (entry != null)
				{
					foreach (var nextPair in sPageJournalEntries)
					{
						if (nextPair.JournalEntry == entry)
						{
							return nextPair.Page;
						}
					}
				}
				return null;
			}

			/// <summary>Fetches the journal entry associated with the given page reference.</summary>
			/// <param name="page">The page reference to fetch the journal entry for.</param>
			/// <returns>
			///  <para>Returns the journal entry associated with the given page reference.</para>
			///  <para>Returns null if the given page was never added to this monitor or if given a null argument.</para>
			/// </returns>
			public static System.Windows.Navigation.JournalEntry GetJournalEntryBy(
				Microsoft.Phone.Controls.PhoneApplicationPage page)
			{
				if (page != null)
				{
					foreach (var nextPair in sPageJournalEntries)
					{
						if (nextPair.Page == page)
						{
							return nextPair.JournalEntry;
						}
					}
				}
				return null;
			}

			#endregion


			#region Event Handlers
			/// <summary>Called when a page has been removed from the application and its navigation history.</summary>
			/// <param name="sender">The main application frame that raised this event.</param>
			/// <param name="e">Provides the journal entry of the page that was just removed.</param>
			private static void OnJournalEntryRemoved(
				object sender, System.Windows.Navigation.JournalEntryRemovedEventArgs e)
			{
				// Validate.
				if ((e == null) || (e.Entry == null))
				{
					return;
				}

				// Check if the page removed by the application exists in this monitor's collection.
				// Traverse all pages monitored by this class.
				for (int index = sPageJournalEntries.Count - 1; index >= 0; index--)
				{
					var nextPair = sPageJournalEntries[index];
					if (e.Entry == nextPair.JournalEntry)
					{
						// Found it! Now remove it from the collection so that the page can be garbaged collected.
						sPageJournalEntries.RemoveAt(index);

						// Notify the Corona control that its parent page has been removed.
						if (PageMonitor.JournalEntryRemoved != null)
						{
							PageMonitor.JournalEntryRemoved.Invoke(null, new PageEventArgs(nextPair.Page));
						}
						return;
					}
				}
			}

			#endregion
		}

		#endregion
	}
}
