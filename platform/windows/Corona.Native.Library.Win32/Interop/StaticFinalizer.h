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

#include "Core\Rtt_Macros.h"
#include <mutex>
#include <unordered_set>


namespace Interop {

/// <summary>
///  <para>Finalizer used to invoke a callback when its destructor gets called.</para>
///  <para>
///   Intended to be created as a static object so that its callback can cleanup its associated static variables.
///  </para>
///  <para>
///   You can establish dependency chains to ensure that static variables associated with one finalizer will be
///   accessible to another finalizer's callback, working-around static destruction order issues between C/C++ files.
///  </para>
/// </summary>
class StaticFinalizer
{
	Rtt_CLASS_NO_COPIES(StaticFinalizer)

	public:
		/// <summary>Defines the function signature to be passed into the StaticFinalizer class' constructor.</summary>
		typedef void(*Callback)();


		/// <summary>
		///  <para>Creates a finalizer that does not invoke a callback.</para>
		///  <para>This is only useful as a dependency target for other finalizers that do have callbacks.</para>
		/// </summary>
		StaticFinalizer();

		/// <summary>Creates a finalizer that will invoke the given callback upon destruction.</summary>
		/// <param name="callback">The callback to be invoked upon destruction. Can be null.</param>
		StaticFinalizer(StaticFinalizer::Callback callback);

		/// <summary>
		///  Invokes the finalizer's callback and the callbacks of all the finalizer it depends on in the correct order.
		/// </summary>
		virtual ~StaticFinalizer();


		/// <summary>
		///  <para>Determines if this finalizer has any direct dependencies.</para>
		///  <para>Direct dependencies are established by calling this object's AddDependency() method.</para>
		/// </summary>
		/// <returns>Returns true if this finalizer has any direct dependencies. Returns false if not.</returns>
		bool HasDependencies() const;

		/// <summary>
		///  <para>Establishes the given finalizer as a direct dependency.</para>
		///  <para>
		///   This guarantees that this finalizer's callback will be invoked before the given finalizer's callback.
		///  </para>
		///  <para>
		///   This is needed in case this finalizer's callback depends on static variables associated with
		///   the given finalizer.
		///  </para>
		/// </summary>
		/// <param name="finalizer">Reference to the finalizer to establish a direct dependency with.</param>
		void AddDependency(const StaticFinalizer& finalizer);

		/// <summary>
		///  <para>Removes the given finalizer as a dependency.</para>
		///  <para>Only applicable if the given finalizer was once passed into the AddDependency() method.</para>
		/// </summary>
		/// <param name="finalizer">Reference to the finalizer to be removed as a dependency.</param>
		void RemoveDependency(const StaticFinalizer& finalizer);

	private:
		/// <summary>
		///  <para>To be called when the finalizer's destructor has been invoked.</para>
		///  <para>Invokes the finalizer's callback as well as all of its direct and indirect finalizer callbacks.</para>
		/// </summary>
		void OnFinalize();


		/// <summary>
		///  <para>Set true if this finalizer's callback has already been invoked. Set false if not invoked yet.</para>
		///  <para>Finalization occurs via the finalizer class' destructor or by one of its dependent finalizers.</para>
		/// </summary>
		bool fWasFinalized;

		/// <summary>The callback to be invoked upon finalization.</summary>
		StaticFinalizer::Callback fCallback;

		/// <summary>
		///  <para>Collection of finalizers that this finalizer depends on.</para>
		///  <para>These finalizers are to be invoked after this finalizer.</para>
		/// </summary>
		std::unordered_set<StaticFinalizer*> fDirectDependencies;

		/// <summary>
		///  <para>Collection of finalizers that depend on this finalizer.</para>
		///  <para>These finalizers are to be invoked before this finalizer.</para>
		/// </summary>
		std::unordered_set<StaticFinalizer*> fIndirectDependencies;

		/// <summary>Mutex used to synchronize access to member variables between threads.</summary>
		mutable std::recursive_mutex fMutex;
};

}	// namespace Interop
