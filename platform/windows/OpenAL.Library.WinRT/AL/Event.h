//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "BaseEvent.h"
#include <mutex>
#include <vector>


namespace AL {

template<class TSender, class TEventArgs>
/// <summary>
///  Defines an event type used to store a collection of event handlers to be easily invoked/raised by the owner
///  of the event, similar to the event handling model in the .NET framework.
/// </summary>
class Event : public BaseEvent
{
	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new event source used to store a collection of handlers to be invoked.</summary>
		Event()
		{
			fHandlerManager = HandlerManager(this);
		}

		/// <summary>Destroys this event and its resources.</summary>
		virtual ~Event()
		{
		}

		#pragma endregion


		#pragma region Handler Class
		/// <summary>
		///  <para>Stores a callback to be invoked when an event gets raised.</para>
		///  <para>
		///   This is an abstract class from which the FunctionHandler and MethodHandler classes derive from.
		///   Instances of those classes are expected to be created and passed into the Event::HandlerManager class'
		///   Add() method so that they'll be invoked when the event gets raised.
		///  </para>
		/// </summary>
		class Handler : public BaseEvent::Handler
		{
			public:
				virtual void Invoke(TSender sender, TEventArgs arguments) = 0;
		};

		#pragma endregion


		#pragma region FunctionHandler Class
		/// <summary>
		///  <para>Stores a C/C++ function pointer that will be called when an event has been raised.</para>
		///  <para>Instances of this class are expected to be given to the Event::HandlerManager class' Add() method</para>
		/// </summary>
		class FunctionHandler : public Handler
		{
			public:
				/// <summary>Defines the function signature to be passed into the handler's constructor.</summary>
				typedef void(*FunctionType)(TSender, TEventArgs);

				/// <summary>Creates a handler without a callback. Will do nothing when invoked.</summary>
				FunctionHandler()
				{
					fFunctionPointer = nullptr;
				}

				/// <summary>
				///  Creates a new event handler which will call the given C/C++ function when the event gets raised.
				/// </summary>
				/// <param name="functionPointer">
				///  <para>Pointer to a C/C++ function or a C++ static function in a class.</para>
				///  <para>Can be null, but then this handler will do nothing when invoked.</para>
				/// </param>
				FunctionHandler(FunctionType functionPointer)
				{
					fFunctionPointer = functionPointer;
				}

				/// <summary>Invokes this handler's function/callback with the given arguments.</summary>
				/// <param name="sender">The object that raised this event.</param>
				/// <param name="arguments">The event's information.</param>
				virtual void Invoke(TSender sender, TEventArgs arguments)
				{
					if (fFunctionPointer)
					{
						(*fFunctionPointer)(sender, arguments);
					}
				}

			private:
				FunctionType fFunctionPointer;
		};

		#pragma endregion


		#pragma region MethodHandler Class
		template<class TClass>
		/// <summary>
		///  <para>Stores a C++ object's method pointer that will be called when an event has been raised.</para>
		///  <para>Instances of this class are expected to be given to the Event::HandlerManager class' Add() method</para>
		/// </summary>
		class MethodHandler : public Handler
		{
			public:
				/// <summary>Defines a class method signature to be passed into the handler's constructor.</summary>
				typedef void(TClass::*MethodType)(TSender, TEventArgs);

				/// <summary>Creates a handler without a callback. Will do nothing when invoked.</summary>
				MethodHandler()
				{
					fObjectPointer = nullptr;
					fMethodPointer = nullptr;
				}

				/// <summary>
				///  Creates a new event handler that will call the given object's method when the event gets raised.
				/// </summary>
				/// <param name="objectPointer">
				///  <para>Pointer to the object that the method belongs to.</para>
				///  <para>Can be null, but then this handler will do nothing when invoked.</para>
				/// </param>
				/// <param name="methodPointer">
				///  <para>Pointer to the C++ class' function to be called by this handler.</para>
				///  <para>Can be null, but then this handler will do nothing when invoked.</para>
				/// </param>
				MethodHandler(TClass *objectPointer, MethodType methodPointer)
				{
					fObjectPointer = objectPointer;
					fMethodPointer = methodPointer;
				}

				/// <summary>Invokes this handler's callback with the given arguments.</summary>
				/// <param name="sender">The object that raised this event.</param>
				/// <param name="arguments">The event's information.</param>
				virtual void Invoke(TSender sender, TEventArgs arguments)
				{
					if (fObjectPointer && fMethodPointer)
					{
						(*fObjectPointer.*fMethodPointer)(sender, arguments);
					}
				}

			private:
				TClass *fObjectPointer;
				MethodType fMethodPointer;
		};

		#pragma endregion


		#pragma region HandlerManager Class
		/// <summary>Provides a means to add or remove handlers to an Event object without having to </summary>
		class HandlerManager
		{
			public:
				/// <summary>
				///  <para>Creates an event handler manager that is not associated with any events.</para>
				///  <para>Calling its Add() or Remove() methods will do nothing in this case.</para>
				/// </summary>
				HandlerManager()
				{
					fEventPointer = nullptr;
				}

				/// <summary>
				///  <para>Creates an event handler manager which can add and remove handlers to the given event object.</para>
				///  <para>
				///   Note that this constructor is not intended to be called by external code. It is only intended
				///   to be called by the Event object itself which will provide an instance of this class via its
				///   getHandlerManager() method.
				///  </para>
				/// </summary>
				/// <param name="eventPointer">
				///  <para>Pointer to the Event object that this manager will add/remove handlers to.</para>
				///  <para>Can be null, which causes this manager to not be associated with an event.</para>
				/// </param>
				HandlerManager(Event *eventPointer)
				{
					fEventPointer = eventPointer;
				}

				/// <summary>Adds the given handler to the collection, to be called when the event gets raised.</summary>
				/// <param name="handlerPointer">
				///  <para>Pointer to the event handler to be called when the event gets raised.</para>
				///  <para>
				///   For C functions or C++ static methods, this is expected to be an instance of the FunctionHandler class.
				///  </para>
				///  <para>For C++ object methods, this is expected to be an instance of the MethodHandler class.</para>
				///  <para>
				///   The given handler pointer is expected to persist after passing it to this method. If you need to delete
				///   the owner of this pointer, then you must call this class' Remove() method before doing so.
				///  </para>
				///  <para>Will be ignored if given a null pointer.</para>
				/// </param>
				void Add(Handler *handlerPointer)
				{
					std::lock_guard<std::recursive_mutex> scopedMutexLock(fEventPointer->fMainMutex);

					// Do not add null pointers to the collection.
					if (nullptr == handlerPointer)
					{
						return;
					}

					// Do not continue if the given handler was already added.
					if (Contains(handlerPointer))
					{
						return;
					}

					// Add the given handler to the collection.
					fEventPointer->fHandlerCollection.push_back(handlerPointer);
				}

				/// <summary>
				///  <para>Removes the given event handler by its memory address.</para>
				///  <para>This prevents the handler from being invoked the next time an event gets raised.</para>
				/// </summary>
				/// <param name="handlerPointer">
				///  <para>The event handler to remove by memory address.</para>
				///  <para>Expected to be the same memory address given to this manager's Add() method.</para>
				/// </param>
				/// <returns>
				///  <para>Returns true if the given event handler was successfully removed.</para>
				///  <para>
				///   Returns false if the given event handler's memory address was not found in the event's handler collection.
				///  </para>
				/// </returns>
				bool Remove(Handler *handlerPointer)
				{
					// Synchronizes thread access to the handler collection below.
					std::lock_guard<std::recursive_mutex> scopedMainMutexLock(fEventPointer->fMainMutex);

					// Prevents a handler from being removed while it is being invoked between threads.
					std::lock_guard<std::recursive_mutex> scopedInvocationMutexLock(fEventPointer->fInvocationMutex);

					// Search for the given handler in the collection and remove it.
					HandlerCollection::iterator iterator;
					for (iterator = fEventPointer->fHandlerCollection.begin();
						iterator != fEventPointer->fHandlerCollection.end(); iterator++)
					{
						if (handlerPointer == (*iterator))
						{
							// Found it! Now remove the handler.
							fEventPointer->fHandlerCollection.erase(iterator);
							return true;
						}
					}

					// Faild to find the given handler.
					return false;
				}

				/// <summary>
				///  Determines if the given event handler is in the event's collection to be invoked when a event gets raised.
				/// </summary>
				/// <param name="handlerPointer">
				///  <para>The event handler to search for by memory address.</para>
				///  <para>Expected to be the same memory address given to this manager's Add() method.</para>
				/// </param>
				/// <returns>
				///  <para>Returns true if the given event handler was found in the collection.</para>
				///  <para>Returns false if not found.</para>
				/// </returns>
				bool Contains(Handler *handlerPointer)
				{
					if (handlerPointer)
					{
						std::lock_guard<std::recursive_mutex> scopedMutexLock(fEventPointer->fMainMutex);
						for (auto&& nextHandlerPointer : fEventPointer->fHandlerCollection)
						{
							if (handlerPointer == nextHandlerPointer)
							{
								return true;
							}
						}
					}
					return false;
				}

			private:
				Event *fEventPointer;
		};

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Gets a manager used to add or remove handlers for this event.</para>
		///  <para>This handler manager is expected to be exposed publicly by the object which owns this event.</para>
		/// </summary>
		HandlerManager& GetHandlerManager()
		{
			return fHandlerManager;
		}

		/// <summary>Raises an event by invoking all of the event handlers with the given arguments.</summary>
		/// <param name="sender">
		///  The object which is raising this event. Expected to be the object that owns this event object.
		/// </param>
		/// <param name="arguments">The event information to be passed to the event's handlers.</param>
		void Raise(TSender sender, TEventArgs arguments)
		{
			// First, copy the event handlers to a temporary collection. This is in case an invoked handler attempts
			// to add/remove a handler to the main collection when invoked, which would break iteratation.
			// Note: For best performance, avoid making a copy on the heap.
			static const int kMaxStackArraySize = 32;
			Handler *handlerPointerStackArray[kMaxStackArraySize];
			Handler **handlerPointerHeapArray = nullptr;
			Handler **handlerPointerArray = nullptr;
			int handlerCount = 0;
			{
				std::lock_guard<std::recursive_mutex> scopedMutexLock(fMainMutex);
				handlerCount = fHandlerCollection.size();
				if (handlerCount > kMaxStackArraySize)
				{
					handlerPointerHeapArray = new Handler*[handlerCount];
					handlerPointerArray = handlerPointerHeapArray;
				}
				else
				{
					handlerPointerArray = handlerPointerStackArray;
				}
				for (int index = 0; index < handlerCount; index++)
				{
					handlerPointerArray[index] = fHandlerCollection[index];
				}
			}

			// Invoke all of the handlers in the collection.
			Handler *handlerPointer;
			for (int index = 0; index < handlerCount; index++)
			{
				// Fetch the next handler.
				handlerPointer = handlerPointerArray[index];
				if (!handlerPointer)
				{
					continue;
				}

				// Block removal of this handler from other threads while we're invoking it.
				// Prevents a possible crash that can occur between the Contains() and Invoke() method calls below.
				std::lock_guard<std::recursive_mutex> scopedMutexLock(fInvocationMutex);

				// Skip this handler if it has been removed while iterating this collection.
				if (fHandlerManager.Contains(handlerPointer) == false)
				{
					continue;
				}

				// Invoke the handler.
				handlerPointer->Invoke(sender, arguments);
			}

			// Delete the temporary array if allocated on the heap.
			if (handlerPointerHeapArray)
			{
				delete[] handlerPointerHeapArray;
			}
		}

		#pragma endregion

	private:
		typedef std::vector<Handler*> HandlerCollection;

		/// <summary>Stores a collection of event handlers to be invoked when the Raise() method gets called.</summary>
		HandlerCollection fHandlerCollection;

		/// <summary>
		///  <para>Provides a public interface for adding and remove event handlers to this event.</para>
		///  <para>This allows this event object to be a private member to the object that raises the event.</para>
		/// </summary>
		HandlerManager fHandlerManager;

		/// <summary>Mutex used to synchronize access to the event handler collection between threads.</summary>
		std::recursive_mutex fMainMutex;

		/// <summary>
		///  <para>Mutex to be locked when a handler is being invoked by this class' Raise() method.</para>
		///  <para>Intended to block removal of a handler while the same handler is being invoked on another thread.</para>
		/// </summary>
		std::recursive_mutex fInvocationMutex;
};

}	// namespace AL
