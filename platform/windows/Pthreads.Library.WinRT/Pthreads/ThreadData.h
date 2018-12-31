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

#include "pthread.h"
#include "KeyDataCollection.h"
#include <hash_map>
#include <memory>
#include <thread>


namespace Pthreads {

class ThreadData
{
	public:
		ThreadData();
		virtual ~ThreadData();

		std::shared_ptr<std::thread> GetStandardThreadPointer();
		void SetStandardThreadPointer(std::thread *threadPointer);
		KeyDataCollection& GetKeyDataCollection();
		void* GetExitData();
		void SetExitData(void* data);

	private:
		std::shared_ptr<std::thread> fStandardThreadPointer;
		KeyDataCollection fKeyDataCollection;
		void* fExitData;
};

}	// namespace Pthreads
