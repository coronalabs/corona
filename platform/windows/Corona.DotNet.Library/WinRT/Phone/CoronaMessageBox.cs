using System;


namespace CoronaLabs.Corona.WinRT.Phone
{
	/// <summary>
	///  <para>Non-blocking XAML based message box supporting custom button text.</para>
	///  <para>Displayed by Corona via the Lua native.showAlert() function.</para>
	/// </summary>
	/// <remarks>
	///  Microsoft's built-in Silverlight XAML <see cref="System.Windows.MessageBox"/> class blocks the main UI thread
	///  and does not support customizable buttons. This custom message box is non-blocking and does support multiple
	///  buttons (or no buttons) with text of your choosing. With this custom message box, you'll get similar
	///  alert dialog behavior as seen on iOS and Android, which makes it easier to port your app via Corona.
	/// </remarks>
	public class CoronaMessageBox : Corona.WinRT.Interop.UI.IMessageBox
	{
		#region Private Member Variables
		/// <summary>Stores the message box configuration such as title, message, and button labels.</summary>
		private Corona.WinRT.Interop.UI.MessageBoxSettings fSettings;

		/// <summary>
		///  <para>Zero based index of the last button that was pressed to close the message box.</para>
		///  <para>Set to -1 if not shown yet, end-user backed out, or the Close() method was called.</para>
		/// </summary>
		private int fLastButtonIndexPressed;

		/// <summary>
		///  <para>Reference to the application frame (aka: root visual) that is hosting the message box popup.</para>
		///  <para>Provides current page orientation and back key events.</para>
		/// </summary>
		private Microsoft.Phone.Controls.PhoneApplicationFrame fRootFrame;

		/// <summary>The XAML popup used to display the custom message box content.</summary>
		private System.Windows.Controls.Primitives.Popup fPopup;

		/// <summary>Displays a fullscreen semi-transparent dark background which steals all tap/touch events.</summary>
		private System.Windows.Controls.Border fFullscreenBackground;

		/// <summary>Displays the title text in the message box.</summary>
		private System.Windows.Controls.TextBlock fTitleTextBlock;

		/// <summary>Rectangle control which draws a dividing line between the title and message text blocks.</summary>
		private System.Windows.Shapes.Rectangle fTitleDividingLine;

		/// <summary>Displays the main message text in the message box.</summary>
		private System.Windows.Controls.TextBlock fMessageTextBlock;

		/// <summary>Scrollable container used to host the "fMessageTextBlock" control.</summary>
		private System.Windows.Controls.ScrollViewer fMessageScrollViewer;

		/// <summary>Rectangle control which draws a dividing line between te message text and the buttons panel.</summary>
		private System.Windows.Shapes.Rectangle fButtonPanelDividingLine;

		/// <summary>Layout container for all button controls.</summary>
		private System.Windows.Controls.StackPanel fButtonStackPanel;

		/// <summary>Scrollable container used to host the "fButtonStackPanel" control.</summary>
		private System.Windows.Controls.ScrollViewer fButtonScrollViewer;

		/// <summary>
		///  Static collection of all message boxes currently shown, where the first message box in the collection
		///  is at the bottom of the z-order and the one in the collection is at the top of the z-order.
		/// </summary>
		private static System.Collections.Generic.List<CoronaMessageBox> sShownMessageBoxes;

		/// <summary>Read-only collection which wraps the mutable "sShownMessageBoxes" collection.</summary>
		private static System.Collections.ObjectModel.ReadOnlyCollection<CoronaMessageBox> sReadOnlyShownMessageBoxes;

		#endregion


		#region Events
		/// <summary>
		///  Raised when the message box has been closed by the end-user via a displayed button, back key,
		///  or if one of the Close() methods have been called.
		/// </summary>
		public event EventHandler<CoronaLabs.WinRT.EmptyEventArgs> Closed;

		#endregion


		#region Constructors
		/// <summary>Initializes all static member variables.</summary>
		static CoronaMessageBox()
		{
			sShownMessageBoxes = new System.Collections.Generic.List<CoronaMessageBox>();
			sReadOnlyShownMessageBoxes =
					new System.Collections.ObjectModel.ReadOnlyCollection<CoronaMessageBox>(sShownMessageBoxes);
		}

		/// <summary>
		///  <para>Creates a new message box initialized with empty text and no buttons.</para>
		///  <para>
		///   You are expected to set the returned message box's <see cref="Title"/>, <see cref="Message"/>,
		///   and <see cref="ButtonLabels"/> properties before showing it.
		///  </para>
		/// </summary>
		public CoronaMessageBox()
		{
			// Set the root frame to null until the message box is shown.
			fRootFrame = null;

			// Create the message box's settings, initialized to its defaults.
			fSettings = new Corona.WinRT.Interop.UI.MessageBoxSettings();

			// Initialize last button pressed index to an invalid index.
			fLastButtonIndexPressed = -1;

			// Create the popup control.
			fPopup = new System.Windows.Controls.Primitives.Popup();

			// Fetch the system UI's default settings needed by this message box.
			var application = System.Windows.Application.Current;
			object resourceValue = application.Resources["PhoneFontSizeLarge"];
			double defaultTitleFontSize = (resourceValue is double) ? (double)resourceValue : 32.0;
			resourceValue = application.Resources["PhoneFontSizeMedium"];
			double defaultContentFontSize = (resourceValue is double) ? (double)resourceValue : 22.667;
			var defaultMessageBoxColor = System.Windows.Media.Color.FromArgb(255, 32, 32, 32);
			resourceValue = application.Resources["PhoneChromeColor"];
			if (resourceValue is System.Windows.Media.Color)
			{
				defaultMessageBoxColor = (System.Windows.Media.Color)resourceValue;
			}
			var defaultFontColor = System.Windows.Media.Colors.White;
			resourceValue = application.Resources["PhoneForegroundColor"];
			if (resourceValue is System.Windows.Media.Color)
			{
				defaultFontColor = (System.Windows.Media.Color)resourceValue;
			}

			// Create a dark fullscreen transparent background.
			// This allows the end user to see what's behind the message box and also steals all touch/tap events.
			fFullscreenBackground = new System.Windows.Controls.Border();
			fFullscreenBackground.Background =
					new System.Windows.Media.SolidColorBrush(System.Windows.Media.Color.FromArgb(128, 0, 0, 0));
			fFullscreenBackground.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
			fFullscreenBackground.VerticalAlignment = System.Windows.VerticalAlignment.Center;
			fPopup.Child = fFullscreenBackground;

			// Create a border panel that will host all of the content within the message box window.
			var messageBoxBorder = new System.Windows.Controls.Border();
			messageBoxBorder.BorderBrush = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Black);
			messageBoxBorder.BorderThickness = new System.Windows.Thickness(2);
			messageBoxBorder.VerticalAlignment = System.Windows.VerticalAlignment.Center;
			messageBoxBorder.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
			fFullscreenBackground.Child = messageBoxBorder;

			// Create the main message box stack panel used to display the title, message, buttons, and background.
			var mainStackPanel = new System.Windows.Controls.StackPanel();
			mainStackPanel.Background = new System.Windows.Media.SolidColorBrush(defaultMessageBoxColor);
			mainStackPanel.Orientation = System.Windows.Controls.Orientation.Vertical;
			messageBoxBorder.Child = mainStackPanel;

			// Create the title text block.
			fTitleTextBlock = new System.Windows.Controls.TextBlock();
			fTitleTextBlock.TextWrapping = System.Windows.TextWrapping.NoWrap;
			fTitleTextBlock.TextTrimming = System.Windows.TextTrimming.WordEllipsis;
			fTitleTextBlock.FontSize = defaultTitleFontSize;
			fTitleTextBlock.FontWeight = System.Windows.FontWeights.SemiBold;
			fTitleTextBlock.Margin = new System.Windows.Thickness(20, 20, 20, 10);
			fTitleTextBlock.Foreground = new System.Windows.Media.SolidColorBrush(defaultFontColor);
			mainStackPanel.Children.Add(fTitleTextBlock);

			// Create the dividing line control used to separate the title and message text blocks.
			fTitleDividingLine = new System.Windows.Shapes.Rectangle();
			fTitleDividingLine.Height = 1;
			fTitleDividingLine.Margin = new System.Windows.Thickness(0);
			fTitleDividingLine.Fill = messageBoxBorder.BorderBrush;
			mainStackPanel.Children.Add(fTitleDividingLine);

			// Create the message scroll view container.
			// Note: This container will only be scrollable if the message text is too big to fit onscreen.
			fMessageScrollViewer = new System.Windows.Controls.ScrollViewer();
			fMessageScrollViewer.Padding = new System.Windows.Thickness(20, 20, 20, 20);
			fMessageScrollViewer.HorizontalScrollBarVisibility = System.Windows.Controls.ScrollBarVisibility.Disabled;
			mainStackPanel.Children.Add(fMessageScrollViewer);

			// Create the message text block.
			fMessageTextBlock = new System.Windows.Controls.TextBlock();
			fMessageTextBlock.TextWrapping = System.Windows.TextWrapping.Wrap;
			fMessageTextBlock.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;
			fMessageTextBlock.FontSize = defaultContentFontSize;
			fMessageTextBlock.Foreground = new System.Windows.Media.SolidColorBrush(defaultFontColor);
			fMessageTextBlock.Margin = new System.Windows.Thickness(0);
			fMessageTextBlock.Padding = new System.Windows.Thickness(0);
			fMessageScrollViewer.Content = fMessageTextBlock;

			// Create the dividing line control used to separate the message text block and the buttons container.
			fButtonPanelDividingLine = new System.Windows.Shapes.Rectangle();
			fButtonPanelDividingLine.Height = 1;
			fButtonPanelDividingLine.Margin = new System.Windows.Thickness(0);
			fButtonPanelDividingLine.Fill = messageBoxBorder.BorderBrush;
			mainStackPanel.Children.Add(fButtonPanelDividingLine);

			// Create the button scroller and panel containers.
			fButtonScrollViewer = new System.Windows.Controls.ScrollViewer();
			fButtonScrollViewer.Padding = new System.Windows.Thickness(10, 10, 10, 10);
			fButtonScrollViewer.HorizontalScrollBarVisibility = System.Windows.Controls.ScrollBarVisibility.Disabled;
			mainStackPanel.Children.Add(fButtonScrollViewer);
			fButtonStackPanel = new System.Windows.Controls.StackPanel();
			fButtonStackPanel.Orientation = System.Windows.Controls.Orientation.Horizontal;
			fButtonScrollViewer.Content = fButtonStackPanel;
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets or sets the message box's optional title text.</summary>
		/// <remarks>
		///  You are expected to set this property before calling the <see cref="Show()"/> method.
		///  Any changes made to this property while the message box is shown will not be displayed until
		///  the next time the message box is shown.
		/// </remarks>
		/// <value>
		///  <para>The title text to be displayed at the top of the message box.</para>
		///  <para>Set to empty string to not display a title in the message box.</para>
		/// </value>
		public string Title
		{
			get
			{
				return fSettings.Title;
			}
			set
			{
				if (value == null)
				{
					value = string.Empty;
				}
				fSettings.Title = value;
			}
		}

		/// <summary>Gets or sets the main text to be displayed in the message box.</summary>
		/// <remarks>
		///  You are expected to set this property before calling the <see cref="Show()"/> method.
		///  Any changes made to this property while the message box is shown will not be displayed until
		///  the next time the message box is shown.
		/// </remarks>
		/// <value>
		///  <para>The main text to be displayed within the message box.</para>
		///  <para>Set to empty string to not display text within the message box.</para>
		/// </value>
		public string Message
		{
			get
			{
				return fSettings.Message;
			}
			set
			{
				if (value == null)
				{
					value = string.Empty;
				}
				fSettings.Message = value;
			}
		}

		/// <summary>Gets a modifiable collection of labels to be displayed as buttons in the message box.</summary>
		/// <remarks>
		///  You are expected to set this property before calling the <see cref="Show()"/> method.
		///  Any changes made to this property while the message box is shown will not be displayed until
		///  the next time the message box is shown.
		/// </remarks>
		/// <value>
		///  <para>Modifiable collection of button labels to be displayed as button in the message box.</para>
		///  <para>
		///   Can be empty, which means that the message box will not display any buttons and can only be closed
		///   by the end-user via the back key.
		///  </para>
		/// </value>
		public System.Collections.Generic.IList<string> ButtonLabels
		{
			get { return fSettings.ButtonLabels; }
		}

		/// <summary>Determines if the message box is currently being shown to the end-user.</summary>
		/// <value>
		///  <para>
		///   Set true if the message box is currently being shown via the <see cref="Show()"/> method.
		///   Note that this property will still be true if the message box is obscurred by any UI displayed on top of it.
		///  </para>
		///  <para>Set false if the message box has been closed or has not been shown yet.</para>
		/// </value>
		public bool IsShowing
		{
			get { return fPopup.IsOpen; }
		}

		/// <summary>Determines if the message box was closed via one of its displayed button.</summary>
		/// <value>
		///  <para>
		///   Set true if the message box was closed via a button press, in which case, property
		///   <see cref="PressedButtonIndex"/> will indicate which button was pressed. Note that this will be true
		///   if you call the <see cref="CloseWithButtonIndex"/> method as well.
		///  </para>
		///  <para>
		///   Set false if the message box was closed via the back key, if a Close() method was called,
		///   or if the message box hasn't been shown yet.
		///  </para>
		/// </value>
		public bool WasButtonPressed
		{
			get { return (fLastButtonIndexPressed >= 0); }
		}

		/// <summary>
		///  <para>Indicates which button was pressed to close the message box.</para>
		///  <para>Note that this property is only applicable if the <see cref="WasButtonPressed"/> property is set true.</para>
		/// </summary>
		/// <value>
		///  <para>
		///   Zero based index of the button that was pressed to close the message box.
		///   This index matches a button label in the <see cref="ButtonLabels"/> collection.
		///  </para>
		///  <para>
		///   Set to -1 if a button was not pressed to close the message box, which is indicated by the
		///   <see cref="WasButtonPressed"/> property. In this case, the message box was closed via the back key
		///   or by one of the Close() methods.
		///  </para>
		/// </value>
		public int PressedButtonIndex
		{
			get { return fLastButtonIndexPressed; }
		}

		/// <summary>Displays the message box using the assigned title, message, and button labels.</summary>
		/// <remarks>This method will do nothing if the message box is currently being shown.</remarks>
		public void Show()
		{
			// Do not continue if already shown.
			if (this.IsShowing)
			{
				return;
			}

			// Update the message box's title.
			fTitleTextBlock.Text = fSettings.Title.Trim();
			if (string.IsNullOrEmpty(fTitleTextBlock.Text))
			{
				fTitleTextBlock.Visibility = System.Windows.Visibility.Collapsed;
			}
			else
			{
				fTitleTextBlock.Visibility = System.Windows.Visibility.Visible;
			}

			// Update the message box's buttons.
			fButtonStackPanel.Children.Clear();
			for (int index = 0; index < fSettings.ButtonLabels.Count; index++)
			{
				string buttonLabel = fSettings.ButtonLabels[index];
				var textBlock = new System.Windows.Controls.TextBlock();
				textBlock.Text = (buttonLabel != null) ? buttonLabel : string.Empty;
				textBlock.TextTrimming = System.Windows.TextTrimming.WordEllipsis;

				var button = new System.Windows.Controls.Button();
				button.Content = textBlock;
				button.Tag = index;
				button.Click += OnButtonClicked;
				fButtonStackPanel.Children.Add(button);
			}
			if (fButtonStackPanel.Children.Count > 0)
			{
				fButtonScrollViewer.Visibility = System.Windows.Visibility.Visible;
			}
			else
			{
				fButtonScrollViewer.Visibility = System.Windows.Visibility.Collapsed;
			}

			// Update the message box's main message text.
			fMessageTextBlock.Text = fSettings.Message.Trim();
			if (string.IsNullOrEmpty(fMessageTextBlock.Text))
			{
				fMessageScrollViewer.Visibility = System.Windows.Visibility.Collapsed;
			}
			else
			{
				fMessageScrollViewer.Visibility = System.Windows.Visibility.Visible;
			}

			// Update the message box layout to fit the screen and application orientation.
			UpdateLayout();

			// Add event handlers.
			fPopup.LayoutUpdated += OnLayoutUpdated;
			fPopup.Opened += OnOpened;
			fPopup.Closed += OnClosed;

			// Display the message box.
			fPopup.IsOpen = true;
		}

		/// <summary>
		///  <para>Closes the message box, if currently shown.</para>
		///  <para>The <see cref="WasButtonPressed"/> property will be set to false in this case.</para>
		/// </summary>
		public void Close()
		{
			// Do not continue if not open.
			if (this.IsShowing == false)
			{
				return;
			}

			// Flag that a button did not close this message box by setting the button index to an invalid value.
			fLastButtonIndexPressed = -1;

			// Close the message box.
			fPopup.IsOpen = false;
		}

		/// <summary>Closes the message box, if currently shown, by simulating a button press.</summary>
		/// <param name="index">Zero based index of a button label in the <see cref="ButtonLabels"/> collection.</param>
		/// <returns>
		///  <para>Returns true if successfully closed the message box with the given button index.</para>
		///  <para>Returns false if the message box is not currently shown or if given an invalid index.</para>
		/// </returns>
		public bool CloseWithButtonIndex(int index)
		{
			// Do not continue if not open.
			if (this.IsShowing == false)
			{
				return false;
			}

			// Do not continue if the given index is out of bounds.
			if ((index < 0) || (index >= fButtonStackPanel.Children.Count))
			{
				return false;
			}

			// Store the index of the button.
			fLastButtonIndexPressed = index;

			// Close the message box.
			fPopup.IsOpen = false;

			// Return true to indicate that the message box was successfully closed.
			return true;
		}

		#endregion


		#region Private Methods
		/// <summary>
		///  <para>
		///   Updates the layout of the message box's title text, message text, and buttons to best fit its content.
		///  </para>
		///  <para>
		///   This method should be called whenever the page layout changes onscreen as well,
		///   such as after an orientation change or a resize.
		///  </para>
		/// </summary>
		private void UpdateLayout()
		{
			// Fetch the application object.
			var application = System.Windows.Application.Current;
			if (application == null)
			{
				return;
			}

			// Fetch the main message box container.
			var messageBox = fFullscreenBackground.Child as System.Windows.FrameworkElement;
			if (messageBox == null)
			{
				return;
			}

			// Check if the application page under this message box has changed.
			var rootFrame = application.RootVisual as Microsoft.Phone.Controls.PhoneApplicationFrame;
			if ((rootFrame != null) && (rootFrame != fRootFrame))
			{
				// Remove event handlers from the previous page.
				if (fRootFrame != null)
				{
					fRootFrame.BackKeyPress -= OnBackKeyPressed;
				}

				// Keep a reference to the current page and listen for its events.
				fRootFrame = rootFrame;
				fRootFrame.BackKeyPress += OnBackKeyPressed;
			}

			// Rotate the popup if the application's current page is landscape.
			bool isPagePortrait = true;
			if (fRootFrame != null)
			{
				switch (fRootFrame.Orientation)
				{
					case Microsoft.Phone.Controls.PageOrientation.Landscape:
					case Microsoft.Phone.Controls.PageOrientation.LandscapeLeft:
					case Microsoft.Phone.Controls.PageOrientation.LandscapeRight:
						isPagePortrait = false;
						break;
				}
				fFullscreenBackground.RenderTransform = fRootFrame.RenderTransform;
			}
			else
			{
				fFullscreenBackground.RenderTransform = null;
			}

			// Resize the semi-transparent background to fill the screen.
			// Note: XAML popups are displayed relative to portrait. So, we must swap width/height when app is landscape.
			if (isPagePortrait)
			{
				fFullscreenBackground.Width = application.Host.Content.ActualWidth;
				fFullscreenBackground.Height = application.Host.Content.ActualHeight;
			}
			else
			{
				fFullscreenBackground.Width = application.Host.Content.ActualHeight;
				fFullscreenBackground.Height = application.Host.Content.ActualWidth;
			}

			// Resize the message box to best fit the current page orientation.
			if (isPagePortrait)
			{
				messageBox.Width = fFullscreenBackground.Width - 40;
			}
			else
			{
				messageBox.Width = fFullscreenBackground.Width * 0.8;
			}
			messageBox.MaxHeight = fFullscreenBackground.Height - 40;

			// Layout the buttons to best fit the message box
			if (fButtonStackPanel.Children.Count > 0)
			{
				if (isPagePortrait && (fButtonStackPanel.Children.Count <= 2))
				{
					fButtonStackPanel.Orientation = System.Windows.Controls.Orientation.Horizontal;
				}
				else if (!isPagePortrait && (fButtonStackPanel.Children.Count <= 3))
				{
					fButtonStackPanel.Orientation = System.Windows.Controls.Orientation.Horizontal;
				}
				else
				{
					fButtonStackPanel.Orientation = System.Windows.Controls.Orientation.Vertical;
				}
				double minButtonWidth;
				double maxButtonWidth;
				if (fButtonStackPanel.Orientation == System.Windows.Controls.Orientation.Horizontal)
				{
					minButtonWidth = fButtonScrollViewer.ActualWidth;
					minButtonWidth -= fButtonScrollViewer.Padding.Left + fButtonScrollViewer.Padding.Right;
					if (fButtonStackPanel.Children.Count >= 3)
					{
						minButtonWidth /= (double)fButtonStackPanel.Children.Count;
					}
					else
					{
						minButtonWidth /= 2.0;
					}
					if (minButtonWidth < 0)
					{
						minButtonWidth = 0;
					}
					maxButtonWidth = minButtonWidth;
					if (fButtonStackPanel.Children.Count == 1)
					{
						maxButtonWidth = fButtonScrollViewer.ActualWidth;
					}
					fButtonStackPanel.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
				}
				else
				{
					minButtonWidth = 0;
					maxButtonWidth = double.PositiveInfinity;
					fButtonStackPanel.HorizontalAlignment = System.Windows.HorizontalAlignment.Stretch;
				}
				foreach (var uiElement in fButtonStackPanel.Children)
				{
					var frameworkElement = uiElement as System.Windows.FrameworkElement;
					if (frameworkElement != null)
					{
						frameworkElement.MinWidth = minButtonWidth;
						frameworkElement.MaxWidth = maxButtonWidth;
					}
				}
			}

			// Resize the message text and button panels to best fill the message box.
			if (messageBox.ActualHeight > 0)
			{
				// Set the size of the button panel first.
				if (fButtonStackPanel.Children.Count > 0)
				{
					// Calculate the height of a button, including its padding.
					double buttonHeight;
					if (fButtonStackPanel.Orientation == System.Windows.Controls.Orientation.Horizontal)
					{
						buttonHeight = fButtonStackPanel.ActualHeight;
					}
					else
					{
						buttonHeight = fButtonStackPanel.ActualHeight / fButtonStackPanel.Children.Count;
					}

					// Calculate what the minimum height of the button panel must be.
					double minButtonPanelHeight;
					if (fButtonStackPanel.Orientation == System.Windows.Controls.Orientation.Horizontal)
					{
						minButtonPanelHeight = fButtonStackPanel.ActualHeight;
					}
					else if (isPagePortrait && (fButtonStackPanel.Children.Count > 3))
					{
						minButtonPanelHeight = buttonHeight * 3.5;
					}
					else if (!isPagePortrait && (fButtonStackPanel.Children.Count > 2))
					{
						minButtonPanelHeight = buttonHeight * 2.5;
					}
					else
					{
						minButtonPanelHeight = fButtonStackPanel.ActualHeight;
					}
					minButtonPanelHeight += fButtonScrollViewer.Margin.Top;
					minButtonPanelHeight += fButtonScrollViewer.Padding.Top;
					minButtonPanelHeight += fButtonScrollViewer.Padding.Bottom;
					minButtonPanelHeight += fButtonScrollViewer.Margin.Bottom;
					
					// Calculate the current height of all elements in the message box, excluding the button panel.
					double heightsExcludingButtons = 0;
					if (fTitleTextBlock.Visibility != System.Windows.Visibility.Collapsed)
					{
						heightsExcludingButtons += fTitleTextBlock.Margin.Top;
						heightsExcludingButtons += fTitleTextBlock.ActualHeight;
						heightsExcludingButtons += fTitleTextBlock.Margin.Bottom;
					}
					if (fTitleDividingLine.Visibility != System.Windows.Visibility.Collapsed)
					{
						heightsExcludingButtons += fTitleDividingLine.Margin.Top;
						heightsExcludingButtons += fTitleDividingLine.ActualHeight;
						heightsExcludingButtons += fTitleDividingLine.Margin.Bottom;
					}
					if (fMessageScrollViewer.Visibility != System.Windows.Visibility.Collapsed)
					{
						heightsExcludingButtons += fMessageScrollViewer.Margin.Top;
						if (fMessageTextBlock.Visibility != System.Windows.Visibility.Collapsed)
						{
							heightsExcludingButtons += fMessageScrollViewer.Padding.Top;
							heightsExcludingButtons += fMessageTextBlock.ActualHeight;
							heightsExcludingButtons += fMessageScrollViewer.Padding.Bottom;
						}
						heightsExcludingButtons += fMessageScrollViewer.Margin.Bottom;
					}
					if (fButtonPanelDividingLine.Visibility != System.Windows.Visibility.Collapsed)
					{
						heightsExcludingButtons += fButtonPanelDividingLine.Margin.Top;
						heightsExcludingButtons += fButtonPanelDividingLine.ActualHeight;
						heightsExcludingButtons += fButtonPanelDividingLine.Margin.Bottom;
					}
					heightsExcludingButtons += fButtonScrollViewer.Margin.Top;
					heightsExcludingButtons += fButtonScrollViewer.Padding.Top;
					heightsExcludingButtons += fButtonScrollViewer.Padding.Bottom;
					heightsExcludingButtons += fButtonScrollViewer.Margin.Bottom;

					// Calculate what the maximum height of the button panel can be to best fit all of its buttons.
					// Note that this prioritizes the title and message text over button, but there still must be
					// a minimum button height so that the end user can still tap them.
					double maxButtonPanelHeight = minButtonPanelHeight;
					if ((heightsExcludingButtons + minButtonPanelHeight) < messageBox.MaxHeight)
					{
						// This is the max button panel height.
						maxButtonPanelHeight = messageBox.MaxHeight - heightsExcludingButtons;

						// Now we shave the button panel height down so that half a button appears in its scroll view.
						// This makes it more obvious to the end user that there are more button that they can scroll to.
						double unpaddedButtonPanelHeight = maxButtonPanelHeight;
						unpaddedButtonPanelHeight -= fButtonScrollViewer.Margin.Top;
						unpaddedButtonPanelHeight -= fButtonScrollViewer.Padding.Top;
						unpaddedButtonPanelHeight -= fButtonScrollViewer.Padding.Bottom;
						unpaddedButtonPanelHeight -= fButtonScrollViewer.Margin.Bottom;
						if (unpaddedButtonPanelHeight < fButtonStackPanel.ActualHeight)
						{
							double delta = unpaddedButtonPanelHeight % buttonHeight;
							if (delta > (buttonHeight * 0.75))
							{
								maxButtonPanelHeight -= delta;
								maxButtonPanelHeight += buttonHeight / 2.0;
							}
							else if (delta < (buttonHeight * 0.25))
							{
								maxButtonPanelHeight -= delta;
								maxButtonPanelHeight -= buttonHeight / 2.0;
							}
						}
					}

					// Apply a maximum button panel height constraint.
					fButtonScrollViewer.MaxHeight = maxButtonPanelHeight;
				}
				else
				{
					// There are no buttons in the message box. Remove the maximum height constraint.
					fButtonStackPanel.MaxHeight = double.PositiveInfinity;
				}

				// Set the height of the main message text panel.
				// This must be done after setting the button panel's height.
				if ((fMessageScrollViewer.Visibility != System.Windows.Visibility.Collapsed) &&
				    (fMessageTextBlock.Visibility != System.Windows.Visibility.Collapsed))
				{
					// Calculate the height of all elements in the message box, excluding the message text panel.
					double heightsExcludingMessage = 0;
					if (fTitleTextBlock.Visibility != System.Windows.Visibility.Collapsed)
					{
						heightsExcludingMessage += fTitleTextBlock.Margin.Top;
						heightsExcludingMessage += fTitleTextBlock.ActualHeight;
						heightsExcludingMessage += fTitleTextBlock.Margin.Bottom;
					}
					if (fTitleDividingLine.Visibility != System.Windows.Visibility.Collapsed)
					{
						heightsExcludingMessage += fTitleDividingLine.Margin.Top;
						heightsExcludingMessage += fTitleDividingLine.ActualHeight;
						heightsExcludingMessage += fTitleDividingLine.Margin.Bottom;
					}
					if (fButtonPanelDividingLine.Visibility != System.Windows.Visibility.Collapsed)
					{
						heightsExcludingMessage += fButtonPanelDividingLine.Margin.Top;
						heightsExcludingMessage += fButtonPanelDividingLine.ActualHeight;
						heightsExcludingMessage += fButtonPanelDividingLine.Margin.Bottom;
					}
					if (fButtonScrollViewer.Visibility != System.Windows.Visibility.Collapsed)
					{
						if (fButtonScrollViewer.MaxHeight != double.PositiveInfinity)
						{
							heightsExcludingMessage += fButtonScrollViewer.MaxHeight;
						}
						else
						{
							heightsExcludingMessage += fButtonScrollViewer.Margin.Top;
							heightsExcludingMessage += fButtonScrollViewer.Margin.Bottom;
						}
					}

					// Apply a maximum text panel height constraint.
					fMessageScrollViewer.MaxHeight = messageBox.MaxHeight - heightsExcludingMessage;
				}
				else
				{
					// There is no message text. Remove the maximum height constraint.
					fMessageScrollViewer.MaxHeight = double.PositiveInfinity;
				}
			}
			else
			{
				// The heights for all controls have not been calculated by the framework yet.
				// So, set the message panel's height to a default value until the XAML frameworks calls this method again.
				if (isPagePortrait || (fButtonStackPanel.Children.Count <= 0))
				{
					fMessageScrollViewer.MaxHeight = fFullscreenBackground.Height / 2;
				}
				else
				{
					fMessageScrollViewer.MaxHeight = fFullscreenBackground.Height / 3;
				}
			}

			// Enable/disable message text scrolling.
			double paddingHeight = fMessageScrollViewer.Padding.Top + fMessageScrollViewer.Padding.Bottom;
			if ((fMessageTextBlock.ActualHeight + paddingHeight) > fMessageScrollViewer.MaxHeight)
			{
				var visibilityMode = System.Windows.Controls.ScrollBarVisibility.Visible;
				if (fMessageScrollViewer.VerticalScrollBarVisibility != visibilityMode)
				{
					fMessageScrollViewer.VerticalScrollBarVisibility = visibilityMode;
					fMessageScrollViewer.Background =
							new System.Windows.Media.SolidColorBrush(System.Windows.Media.Color.FromArgb(255, 64, 64, 64));
				}
			}
			else
			{
				var visibilityMode = System.Windows.Controls.ScrollBarVisibility.Disabled;
				if (fMessageScrollViewer.VerticalScrollBarVisibility != visibilityMode)
				{
					fMessageScrollViewer.VerticalScrollBarVisibility = visibilityMode;
					fMessageScrollViewer.Background =
							new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent);
				}
			}

			// Enable/disable button scrolling.
			paddingHeight = fButtonScrollViewer.Padding.Top + fButtonScrollViewer.Padding.Bottom;
			if ((fButtonStackPanel.Orientation == System.Windows.Controls.Orientation.Horizontal) ||
			    ((fButtonStackPanel.ActualHeight + paddingHeight) <= fButtonScrollViewer.ActualHeight))
			{
				var visibilityMode = System.Windows.Controls.ScrollBarVisibility.Disabled;
				if (fButtonScrollViewer.VerticalScrollBarVisibility != visibilityMode)
				{
					fButtonScrollViewer.VerticalScrollBarVisibility = visibilityMode;
					fButtonScrollViewer.Background =
							new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent);
				}
			}
			else
			{
				var visibilityMode = System.Windows.Controls.ScrollBarVisibility.Visible;
				if (fButtonScrollViewer.VerticalScrollBarVisibility != visibilityMode)
				{
					fButtonScrollViewer.VerticalScrollBarVisibility = visibilityMode;
					fButtonScrollViewer.Background =
							new System.Windows.Media.SolidColorBrush(System.Windows.Media.Color.FromArgb(255, 64, 64, 64));
				}
			}

			// Show a dividing line under the title if:
			// 1) The message text block is set up to scroll.
			// 2) There is no message text and the button panel is set up to scroll.
			bool showTitleDividingLine = false;
			if (fTitleTextBlock.Visibility != System.Windows.Visibility.Collapsed)
			{
				if ((fMessageScrollViewer.Visibility != System.Windows.Visibility.Collapsed) &&
				    (fMessageScrollViewer.VerticalScrollBarVisibility == System.Windows.Controls.ScrollBarVisibility.Visible))
				{
					showTitleDividingLine = true;
				}
				else if ((fMessageScrollViewer.Visibility == System.Windows.Visibility.Collapsed) &&
				         (fButtonScrollViewer.Visibility != System.Windows.Visibility.Collapsed) &&
				         (fButtonScrollViewer.VerticalScrollBarVisibility == System.Windows.Controls.ScrollBarVisibility.Visible))
				{
					showTitleDividingLine = true;
				}
			}
			fTitleDividingLine.Opacity = showTitleDividingLine ? 1.0 : 0;

			// Show a dividing line above the button panel if:
			// 1) The message text block is set up to scroll.
			// 2) The button panel is set up to scroll.
			bool showButtonPanelDividingLine = false;
			if (fButtonScrollViewer.Visibility != System.Windows.Visibility.Collapsed)
			{
				if ((fMessageScrollViewer.Visibility != System.Windows.Visibility.Collapsed) &&
				    (fMessageScrollViewer.VerticalScrollBarVisibility == System.Windows.Controls.ScrollBarVisibility.Visible))
				{
					showButtonPanelDividingLine = true;
				}
				else if (fButtonScrollViewer.VerticalScrollBarVisibility == System.Windows.Controls.ScrollBarVisibility.Visible)
				{
					if ((fTitleTextBlock.Visibility != System.Windows.Visibility.Collapsed) ||
					    (fMessageScrollViewer.Visibility != System.Windows.Visibility.Collapsed))
					{
						showButtonPanelDividingLine = true;
					}
				}
			}
			fButtonPanelDividingLine.Opacity = showButtonPanelDividingLine ? 1.0 : 0;
		}

		/// <summary>Called when the message box's XAML popup has been just been displayed onscreen.</summary>
		/// <param name="sender">The popup that raised this event.</param>
		/// <param name="e">Empty event arguments.</param>
		private void OnOpened(object sender, EventArgs e)
		{
			// Track that this message box is currently being shown by adding it to the following static collection.
			// This collection tracks the z-order of all displayed message boxes, where the first element is on top.
			// This is needed to handle the back key event correctly, where it is only applied to the top most message box.
			sShownMessageBoxes.Insert(0, this);
		}

		/// <summary>Called when the message box's XAML popup has just been closed and is no longer on screen.</summary>
		/// <param name="sender">The popup that raised this event.</param>
		/// <param name="e">Empty event arguments.</param>
		private void OnClosed(object sender, EventArgs e)
		{
			// Remove event handlers.
			fPopup.LayoutUpdated -= OnLayoutUpdated;
			fPopup.Opened -= OnOpened;
			fPopup.Closed -= OnClosed;
			if (fRootFrame != null)
			{
				fRootFrame.BackKeyPress -= OnBackKeyPressed;
			}

			// Remove this message box instance from the global "shown" message box collection.
			// Note: This collection tracks the z-order of all currently displayed message boxes.
			sShownMessageBoxes.Remove(this);

			// Drop the reference to the page that is behind this message box. This allows it to be garbage collected.
			fRootFrame = null;

			// Notify the owner that this message box has been closed.
			if (this.Closed != null)
			{
				this.Closed.Invoke(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
			}
		}

		/// <summary>Called when the popup or page layout has changed, such as after a resize or orientation change.</summary>
		/// <param name="sender">The popup that raised this event.</param>
		/// <param name="e">Empty event arguments.</param>
		private void OnLayoutUpdated(object sender, EventArgs e)
		{
			// Do not continue if the message box is not currently displayed/open.
			if (this.IsShowing == false)
			{
				return;
			}

			// Update the message box's layout to best fit the screen.
			UpdateLayout();
		}

		/// <summary>Called when one of the message box's buttons have been tapped.</summary>
		/// <param name="sender">The button control that raised this event.</param>
		/// <param name="e">Event arguments indicating which object raised this event.</param>
		private void OnButtonClicked(object sender, System.Windows.RoutedEventArgs e)
		{
			// Fetch the button that was clicked/tapped on.
			var button = sender as System.Windows.Controls.Button;
			if (button == null)
			{
				return;
			}

			// Store the index of the button that was pressed.
			fLastButtonIndexPressed = (int)button.Tag;

			// Close the message box.
			fPopup.IsOpen = false;
		}

		/// <summary>Called when the back key has been pressed for the current page.</summary>
		/// <param name="sender">The page that raised this event.</param>
		/// <param name="e">Provides a "Cancel" property, which if set true, will cancel/block navigation.</param>
		private void OnBackKeyPressed(object sender, System.ComponentModel.CancelEventArgs e)
		{
			// Do not continue if another event handler has already handled the back key.
			if (e.Cancel == true)
			{
				return;
			}

			// Ignore this event if this message box is not currently shown to the end-user.
			if (this.IsShowing == false)
			{
				return;
			}

			// Ignore this event if this message box is not at the top of the z-order.
			if ((sShownMessageBoxes.Count > 0) && (sShownMessageBoxes[0] != this))
			{
				return;
			}

			// This message box is at the top of the screen. Close it.
			Close();

			// Cancel back key navigation.
			e.Cancel = true;
		}

		#endregion


		#region Public Static Methods/Properties
		/// <summary>Gets a read-only collection of all currently shown message boxes.</summary>
		/// <remarks>
		///  The order of the message boxes in this collection indicates their z-order on screen.
		///  The first message box in the collection (ie: index 0) is at the top of the z-order
		///  and the last message box in the collection is at the bottom.
		/// </remarks>
		/// <value>
		///  <para>Read-only collection of all currently shown message boxes.</para>
		///  <para>This collection will be empty if no message boxes are currently shown.</para>
		/// </value>
		public static System.Collections.ObjectModel.ReadOnlyCollection<CoronaMessageBox> ShownMessagBoxes
		{
			get { return sReadOnlyShownMessageBoxes; }
		}

		#endregion
	}
}
