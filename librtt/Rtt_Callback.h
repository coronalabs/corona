//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
