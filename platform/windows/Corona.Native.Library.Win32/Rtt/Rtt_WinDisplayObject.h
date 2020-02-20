//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
