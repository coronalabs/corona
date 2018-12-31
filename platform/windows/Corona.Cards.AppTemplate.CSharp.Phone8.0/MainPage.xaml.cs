using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using CoronaCardsAppTemplate.Resources;


namespace CoronaCardsAppTemplate
{
	public partial class MainPage : PhoneApplicationPage
	{
		public MainPage()
		{
			// Initialize this page's components that were set up via the UI designer.
			InitializeComponent();

			// Set up Corona to automatically start up when the control's Loaded event has been raised.
			// Note: By default, Corona will run the "main.lua" file in the "Assets\Corona" directory.
			//       You can change the defaults via the CoronaPanel's AutoLaunchSettings property.
			fCoronaPanel.AutoLaunchEnabled = true;

			// Set up the CoronaPanel control to render fullscreen via the DrawingSurfaceBackgroundGrid control.
			// This significantly improves the framerate and is the only means of achieving 60 FPS.
			fCoronaPanel.BackgroundRenderingEnabled = true;
			fDrawingSurfaceBackgroundGrid.SetBackgroundContentProvider(fCoronaPanel.BackgroundContentProvider);
			fDrawingSurfaceBackgroundGrid.SetBackgroundManipulationHandler(fCoronaPanel.BackgroundManipulationHandler);
		}
	}
}
