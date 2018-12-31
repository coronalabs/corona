using System;


namespace CoronaLabs.Corona.AppTemplate
{
	/// <summary>The page that gets displayed on application startup.</summary>
	public partial class MainPage : Microsoft.Phone.Controls.PhoneApplicationPage
	{
		/// <summary>Initializes the page and its member variables.</summary>
		public MainPage()
		{
			// Initialize this page's components that were set up via the UI designer.
			InitializeComponent();

			// Set up Corona to automatically start up when the control's Loaded event has been raised.
			fCoronaPanel.AutoLaunchEnabled = true;

			// Set up the CoronaPanel control to render fullscreen via the DrawingSurfaceBackgroundGrid control.
			// This significantly improves the framerate and is the only means of achieving 60 FPS.
			fCoronaPanel.BackgroundRenderingEnabled = true;
			fDrawingSurfaceBackgroundGrid.SetBackgroundContentProvider(fCoronaPanel.BackgroundContentProvider);
			fDrawingSurfaceBackgroundGrid.SetBackgroundManipulationHandler(fCoronaPanel.BackgroundManipulationHandler);
		}
	}
}
