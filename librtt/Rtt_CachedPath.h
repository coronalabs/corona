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
