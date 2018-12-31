Imports System.Diagnostics
Imports System.Resources
Imports System.Windows.Markup

Partial Public Class App
    Inherits Application

    ''' <summary>
    ''' Provides easy access to the root frame of the Phone Application.
    ''' </summary>
    ''' <returns>The root frame of the Phone Application.</returns>
    Public Shared Property RootFrame As PhoneApplicationFrame

    ''' <summary>
    ''' Constructor for the Application object.
    ''' </summary>
    Public Sub New()
        ' Standard XAML initialization
        InitializeComponent()

        ' Phone-specific initialization
        InitializePhoneApplication()

        ' Language display initialization
        InitializeLanguage()

        ' Show graphics profiling information while debugging.
        If Debugger.IsAttached Then
            ' Display the current frame rate counters.
            Application.Current.Host.Settings.EnableFrameRateCounter = True

            ' Show the areas of the app that are being redrawn in each frame.
            'Application.Current.Host.Settings.EnableRedrawRegions = True

            ' Enable non-production analysis visualization mode,
            ' which shows areas of a page that are handed off to GPU with a colored overlay.
            'Application.Current.Host.Settings.EnableCacheVisualization = True


            ' Prevent the screen from turning off while under the debugger by disabling
            ' the application's idle detection.
            ' Caution:- Use this under debug mode only. Application that disables user idle detection will continue to run
            ' and consume battery power when the user is not using the phone.
            PhoneApplicationService.Current.UserIdleDetectionMode = IdleDetectionMode.Disabled
        End If
    End Sub

    ' Code to execute when the application is launching (eg, from Start)
    ' This code will not execute when the application is reactivated
    Private Sub Application_Launching(ByVal sender As Object, ByVal e As LaunchingEventArgs)
    End Sub

    ' Code to execute when the application is activated (brought to foreground)
    ' This code will not execute when the application is first launched
    Private Sub Application_Activated(ByVal sender As Object, ByVal e As ActivatedEventArgs)
    End Sub

    ' Code to execute when the application is deactivated (sent to background)
    ' This code will not execute when the application is closing
    Private Sub Application_Deactivated(ByVal sender As Object, ByVal e As DeactivatedEventArgs)
    End Sub

    ' Code to execute when the application is closing (eg, user hit Back)
    ' This code will not execute when the application is deactivated
    Private Sub Application_Closing(ByVal sender As Object, ByVal e As ClosingEventArgs)
    End Sub

    ' Code to execute if a navigation fails
    Private Sub RootFrame_NavigationFailed(ByVal sender As Object, ByVal e As NavigationFailedEventArgs)
        If Diagnostics.Debugger.IsAttached Then
            ' A navigation has failed; break into the debugger
            Diagnostics.Debugger.Break()
        End If
    End Sub

    Public Sub Application_UnhandledException(ByVal sender As Object, ByVal e As ApplicationUnhandledExceptionEventArgs) Handles Me.UnhandledException

        ' Show graphics profiling information while debugging.
        If Diagnostics.Debugger.IsAttached Then
            Diagnostics.Debugger.Break()
        Else
            e.Handled = True
            MessageBox.Show(e.ExceptionObject.Message & Environment.NewLine & e.ExceptionObject.StackTrace,
                            "Error", MessageBoxButton.OK)
        End If
    End Sub

#Region "Phone application initialization"
    ' Avoid double-initialization
    Private phoneApplicationInitialized As Boolean = False

    ' Do not add any additional code to this method
    Private Sub InitializePhoneApplication()
        If phoneApplicationInitialized Then
            Return
        End If

        ' Create the frame but don't set it as RootVisual yet; this allows the splash
        ' screen to remain active until the application is ready to render.
        RootFrame = New PhoneApplicationFrame()
        AddHandler RootFrame.Navigated, AddressOf CompleteInitializePhoneApplication

        ' Handle navigation failures
        AddHandler RootFrame.NavigationFailed, AddressOf RootFrame_NavigationFailed

        'Handle reset requests for clearing the backstack
        AddHandler RootFrame.Navigated, AddressOf CheckForResetNavigation

        ' Ensure we don't initialize again
        phoneApplicationInitialized = True
    End Sub

    ' Do not add any additional code to this method
    Private Sub CompleteInitializePhoneApplication(ByVal sender As Object, ByVal e As NavigationEventArgs)
        ' Set the root visual to allow the application to render
        If RootVisual IsNot RootFrame Then
            RootVisual = RootFrame
        End If

        ' Remove this handler since it is no longer needed
        RemoveHandler RootFrame.Navigated, AddressOf CompleteInitializePhoneApplication
    End Sub

    Private Sub CheckForResetNavigation(ByVal sender As Object, ByVal e As NavigationEventArgs)
        ' If the app has received a 'reset' navigation, then we need to check
        ' on the next navigation to see if the page stack should be reset
        If e.NavigationMode = NavigationMode.Reset Then
            AddHandler RootFrame.Navigated, AddressOf ClearBackStackAfterReset
        End If
    End Sub

    Private Sub ClearBackStackAfterReset(ByVal sender As Object, ByVal e As NavigationEventArgs)
        ' Unregister the event so it doesn't get called again
        RemoveHandler RootFrame.Navigated, AddressOf ClearBackStackAfterReset

        ' Only clear the stack for 'new' (forward) and 'refresh' navigations
        If e.NavigationMode <> NavigationMode.New And e.NavigationMode <> NavigationMode.Refresh Then
            Return
        End If

        ' For UI consistency, clear the entire page stack
        While Not RootFrame.RemoveBackEntry() Is Nothing
            ' do nothing
        End While
    End Sub
#End Region

    ' Initialize the app's font and flow direction as defined in its localized resource strings.
    '
    ' To ensure that the font of your application is aligned with its supported languages and that the
    ' FlowDirection for each of those languages follows its traditional direction, ResourceLanguage
    ' and ResourceFlowDirection should be initialized in each resx file to match these values with that
    ' file's culture. For example:
    '
    ' AppResources.es-ES.resx
    '    ResourceLanguage's value should be "es-ES"
    '    ResourceFlowDirection's value should be "LeftToRight"
    '
    ' AppResources.ar-SA.resx
    '     ResourceLanguage's value should be "ar-SA"
    '     ResourceFlowDirection's value should be "RightToLeft"
    '
    ' For more info on localizing Windows Phone apps see http://go.microsoft.com/fwlink/?LinkId=262072.
    '
    Private Sub InitializeLanguage()
        Try
            ' Set the font to match the display language defined by the
            ' ResourceLanguage resource string for each supported language.
            '
            ' Fall back to the font of the neutral language if the Display
            ' language of the phone is not supported.
            '
            ' If a compiler error is hit then ResourceLanguage is missing from
            ' the resource file.
            RootFrame.Language = XmlLanguage.GetLanguage(AppResources.ResourceLanguage)

            ' Set the FlowDirection of all elements under the root frame based
            ' on the ResourceFlowDirection resource string for each
            ' supported language.
            '
            ' If a compiler error is hit then ResourceFlowDirection is missing from
            ' the resource file.
            Dim flow As FlowDirection = DirectCast([Enum].Parse(GetType(FlowDirection), AppResources.ResourceFlowDirection), FlowDirection)
            RootFrame.FlowDirection = flow
        Catch
            ' If an exception is caught here it is most likely due to either
            ' ResourceLangauge not being correctly set to a supported language
            ' code or ResourceFlowDirection is set to a value other than LeftToRight
            ' or RightToLeft.

            If Debugger.IsAttached Then
                Debugger.Break()
            End If

            Throw
        End Try
    End Sub

End Class