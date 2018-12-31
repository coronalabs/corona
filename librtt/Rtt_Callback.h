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

#ifndef _Rtt_Callback_H__
#define _Rtt_Callback_H__

#include "Rtt_MCallback.h"


namespace Rtt
{

/// Abstract callback class which implements the MCallback interface.
class BaseCallback : public MCallback
{
	public:
		virtual void Invoke() = 0;
		void operator()()
		{
			Invoke();
		}
};

/// Callback used to invoke a C function or a C++ class static function.
/// Implements the MCallback interface so that it can be used by Corona's core classes.
class FunctionCallback : public BaseCallback
{
	public:
		typedef void(*EmptyFunctionType)();

		FunctionCallback()
		{
			fFunctionPointer = NULL;
		}

		FunctionCallback(EmptyFunctionType functionPointer)
		{
			fFunctionPointer = functionPointer;
		}

		virtual void Invoke()
		{
			if (fFunctionPointer)
			{
				fFunctionPointer();
			}
		}

	private:
		EmptyFunctionType fFunctionPointer;
};

/// Callback used to invoke a C++ object's method.
/// Implements the MCallback interface so that it can be used by Corona's core classes.
template<class TClass>
class MethodCallback : public BaseCallback
{
	public:
		typedef void(TClass::*EmptyMethodType)();

		MethodCallback()
		{
			fObjectPointer = NULL;
			fMethodPointer = NULL;
		}

		MethodCallback(TClass *objectPointer, EmptyMethodType methodPointer)
		{
			fObjectPointer = objectPointer;
			fMethodPointer = methodPointer;
		}

		TClass* GetObject() const
		{
			return fObjectPointer;
		}

		virtual void Invoke()
		{
			if (fObjectPointer && fMethodPointer)
			{
				(*fObjectPointer.*fMethodPointer)();
			}
		}

	private:
		TClass *fObjectPointer;
		EmptyMethodType fMethodPointer;
};

} // namespace Rtt

#endif // _Rtt_Callback_H__
