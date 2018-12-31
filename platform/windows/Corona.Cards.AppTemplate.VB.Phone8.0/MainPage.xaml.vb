Imports System
Imports System.Threading
Imports System.Windows.Controls
Imports Microsoft.Phone.Controls
Imports Microsoft.Phone.Shell


Partial Public Class MainPage
    Inherits PhoneApplicationPage

	Public Sub New()
		' Initialize this page's components that were set up via the UI designer.
		InitializeComponent()

		' Set up Corona to automatically start up when the control's Loaded event has been raised.
		' Note: By default, Corona will run the "main.lua" file in the "Assets\Corona" directory.
		'       You can change the defaults via the CoronaPanel's AutoLaunchSettings property.
		fCoronaPanel.AutoLaunchEnabled = True

		' Set up the CoronaPanel control to render fullscreen via the DrawingSurfaceBackgroundGrid control.
		' This significantly improves the framerate and is the only means of achieving 60 FPS.
		fCoronaPanel.BackgroundRenderingEnabled = True
		fDrawingSurfaceBackgroundGrid.SetBackgroundContentProvider(fCoronaPanel.BackgroundContentProvider)
		fDrawingSurfaceBackgroundGrid.SetBackgroundManipulationHandler(fCoronaPanel.BackgroundManipulationHandler)
	End Sub

End Class
