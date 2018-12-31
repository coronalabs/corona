//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Core\Rtt_Geometry.h"
#include "Interop\UI\UIComponent.h"
#include "Rtt_PlatformDisplayObject.h"


#pragma region Forward Declarations
namespace Interop
{
	namespace UI
	{
		class Control;
		class HandleMessageEventArgs;
	}
	class RuntimeEnvironment;
}
namespace Rtt
{
	class Display;
	class Renderer;
}
extern "C"
{
	struct lua_State;
}

#pragma endregion


namespace Rtt
{

class WinDisplayObject : public PlatformDisplayObject
{
	public:
		WinDisplayObject(Interop::RuntimeEnvironment& environment, const Rect& bounds);
		virtual ~WinDisplayObject();

		Interop::RuntimeEnvironment& GetRuntimeEnvironment() const;
		void SetFocus() const;
		virtual Interop::UI::Control* GetControl() const = 0;
		virtual bool Initialize();
		virtual void DidMoveOffscreen();
		virtual void WillMoveOnscreen();
		virtual bool CanCull() const;
		virtual void Prepare(const Display& display);
		virtual void Draw(Renderer& renderer) const;
		virtual void GetSelfBounds(Rect& rect) const;
		virtual void SetSelfBounds(Real width, Real height);
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);

	private:
		/// <summary>Called when a Windows message has been dispatched to this object's native UI control.</summary>
		/// <param name="sender">Reference to the control the message was dispatched to.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments);


		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		Interop::UI::UIComponent::ReceivedMessageEvent::MethodHandler<WinDisplayObject> fReceivedMessageEventHandler;

		/// <summary>Reference to the Corona runtime environment that owns this object.</summary>
		Interop::RuntimeEnvironment& fEnvironment;

		/// <summary>The bounds of the native UI object in Corona coordinates.</summary>
		Rect fSelfBounds;
};

}	// namespace Rtt
