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

#include <hash_map>
#include <thread>


namespace Pthreads {

class ThreadData;

class ThreadDataManager
{
	public:
		ThreadDataManager();
		virtual ~ThreadDataManager();

		ThreadData* GetBy(std::thread::id &id);
		ThreadData* GetByIndex(int index);
		int GetCount();

	private:
		class CollectionKeyHashComparer
		{
			public:
				enum { bucket_size = 1 };

				size_t operator()(const std::thread::id& id)
				{
					return id.hash();
				}

				bool operator()(const std::thread::id& x, const std::thread::id& y)
				{
					return (x.hash() == y.hash());
				}
		};

		typedef std::hash_map<std::thread::id, ThreadData*, CollectionKeyHashComparer> CollectionType;
		typedef std::pair<std::thread::id, ThreadData*> CollectionPairType;

		CollectionType fCollection;
};

}	// namespace Pthreads
