//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_PlatformSurface.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	namespace Interop {
		namespace Graphics {
			interface class IRenderSurface;
		}
		namespace UI {
			interface class IPage;
		}
	}
	ref class CoronaRuntimeEnvironment;
} } }

#pragma endregion


namespace Rtt
{

class WinRTScreenSurface : public PlatformSurface
{
	Rtt_CLASS_NO_COPIES(WinRTScreenSurface)

	public:
		/// <summary>
		///  Creates a new screen surface interface used by Corona's renderer to acquire the surface size and orientation.
		/// </summary>
		/// <param name="environment">
		///  <para>Provides the Corona control's render surface and page needed by this surface interface.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		WinRTScreenSurface(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment);

		/// <summary>Destroys resources created by this surface.</summary>
		virtual ~WinRTScreenSurface();

		/// <summary>
		///  Called when the rendering system want to make this surface's Direct3D/OpenGL context the current context.
		/// </summary>
		virtual void SetCurrent() const;

		/// <summary>Called by the rendering system to swap the back buffer it just drawed to with the front buffer.</summary>
		virtual void Flush() const;

		/// <summary>Gets the surface's pixel width relative to the orientation returned by GetOrientation().</summary>
		/// <returns>Returns the pixel width relative to the surface's current orientation.</returns>
		virtual S32 Width() const;

		/// <summary>Gets the surface's pixel height relative to the orientation returned by GetOrientation().</summary>
		/// <returns>Returns the pixel height relative to the surface's current orientation.</returns>
		virtual S32 Height() const;

		/// <summary>Gets the surface's orientation relative to portrait.</summary>
		/// <returns>
		///  <para>Returns kUpright if the surface is portrait.</para>
		///  <para>
		///   Returns kSidewaysRight if the surface is landscape, rotated 90 degrees clockwise compared to portrait.
		///  </para>
		///  <para>
		///   Returns kSidewaysLeft if the surface is landscape, rotated 90 degrees counter-clockwise compared to portrait.
		///  </para>
		///  <para>Returns kUpsideDown if the surface is 180 degrees rotated compared to portrait.</para>
		/// </returns>
		virtual DeviceOrientation::Type GetOrientation() const;

		/// <summary>Gets the surface's pixel width relative to a portrait orientation.</summary>
		/// <returns>Returns pixel width relative to portrait.</returns>
		virtual S32 DeviceWidth() const;

		/// <summary>Gets the surface's pixel height relative to a portrait orientation.</summary>
		/// <returns>Returns pixel height relative to portrait.</returns>
		virtual S32 DeviceHeight() const;

		/// <summary>
		///  Gets a Microsoft recommended standard scaled width (relative to portrait) based on the device resolution and DPI.
		/// </summary>
		/// <returns>Returns a Microsoft standard scaled width relative to portrait.</returns>
		virtual S32 AdaptiveWidth() const;

		/// <summary>
		///  Gets a Microsoft recommended standard scaled height (relative to portrait) based on the device resolution and DPI.
		/// </summary>
		/// <returns>Returns a Microsoft standard scaled height relative to portrait.</returns>
		virtual S32 AdaptiveHeight() const;

	private:
		CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ fEnvironment;
		CoronaLabs::Corona::WinRT::Interop::Graphics::IRenderSurface^ fRenderSurface;
		CoronaLabs::Corona::WinRT::Interop::UI::IPage^ fPage;
};

class WinRTOffscreenSurface : public PlatformSurface
{
	public:
		WinRTOffscreenSurface(const PlatformSurface& parent);
		virtual ~WinRTOffscreenSurface();

		virtual void SetCurrent() const;
		virtual void Flush() const;
		virtual S32 Width() const;
		virtual S32 Height() const;

	private:
		S32 fWidth;
		S32 fHeight;
};

} // namespace Rtt
