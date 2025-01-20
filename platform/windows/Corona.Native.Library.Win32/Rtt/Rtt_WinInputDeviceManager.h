//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Input\Rtt_PlatformInputDeviceManager.h"
#include "Interop\Input\InputDeviceMonitor.h"
#include "Interop\Input\TapTracker.h"
#include "Interop\UI\RenderSurfaceControl.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_Event.h"
#include <Windows.h>


#pragma region Forward Declarations
namespace Rtt
{
	class InputDeviceDescriptor;
	class PlatformInputDevice;
}

#pragma endregion


namespace Rtt
{

	/// <summary>
	///  Manages Windows input devices such as a mouse or keyboard and dispatches their input events
	///  to the Corona runtime's Lua state.
	/// </summary>
	class WinInputDeviceManager : public PlatformInputDeviceManager
	{
		Rtt_CLASS_NO_COPIES(WinInputDeviceManager)

		public:
		#pragma region Public CursorStyle Enum
		/// <summary>
		///  <para>Indicates the type of mouse cursor to be used such as kDefaultArrow, kPointingHand, etc.</para>
		///  <para>Intended to be passed to the WinInputDeviceManager class' SetCursor() metohd.</para>
		/// </summary>
		enum class CursorStyle : WORD
		{
			kAppStarting = (WORD)IDC_APPSTARTING,
			kDefaultArrow = (WORD)IDC_ARROW,
			kCrosshair = (WORD)IDC_CROSS,
			kPointingHand = (WORD)IDC_HAND,
			kHelp = (WORD)IDC_HELP,
			kIBeam = (WORD)IDC_IBEAM,
			kSlashedCircle = (WORD)IDC_NO,
			kMove = (WORD)IDC_SIZEALL,
			kSizeNorthEastSouthWest = (WORD)IDC_SIZENESW,
			kSizeNorthSouth = (WORD)IDC_SIZENS,
			kSizeNorthWestSouthEast = (WORD)IDC_SIZENWSE,
			kSizeWestEast = (WORD)IDC_SIZEWE,
			kUpArrow = (WORD)IDC_UPARROW,
			kHourGlass = (WORD)IDC_WAIT,
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new Windows input device manager.</summary>
		/// <param name="environment">
		///  <para>Reference to the Corona runtime environment this object will be tied to.</para>
		///  <para>Windows input device events will be dispatched as Corona events to this runtime's Lua state.</para>
		/// </param>
		WinInputDeviceManager(Interop::RuntimeEnvironment& environment);

		/// <summary>Destroys this object and its allocated resources.</summary>
		virtual ~WinInputDeviceManager();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Determines if the operating system and hardware supports multitouch touchscreen input.</summary>
		/// <returns>Returns true if capable of receiving multitouch input. Returns false if not.</returns>
		bool IsMultitouchSupported() const;

		/// <summary>
		///  <para>Shows/hides the mouse cursor while it is hovering over the Corona rendering surface control.</para>
		///  <para>Intended to be called by the Lua native.setProperty("mouseCursorVisible") function.</para>
		///  <para>
		///   Note: This will not hide the cursor while IsWaitCursorEnabled() is true. But it will hide all other cursors.
		///  </para>
		/// </summary>
		/// <param name="value">Set true to show the mouse cursor. Set false to hide it.</param>
		void SetCursorVisible(bool value);

		/// <summary>
		///  Determines if the cursor is set up to be hidden or shown while hovering over the
		///  Corona rendering surface control.
		/// </summary>
		/// <returns>Returns true if the cursor is set up to be shown. Returns false if hidden.</returns>
		bool IsCursorVisible() const;

		/// <summary>
		///  Sets the mouse cursor icon to be shown while it is hovering over the control that Corona is rendering to.
		/// </summary>
		/// <param name="value">The mouse cursor icon to be shown such as kDefaultArrow, kPointingHand, etc.</param>
		void SetCursor(WinInputDeviceManager::CursorStyle value);

		/// <summary>
		///  Gets the icon type that will be shown while the mouse is hovering over the control that Corona renders to.
		/// </summary>
		/// <returns>Returns the mouse cursor style such as kDefaultArrow, kPointingHand, etc.</returns>
		WinInputDeviceManager::CursorStyle GetCursor() const;

		/// <summary>
		///  <para>Enables or disables the mouse wait cursor.</para>
		///  <para>Intended to be called by the Lua native.setActivityIndicator() for Win32 desktop apps.</para>
		/// </summary>
		/// <param name="value">Set true to show a "wait" mouse cursor. Set false to show the default mouse cursor.</param>
		void SetWaitCursorEnabled(bool value);

		/// <summary>Determines if the mouse "wait" cursor is enabled or not.</summary>
		/// <returns>
		///  <para>Returns true if set up to display a "wait" mouse cursor.</para>
		///  <para>Returns false if set up to display the default mouse cursor.</para>
		/// </returns>
		bool IsWaitCursorEnabled() const;

		#pragma endregion

		protected:
		#pragma region Protected Methods
		/// <summary>Called when this device manager needs a new input device object to be created.</summary>
		/// <param name="descriptor">Unique descriptor used to identify the new input device.</param>
		/// <returns>Returns a pointer to the newly created input device object.</returns>
		virtual PlatformInputDevice* CreateUsing(const InputDeviceDescriptor& descriptor) override;

		/// <summary>Called when this device manager needs to destroy the given input device object.</summary>
		/// <param name="devicePointer">Pointer to the input device object to be deleted.</param>
		virtual void Destroy(PlatformInputDevice* devicePointer) override;

		#pragma endregion

		private:
		#pragma region Private Constants
		enum
		{
			/// <summary>
			///  <para>Maximum number of touchscreen touch points supported at the same time.</para>
			///  <para>This is a limitation imposed by this class' member variable "fTouchPointStates".</para>
			/// </summary>
			kMaxTouchPoints = 16
		};

		#pragma endregion


		#pragma region Private TouchPointState Struct
		/// <summary>
		///  <para>Stores the current state of a single touchscreen touch point.</para>
		///  <para>Used to track the starting point and the last received touch point.</para>
		/// </summary>
		struct TouchPointState
		{
			/// <summary>Set true if a touch event has started.</summary>
			bool HasStarted;

			/// <summary>The touch or mouse drag starting coordinate.</summary>
			POINT StartPoint;

			/// <summary>
			///  <para>The last touch/mouse point received.</para>
			///  <para>Used to cancel a touch event.</para>
			/// </summary>
			POINT LastPoint;

			/// <summary>Stores the unique identifier assigned to the Win32 "TOUCHINPUT.dwID" field.</summary>
			DWORD TouchInputId;
		};

		#pragma endregion


		#pragma region Private Methods
		/// <summary>
		///  <para>Called when the Corona runtime has just loaded its project.</para>
		///  <para>Raised after loading the "config.lua" file and just but before executing the "shell.lua" file.</para>
		/// </summary>
		/// <param name="sender">The RuntimeEnvironment instance that raised this event.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnRuntimeLoaded(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);

		/// <summary>Called when the Corona runtime has just been resumed after a suspension.</summary>
		/// <param name="sender">The RuntimeEnvironment instance that raised this event.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnRuntimeResumed(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);

		/// <summary>Called when the Corona runtime has just been suspended.</summary>
		/// <param name="sender">The RuntimeEnvironment instance that raised this event.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnRuntimeSuspended(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);

		/// <summary>Called when a new input device has been discovered by the system.</summary>
		/// <param name="sender">The InputDeviceMonitor instance that raised this event.</param>
		/// <param name="arguments">Provides information about the newly discovered input device.</param>
		void OnDiscoveredDevice(
			Interop::Input::InputDeviceMonitor& sender, Interop::Input::InputDeviceInterfaceEventArgs& arguments);

		/// <summary>Called when the rendering surface has received a Windows message.</summary>
		/// <param name="sender">Reference to the window/control that received the Windows message.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments);

		/// <summary>
		///  <para>To be called when a mouse event has been received.</para>
		///  <para>Dispatches the given data as a Corona "mouse" event to Lua.</para>
		/// </summary>
		/// <param name="eventType">Mouse event type such as kDown, kUp, kMove, etc.</param>
		/// <param name="point">The mouse cursor's current position in Corona content coordinates.</param>
		/// <param name="scrollWheelDeltaX">
		///  The distance traveled by the mouse's horizontal scroll wheel in Corona content coordinates.
		/// </param>
		/// <param name="scrollWheelDeltaY">
		///  The distance traveled by the mouse's vertical scroll wheel in Corona content coordinates.
		/// </param>
		/// <param name="mouseButtonFlags">The WParam data provided by the Windows mouse message.</param>
		void OnReceivedMouseEvent(
			Rtt::MouseEvent::MouseEventType eventType, POINT& point,
			float scrollWheelDeltaX, float scrollWheelDeltaY, WPARAM mouseButtonFlags);

		/// <summary>
		///  <para>To be called when a mouse/touch event has been received.</para>
		///  <para>Dispatches the given data as a Corona "touch" event to Lua.</para>
		/// </summary>
		/// <param name="touchIndex">
		///  Zero based index assigned to the touch point. Index 0 is expected to be the primary finger/touch point.
		/// </param>
		/// <param name="currentPosition">The current mouse/touch position in Corona content coordinates.</param>
		/// <param name="startPosition">
		///  The position where the mouse/touch drag started in Corona content coordinates.
		/// </param>
		/// <param name="phase">The touch phase such as kBegan, kMoved, or kEnded.</param>
		void OnReceivedTouchEvent(
			uint32_t touchIndex, POINT currentPosition, POINT startPosition, Rtt::TouchEvent::Phase phase);

		/// <summary>
		///  <para>Extract the mouse x/y coordinate from the given Windows message LPARAM.</para>
		///  <para>
		///   Automatically applies the Corona Simulator current device's zoom level scale and rotation
		///   to the returned coordinate, if applicable.
		///  </para>
		/// </summary>
		/// <param name="LParam">The LPARAM part of the Windows message providing the mouse coordinates.</param>
		/// <returns>
		///  Returns the extracted mouse coordinate with the Corona Simulator's device scaling/rotation
		///  already applied to it, if applicable.
		/// </returns>
		POINT GetMousePointFrom(LPARAM LParam);

		/// <summary>
		///  <para>
		///   Determines if the currently received Win32 mouse message such as WM_LBUTTONDOWN, WM_MOUSEMOVE, etc.
		///   was generated by the touchscreen instead of a physical mouse.
		///  </para>
		///  <para>
		///   This method calls the Win32 GetMessageExtraInfo() method which is stateful. Meaning that this method
		///   is expected to be called when a Win32 mouse message has just been received by a WndProc callback.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if the last received mouse message was generated by a touchscreen.</para>
		///  <para>Returns false if generated by an actual mouse.</para>
		/// </returns>
		bool WasMouseMessageGeneratedFromTouchInput();

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Reference to the Corona runtime environment that owns this object.</summary>
		Interop::RuntimeEnvironment& fEnvironment;

		/// <summary>Handler to be invoked when the runtime's "Loaded" event has been raised.</summary>
		Interop::RuntimeEnvironment::LoadedEvent::MethodHandler<WinInputDeviceManager> fRuntimeLoadedEventHandler;

		/// <summary>Handler to be invoked when the runtime's "Resumed" event has been raised.</summary>
		Interop::RuntimeEnvironment::ResumedEvent::MethodHandler<WinInputDeviceManager> fRuntimeResumedEventHandler;

		/// <summary>Handler to be invoked when the runtime's "Suspended" event has been raised.</summary>
		Interop::RuntimeEnvironment::SuspendedEvent::MethodHandler<WinInputDeviceManager> fRuntimeSuspendedEventHandler;

		/// <summary>Handler to be invoked when the device monitor's "DiscoveredDevice" event has been raised.</summary>
		Interop::Input::InputDeviceMonitor::DiscoveredDeviceEvent::MethodHandler<WinInputDeviceManager> fDiscoveredDeviceEventHandler;

		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		Interop::UI::RenderSurfaceControl::ReceivedMessageEvent::MethodHandler<WinInputDeviceManager> fReceivedMessageEventHandler;

		/// <summary>Monitors input devices such as keyboards, mice, and game controllers.</summary>
		Interop::Input::InputDeviceMonitor fDeviceMonitor;

		/// <summary>
		///  <para>
		///   Set true if multitouch is supported by the system and that this class will be able to deliver
		///   real touchscreen "touch" events to Lua.
		///  </para>
		///  <para>
		///   Set false if multitouch touchscreen is not supported, which will always be the case for
		///   Windows Vista and older OS versions.
		///  </para>
		/// </summary>
		bool fIsMultitouchSupported;

		/// <summary>
		///  <para>Stores information about each touch point/finger on the touchscreen.</para>
		///  <para>
		///   The first element in the array is expected to store the primary finger's touch information
		///   and/or simulated touch events from the mouse.
		///  </para>
		/// </summary>
		TouchPointState fTouchPointStates[kMaxTouchPoints];

		/// <summary>
		///  <para>Tracker used to determine if the received touch events should trigger a Lua "tap" event.</para>
		///  <para>Received touch events are expected to be passed to this object's UpdateWith() method.</para>
		///  <para>
		///   After calling UpdateWith(), you are expected to call HasTapOccurred() to determine if
		///   a "tap" event should be dispatched.
		///  </para>
		/// </summary>
		Interop::Input::TapTracker fTapTracker;

		/// <summary>
		///  <para>Set true if member variable "fLastMouseMovePoint" is storing a valid value.</para>
		///  <para>Set false if it should be ignored.</para>
		/// </summary>
		bool fIsLastMouseMovePointValid;

		/// <summary>Stores the last WM_MOUSEMOVE message's x/y mouse position in Corona content coordinates.</summary>
		POINT fLastMouseMovePoint;

		/// <summary>Set true to show a mouse cursor. Set false to hide the cursor.</summary>
		bool fIsCursorVisible;

		/// <summary>Set true to show a "wait" mouse cursor. Set false to show the default cursor.</summary>
		bool fIsWaitCursorEnabled;

		/// <summary>Cursor type to be shown when the mouse is hovering over the control Corona renders to.</summary>
		CursorStyle fCursorStyle;

		#pragma endregion
	};

}	// namespace Rtt
