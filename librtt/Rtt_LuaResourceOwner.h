//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaResourceOwner_H__
#define _Rtt_LuaResourceOwner_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

class LuaResource;
class MEvent;

// ----------------------------------------------------------------------------

class LuaResourceOwner
{
	public:
		LuaResourceOwner();
		virtual ~LuaResourceOwner();

	protected:
		void SetResource( LuaResource *resource );
		const LuaResource* GetResource() const { return fResource; }

	public:
		void DispatchEvent( const MEvent& e );

	private:
		LuaResource *fResource;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaResourceOwner_H__
