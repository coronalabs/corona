using System;
using ReceivedTouchEventTokenTable = System.Runtime.InteropServices.WindowsRuntime.EventRegistrationTokenTable
											<System.EventHandler<CoronaLabs.Corona.WinRT.Interop.Input.TouchEventArgs>>;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Input
{
	/// <summary>Provides input device events such as taps, touches, keys, etc.</summary>
	public class DotNetInputDeviceServices : Corona.WinRT.Interop.Input.IInputDeviceServices
	{
		#region Private Member Variables
		/// <summary>UI control to receive input events from.</summary>
		private System.Windows.FrameworkElement fUIEventSource;

		/// <summary>
		///  <para>Reference to the page that is hosting the "fUIElement" control.</para>
		///  <para>Needed to acquire the back key navigation event.</para>
		/// </summary>
		private UI.DotNetPageAdapter fPageAdapter;

		/// <summary>Stores event handlers for the ReceivedTouch event, which this object handles specially.</summary>
		private ReceivedTouchEventTokenTable fReceivedTouchEventTokenTable;

		/// <summary>Set true if the this object has added an event handler to the static "FrameReported" event.</summary>
		private bool fHasAddedFrameReportedEventHandler;

		/// <summary>
		///  <para>Set true if the ReceivdTouch event should provide touch events for all pointers/fingers on the screen.</para>
		///  <para>Set false if the ReceivedTouch event should only provide touch events for one pointer/finger.</para>
		/// </summary>
		private bool fMultitouchEnabled;

		/// <summary>
		///  <para>Collection of last received touch points that were within the "fUIEventSource" control's bounds.</para>
		///  <para>This is a dictionary using the touch point ID as the key and the TouchPoint object as the value.</para>
		///  <para>The touch point entry should be removed once the touch event has ended.</para>
		/// </summary>
		private System.Collections.Generic.Dictionary<int, System.Windows.Input.TouchPoint> fLastHandledTouchPoints;

		#endregion


		#region Events
		/// <summary>Raised when a key/button has been pressed down.</summary>
		public event EventHandler<Corona.WinRT.Interop.Input.KeyEventArgs> ReceivedKeyDown;

		/// <summary>Raised when a key/button has been released.</summary>
		public event EventHandler<Corona.WinRT.Interop.Input.KeyEventArgs> ReceivedKeyUp;

		/// <summary>Raised when the Corona surface control has been tapped on.</summary>
		public event EventHandler<Corona.WinRT.Interop.Input.TapEventArgs> ReceivedTap;

		/// <summary>Raised when the Corona surface has received a touch began, moved, ended, or canceled event.</summary>
		public event EventHandler<Corona.WinRT.Interop.Input.TouchEventArgs> ReceivedTouch
		{
			add
			{
				// Add the given event handler to the event table.
				var eventTokenTable = ReceivedTouchEventTokenTable.GetOrCreateEventRegistrationTokenTable(
											ref fReceivedTouchEventTokenTable);
				var token = eventTokenTable.AddEventHandler(value);

				// Subscribe to the system's touch events, if not done already.
				AddFrameReportedEventHandler();

				// Return the event token to the caller.
				return token;
			}
			remove
			{
				// Remove the given event handler from the event table.
				var eventTokenTable = ReceivedTouchEventTokenTable.GetOrCreateEventRegistrationTokenTable(
											ref fReceivedTouchEventTokenTable);
				eventTokenTable.RemoveEventHandler(value);

				// Remove our touch event handler from the system if this object no longer has any event handlers.
				// Note: This is important because the "FrameReported" event is static and we don't want to leave
				//       this object's reference in its list, preventing this object from being garbage collected.
				if (eventTokenTable.InvocationList == null)
				{
					RemoveFrameReportedEventHandler();
				}
			}
		}

		#endregion


		#region Constructors/Destructors
		/// <summary>
		///  <para>Creates a new input event provider.</para>
		///  <para>This object will not provide tap, touch, or key events until a UIElement reference has been given to it.</para>
		/// </summary>
		public DotNetInputDeviceServices()
		{
			// Initialize member variables.
			fUIEventSource = null;
			fPageAdapter = new UI.DotNetPageAdapter();
			fHasAddedFrameReportedEventHandler = false;
			fMultitouchEnabled = false;
			fReceivedTouchEventTokenTable = null;
			fLastHandledTouchPoints = new System.Collections.Generic.Dictionary<int, System.Windows.Input.TouchPoint>();

			// Subscribe to the page's back navigation event.
			// Will be convert to a "back" key event that is compatible with Corona.
			fPageAdapter.NavigatingBack += OnPageNavigatingBack;
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>
		///  Determines if the input system will provide touch events from the primary pointer/finger
		///  or from multiple pointers/fingers.
		/// </summary>
		/// <value>
		///  <para>Set true if all pointers/fingers on the screen will provide touch events.</para>
		///  <para>
		///   Set false if touch events will only be provided from one pointer/finger. This means that the touch events
		///   generated by any additional pointers/fingers on the screen will be ignored.
		///  </para>
		/// </value>
		public bool MultitouchEnabled
		{
			get { return fMultitouchEnabled; }
			set { fMultitouchEnabled = value; }
		}

		/// <summary>Gets or sets the UI control or page to receive input events from.</summary>
		/// <value>
		///  <para>Reference to a UI control or page that this object will provide input events for.</para>
		///  <para>Set to null if this object is not listening to input events from a UI control or page.</para>
		/// </value>
		public System.Windows.FrameworkElement UIEventSource
		{
			get { return fUIEventSource; }
			set
			{
				// Do not continue if the reference won't be changing.
				if (value == fUIEventSource)
				{
					return;
				}

				// Remove event handlers from the previous UI control.
				if (fUIEventSource != null)
				{
					fUIEventSource.Loaded -= OnLoaded;
					fUIEventSource.Unloaded -= OnUnloaded;
					fUIEventSource.KeyDown -= OnKeyDown;
					fUIEventSource.KeyUp -= OnKeyUp;
					fUIEventSource.Tap -= OnTap;
					fUIEventSource.DoubleTap -= OnDoubleTap;
					RemoveFrameReportedEventHandler();
				}

				// Store the given UI control reference and fetch its parent page.
				fUIEventSource = value;

				// Fetch the page that is hosting the given UI element.
				// Note: If the given UI control is a page, then this method will return the same reference.
				fPageAdapter.Page = UI.DotNetPageServices.FetchParentPageFrom(fUIEventSource);

				// Add event handlers to the give UI control.
				if (fUIEventSource != null)
				{
					fUIEventSource.Loaded += OnLoaded;
					fUIEventSource.Unloaded += OnUnloaded;
					fUIEventSource.KeyDown += OnKeyDown;
					fUIEventSource.KeyUp += OnKeyUp;
					fUIEventSource.Tap += OnTap;
					fUIEventSource.DoubleTap += OnDoubleTap;
					AddFrameReportedEventHandler();
				}
			}
		}

		#endregion


		#region Event Handlers
		/// <summary>Called when the UI control or its page has been loaded on screen.</summary>
		/// <param name="sender">The UI control that raised this event.</param>
		/// <param name="e">Event arguments indicating which object raised this event.</param>
		private void OnLoaded(object sender, System.Windows.RoutedEventArgs e)
		{
			// Re-add our touch event handler.
			AddFrameReportedEventHandler();
		}

		/// <summary>Called when the UI control or its page has been unloaded/removed from the screen.</summary>
		/// <param name="sender">The UI control that raised this event.</param>
		/// <param name="e">Event arguments indicating which object raised this event.</param>
		private void OnUnloaded(object sender, System.Windows.RoutedEventArgs e)
		{
			// Temporarily remove our handler from the static "FrameReported" event.
			// No point in receiving events while the control or its page is no longer on screen.
			RemoveFrameReportedEventHandler();
		}

		/// <summary>Called when a key has been pressed down in the UI control this object is listening to.</summary>
		/// <param name="sender">The UI control that raised this event.</param>
		/// <param name="e">Provides information about the key that was pressed down.</param>
		private void OnKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
		{
			// Not currently supported.
		}

		/// <summary>Called when a key has been released in the UI control this object is listening to.</summary>
		/// <param name="sender">The UI control that raised this event.</param>
		/// <param name="e">Provides information about the key that was released.</param>
		private void OnKeyUp(object sender, System.Windows.Input.KeyEventArgs e)
		{
			// Not currently supported.
		}

		/// <summary>
		///  <para>Called when the back button has been pressed for the application page this object is listening to.</para>
		///  <para>Converts the back navigation event to a back key event that is compatible with Corona.</para>
		/// </summary>
		/// <param name="sender">The page that raised this event.</param>
		/// <param name="e">Provides a "Cancel" property that will prevent navigating back if set true.</param>
		private void OnPageNavigatingBack(Corona.WinRT.Interop.UI.IPage sender, CoronaLabs.WinRT.CancelEventArgs e)
		{
			// Do not continue if the given event was already handled/canceled.
			if (e.Cancel)
			{
				return;
			}

			// Create event arguments for a "back" key.
			var keyEventArgs = CreateKeyEventFor(Corona.WinRT.Interop.Input.Key.Back);

			// Raise a key down event.
			if (this.ReceivedKeyDown != null)
			{
				this.ReceivedKeyDown(this, keyEventArgs);
				if (keyEventArgs.Handled)
				{
					e.Cancel = true;
					return;
				}
			}

			// Raise a key up event.
			if (this.ReceivedKeyUp != null)
			{
				this.ReceivedKeyUp(this, keyEventArgs);
				if (keyEventArgs.Handled)
				{
					e.Cancel = true;
					return;
				}
			}
		}

		/// <summary>
		///  <para>Called when the UI control this object is listening to has been tapped on once.</para>
		///  <para>Relays the event to this object's listeners.</para>
		/// </summary>
		/// <param name="sender">The UI control that has raised this event.</param>
		/// <param name="e">Provides information about the tap event such as coordinates, timestamp, etc.</param>
		private void OnTap(object sender, System.Windows.Input.GestureEventArgs e)
		{
			int tapCount = 1;
			RaiseReceivedTapEventUsing(e, tapCount);
		}

		/// <summary>
		///  <para>Called when the UI control this object is listening to has been tapped on twice.</para>
		///  <para>Relays the event to this object's listeners.</para>
		/// </summary>
		/// <param name="sender">The UI control that has raised this event.</param>
		/// <param name="e">Provides information about the tap event such as coordinates, timestamp, etc.</param>
		private void OnDoubleTap(object sender, System.Windows.Input.GestureEventArgs e)
		{
			int tapCount = 2;
			RaiseReceivedTapEventUsing(e, tapCount);
		}

		/// <summary>
		///  <para>Called when a multitouch event has been received from the screen.</para>
		///  <para>Relays this event as a Corona compatible "touch" event.</para>
		/// </summary>
		/// <remarks>
		///  Note that the touch events received via this method are global and need to be converted to
		///  coordinates relative to the UI control given to this object.
		/// </remarks>
		/// <param name="sender">The object that raised this event.</param>
		/// <param name="e">Provides multitouch information such as number of fingers, coordinates, etc.</param>
		private void OnTouchFrameReported(object sender, System.Windows.Input.TouchFrameEventArgs e)
		{
			// Validate.
			if ((e == null) || (fUIEventSource == null))
			{
				return;
			}

			// Convert the touch event's timestamp from Ticks to DateTime.
			DateTimeOffset currentTime = DateTimeOffset.Now;
			int currentTicks = System.Environment.TickCount;
			if (currentTicks == int.MinValue)
			{
				// We can't flip the sign of an integer at it's minimum value.
				// So, add one millisecond so that we can apply the subtraction down below
				currentTicks++;
			}
			double deltaTimeInMilliseconds = (double)(currentTicks - e.Timestamp);
			DateTimeOffset timestamp = currentTime;
			if (deltaTimeInMilliseconds > 0)
			{
				timestamp = currentTime.Subtract(TimeSpan.FromMilliseconds(deltaTimeInMilliseconds));
			}

			// Relay the received touch event(s).
			if (fMultitouchEnabled)
			{
				foreach (var touchPoint in e.GetTouchPoints(fUIEventSource))
				{
					RaiseReceivedTouchEventUsing(touchPoint, timestamp);
				}
			}
			else
			{
				RaiseReceivedTouchEventUsing(e.GetPrimaryTouchPoint(fUIEventSource), timestamp);
			}
		}

		#endregion


		#region Private Methods
		/// <summary>Adds an event handler to the static "Touch.FrameReported" event, if needed and not done already.</summary>
		private void AddFrameReportedEventHandler()
		{
			// Do not continue if a UI control has not been assigned.
			if (fUIEventSource == null)
			{
				return;
			}

			// Do not continue if the UI control and its page is not currently loaded on screen.
			var parentPage = UI.DotNetPageServices.FetchParentPageFrom(fUIEventSource);
			if ((parentPage == null) || (parentPage.Parent == null))
			{
				return;
			}

			// Do not continue if there are no "ReceivedTouch" event handlers assigned to this object.
			var eventTokenTable = ReceivedTouchEventTokenTable.GetOrCreateEventRegistrationTokenTable(
										ref fReceivedTouchEventTokenTable).InvocationList;
			if (eventTokenTable == null)
			{
				return;
			}

			// Do not continue if we've already added the event handler.
			if (fHasAddedFrameReportedEventHandler)
			{
				return;
			}

			// Subscribe to the system's touch events.
			System.Windows.Input.Touch.FrameReported += OnTouchFrameReported;
			fHasAddedFrameReportedEventHandler = true;
		}

		/// <summary>Removes an event handler from the static "Touch.FrameReported" event.</summary>
		private void RemoveFrameReportedEventHandler()
		{
			if (fHasAddedFrameReportedEventHandler)
			{
				System.Windows.Input.Touch.FrameReported -= OnTouchFrameReported;
				fHasAddedFrameReportedEventHandler = false;
			}
		}

		/// <summary>Raises a <see cref="ReceivedTap"/> event for the given gesture data.</summary>
		/// <param name="e">The gesture event arguments providing tap information.</param>
		/// <param name="tapCount">Number of times the same coordinates was tapped on. Must be 1 or higher.</param>
		private void RaiseReceivedTapEventUsing(System.Windows.Input.GestureEventArgs e, int tapCount)
		{
			// Do not continue if the given event was already handled.
			if (e.Handled)
			{
				return;
			}

			// Do not continue if the "ReceivedTap" event has no subscribers.
			if (this.ReceivedTap == null)
			{
				return;
			}

			// Do not continue if a UI control has not been assigned.
			// This is needed to convert the touch coordinates to a position relative to the control.
			if (fUIEventSource == null)
			{
				return;
			}

			// Convert the tap coordinates to pixels.
			var relativePosition = e.GetPosition(fUIEventSource);
			double scaleFactor = (double)System.Windows.Application.Current.Host.Content.ScaleFactor / 100.0;
			relativePosition.X = Math.Round(relativePosition.X * scaleFactor, MidpointRounding.AwayFromZero);
			relativePosition.Y = Math.Round(relativePosition.Y * scaleFactor, MidpointRounding.AwayFromZero);

			// Create the tap event arguments.
			Corona.WinRT.Interop.Input.TouchPoint tapPoint;
			tapPoint.X = relativePosition.X;
			tapPoint.Y = relativePosition.Y;
			tapPoint.Timestamp = DateTimeOffset.Now;
			var tapEventArgs = new Corona.WinRT.Interop.Input.TapEventArgs(tapPoint, tapCount);

			// Raise the event.
			ReceivedTap(this, tapEventArgs);

			// Flag the event as handled if set.
			if (tapEventArgs.Handled)
			{
				e.Handled = true;
			}
		}

		/// <summary>Raises a <see cref="ReceivedTouch"/> event for the given touch data.</summary>
		/// <param name="point">Touch data to be passed into the event. Will be ignored if set to null.</param>
		/// <param name="timestamp">The date and time when the touch occurred.</param>
		private void RaiseReceivedTouchEventUsing(System.Windows.Input.TouchPoint point, DateTimeOffset timestamp)
		{
			// Validate.
			if (point == null)
			{
				return;
			}

			// Fetch the pointer/finger's unique integer ID.
			int pointerId = (point.TouchDevice != null) ? point.TouchDevice.Id : 0;

			// Fetch the touch phase.
			Corona.WinRT.Interop.Input.TouchPhase phase;
			switch (point.Action)
			{
				case System.Windows.Input.TouchAction.Move:
					phase = Corona.WinRT.Interop.Input.TouchPhase.Moved;
					break;
				case System.Windows.Input.TouchAction.Up:
					phase = Corona.WinRT.Interop.Input.TouchPhase.Ended;
					break;
				case System.Windows.Input.TouchAction.Down:
				default:
					phase = Corona.WinRT.Interop.Input.TouchPhase.Began;
					break;
			}

			// If a touch point began outside of the control, then ignore all of its "moved" and "ended" events.
			if ((phase != Corona.WinRT.Interop.Input.TouchPhase.Began) &&
			    (fLastHandledTouchPoints.ContainsKey(pointerId) == false))
			{
				return;
			}

			// Determine if the touch point is within the UI control's bounds.
			bool isWithinBounds = true;
			if (point.TouchDevice.DirectlyOver != fUIEventSource)
			{
				// One last special check.
				// If we're providing input event for a page, then allow touch event for a fullscreen background grid control.
				var page = fUIEventSource as Microsoft.Phone.Controls.PhoneApplicationPage;
				var backgroundGrid = point.TouchDevice.DirectlyOver as System.Windows.Controls.DrawingSurfaceBackgroundGrid;
				if ((page == null) || (backgroundGrid == null))
				{
					isWithinBounds = false;
				}
			}

			// Handle touch points outside of the UI control's bounds.
			if (isWithinBounds == false)
			{
				if ((phase == Corona.WinRT.Interop.Input.TouchPhase.Ended) ||
				    (phase == Corona.WinRT.Interop.Input.TouchPhase.Canceled))
				{
					// The touch action has ended. Use the last valid point received within the UI control's bounds.
					fLastHandledTouchPoints.TryGetValue(pointerId, out point);
				}
				else
				{
					// Ignore touch moves that happened outside of the UI control.
					return;
				}
			}

			// Update our collection of last received touch points within the UI control's bounds.
			if ((phase == Corona.WinRT.Interop.Input.TouchPhase.Ended) ||
			    (phase == Corona.WinRT.Interop.Input.TouchPhase.Canceled))
			{
				fLastHandledTouchPoints.Remove(pointerId);
			}
			else
			{
				fLastHandledTouchPoints[pointerId] = point;
			}

			// Fetch the event table.
			var eventTokenTable = ReceivedTouchEventTokenTable.GetOrCreateEventRegistrationTokenTable(
											ref fReceivedTouchEventTokenTable).InvocationList;
			if (eventTokenTable == null)
			{
				return;
			}

			// Convert the touch coordinates to pixels.
			double scaleFactor = (double)System.Windows.Application.Current.Host.Content.ScaleFactor / 100.0;
			double pixelPointX = Math.Round(point.Position.X * scaleFactor, MidpointRounding.AwayFromZero);
			double pixelPointY = Math.Round(point.Position.Y * scaleFactor, MidpointRounding.AwayFromZero);

			// Store the touch coordinate and timestamp to Corona's touch point type.
			Corona.WinRT.Interop.Input.TouchPoint coronaTouchPoint;
			coronaTouchPoint.X = pixelPointX;
			coronaTouchPoint.Y = pixelPointY;
			coronaTouchPoint.Timestamp = timestamp;

			// Raise a "ReceivedTouch" event.
			var eventArgs = new Corona.WinRT.Interop.Input.TouchEventArgs(pointerId, phase, coronaTouchPoint);
			eventTokenTable(null, eventArgs);
		}

		/// <summary>Creates a "KeyEventArgs" object for the given key.</summary>
		/// <remarks>
		///  This method will also fetch the current down/up state of modifiers keys Alt, Control, and Shift
		///  and store those states into the returned event arguments object.
		/// </remarks>
		/// <param name="key">
		///  <para>The key to be wrapped by a KeyEventArgs object.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <returns>Returns a new KeyEventArgs object wrapping the given key.</returns>
		private Corona.WinRT.Interop.Input.KeyEventArgs CreateKeyEventFor(Corona.WinRT.Interop.Input.Key key)
		{
			// Validate argument.
			if (key == null)
			{
				throw new NullReferenceException();
			}

			// Determine if modifier keys Alt, Control, and Shift are currently pressed down.
			var modifierKeys = System.Windows.Input.Keyboard.Modifiers;
			var commandKeyMask = System.Windows.Input.ModifierKeys.Windows | System.Windows.Input.ModifierKeys.Apple;
			bool isCommandDown = ((modifierKeys & commandKeyMask) != 0);
			bool isAltDown = ((modifierKeys & System.Windows.Input.ModifierKeys.Alt) != 0);
			bool isConrolDown = ((modifierKeys & System.Windows.Input.ModifierKeys.Control) != 0);
			bool isShiftDown = ((modifierKeys & System.Windows.Input.ModifierKeys.Shift) != 0);

			// Create and return the event for the given key.
			return new Corona.WinRT.Interop.Input.KeyEventArgs(key, isAltDown, isConrolDown, isShiftDown, isCommandDown);
		}

		#endregion
	}
}
