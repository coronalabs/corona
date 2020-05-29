//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "Control.h"
#include <Windows.h>


#pragma region Forward Declarations
namespace Rtt
{
	class NativeWindowMode;
}

#pragma endregion


namespace Interop { namespace UI {

/// <summary>Represents a desktop window or dialog.</summary>
class Window : public Control
{
	Rtt_CLASS_NO_COPIES(Window)

	public:
		/// <summary>Creates a new Window object which wraps the given window handle.</summary>
		/// <param name="windowHandle">
		///  <para>Handle to a desktop window to wrap.</para>
		///  <para>Can be null, but then the Window object will do nothing.</para>
		/// </param>
		Window(HWND windowHandle);

		/// <summary>Destroys this object.</summary>
		virtual ~Window();

		/// <summary>Changes the window mode to minimized, maximized, fullscreen, or a normal window.</summary>
		/// <param name="windowMode">The mode to set the window to.</param>
		void SetWindowMode(const Rtt::NativeWindowMode& windowMode);

		/// <summary>Gets the window's current mode such as normal, minimized, maximized, or fullscreen.</summary>
		/// <returns>
		///  <para>Returns the window's current mode such as normal, minimized, maximized, or fullscreen.</para>
		///  <para>Returns kNormal if this object is not reference window handle.</para>
		/// </returns>
		const Rtt::NativeWindowMode& GetWindowMode() const;

		/// <summary>
		///  <para>Gets the width and height of the window in normal mode, including its borders.</para>
		///  <para>These values do not change when maximizing or minimizing the window.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the width and height of the window in normal mode, including its borders.</para>
		///  <para>Returns a zero by zero size if this object was not assigned a windows handle.</para>
		/// </returns>
		SIZE GetNormalModeSize() const;

		/// <summary>
		///  <para>Gets the x/y position and width/height of the window in normal mode.</para>
		///  <para>These values are in workspace coordinates, which excludes the Windows' taskbar.</para>
		///  <para>Unlike GetBounds(), the normal mode bounds do not change when maximizing or minimizing the window.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns this window's position, width, and height in workspace coordinates.</para>
		///  <para>Returns a rectangle with all zeros if this object was not assigned a windows handle.</para>
		/// </returns>
		RECT GetNormalModeBounds() const;

		/// <summary>
		///  Changes the x/y position and width/height of the window when it is in normal mode on the desktop.
		/// </summary>
		/// <param name="bounds">The window's new bounds in workspace coordinates.</param>
		void SetNormalModeBounds(const RECT& bounds);

		/// <summary>
		///  <para>Gets the x/y position and width/height of the client area within the window when in normal mode.</para>
		///  <para>The client area is the region within the window's borders and is where content is rendered.</para>
		///  <para>
		///   These values are in workspace coordinates, not client coordinates.
		///   Workspace coordinates excludes the Windows' taskbar.
		///  </para>
		///  <para>
		///   Unlike GetClientBounds(), the normal mode client bounds do not change when the window is
		///   minimized or maximized.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns the client area's position, width, and height within the window in workspace coordinates.</para>
		///  <para>Returns a rectangle with all zeros if this object was not assigned a windows handle.</para>
		/// </returns>
		RECT GetNormalModeClientBoundsInWorkspace() const;

		/// <summary>
		///  <para>Gets the width and height of the client area within the window when in normal mode.</para>
		///  <para>
		///   Unlike GetClientBounds(), the normal mode client bounds/size do not change when the window is
		///   minimized or maximized.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns the client area's width and height when the window is in normal mode.</para>
		///  <para>Returns a zero by zero size if this object was not assigned a windows handle.</para>
		/// </returns>
		SIZE GetNormalModeClientSize() const;

		/// <summary>
		///  <para>Changes the width and height of the client area within the window when in normal mode.</para>
		///  <para>This will also change the window frame's full width and height, which contains the client area.</para>
		/// </summary>
		/// <param name="size">The width and height to change the client area to.</param>
		void SetNormalModeClientSize(const SIZE& size);

	private:
		/// <summary>Fetches the Win32 show state of the window, normalized to simpler SW_* constant states.</summary>
		/// <returns>
		///  <para>Returns either SW_RESTORE, SW_MINIMIZE, or SW_MAXIMIZE.</para>
		///  <para>Returns SW_RESTORE if not referencing a window handle.</para>
		/// </returns>
		int GetNormalizedWin32ShowState() const;


		/// <summary>
		///  <para>Stores a copy of the window styles assigned to the window.</para>
		///  <para>Used by this class to restore a window back to its defaults when returning from fullscreen.</para>
		/// </summary>
		LONG fPreviousWindowStyles;
};

} }	// namespace Interop::UI
