//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CachedPath_H__
#define _Rtt_CachedPath_H__

#include "Display/Rtt_DisplayPath.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class ClosedPath;

// ----------------------------------------------------------------------------

class CachedPath : public DisplayPath
{
	Rtt_CLASS_NO_COPIES( CachedPath )

	public:
		CachedPath( ClosedPath& path );

	public:
		Rtt_FORCE_INLINE const ClosedPath* operator->() const { return & fPath; }
		Rtt_FORCE_INLINE ClosedPath* operator->() { return & fPath; }

		Rtt_FORCE_INLINE ClosedPath& OriginalPath() const { return fPath; }

	public:
//		virtual void Build( const Matrix& srcToDstSpace );
		virtual void GetSelfBounds( Rect& rect ) const;

	private:
		ClosedPath& fPath;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CachedPath_H__
