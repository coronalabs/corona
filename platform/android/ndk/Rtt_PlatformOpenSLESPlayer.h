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


#ifndef _Rtt_PlatformOpenSLESPlayer_H__
#define _Rtt_PlatformOpenSLESPlayer_H__

#include "Core/Rtt_Config.h"

#if defined(Rtt_USE_OPENSLES)

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <pthread.h>
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
#include "CoronaLibrary.h"
#include "CoronaLua.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_ResourceHandle.h"
#include "Rtt_Scheduler.h"
#include "Rtt_PlatformNotifier.h"
#include "Rtt_Event.h"
#include "Rtt_MCallback.h"


#define PREFETCH_ERROR_MASK (SL_PREFETCHEVENT_STATUSCHANGE | SL_PREFETCHEVENT_FILLLEVELCHANGE)
#define OPENSLES_BUFFERS 4

namespace Rtt
{
	inline float	fclamp(float v, float min, float max)
	{
		Rtt_ASSERT(min <= max);
		return fmax(min, fmin(v, max));
	}
	inline int	imax(int a, int b) { if (a < b) return b; else return a; }
	inline int	imin(int a, int b) { if (a < b) return a; else return b; }
	inline int	iclamp(int i, int min, int max)
	{
		Rtt_ASSERT(min <= max);
		return imax(min, imin(i, max));
	}

	// Local event
	struct openSLESoundCompletionEvent : public VirtualEvent
	{
		typedef VirtualEvent Super;
		typedef openSLESoundCompletionEvent Self;

		openSLESoundCompletionEvent(int ch, const char* handle, bool completed);

		static const char kName[];
		virtual const char* Name() const;
		virtual int Push(lua_State *L) const;

	private:
		int fChannel;
		const char* fHandle;
		bool fCompleted;
	};

	//
	// helper
	//

	// A smart (strong) pointer asserts that the pointed-to object will
	// not go away as long as the strong pointer is valid.  "Owners" of an
	// object should keep strong pointers; other objects should use a
	// strong pointer temporarily while they are actively using the
	// object, to prevent the object from being deleted.
	template<class T>
	class smart_ptr
	{
	public:
		smart_ptr(T* ptr) :
			m_ptr(ptr)
		{
			if (m_ptr)
			{
				m_ptr->add_ref();
			}
		}

		smart_ptr() : m_ptr(NULL) {}
		smart_ptr(const smart_ptr<T>& s)
			:
			m_ptr(s.m_ptr)
		{
			if (m_ptr)
			{
				m_ptr->add_ref();
			}
		}

		~smart_ptr()
		{
			if (m_ptr)
			{
				m_ptr->drop_ref();
			}
		}

		//	operator bool() const { return m_ptr != NULL; }
		void	operator=(const smart_ptr<T>& s) { set_ref(s.m_ptr); }
		void	operator=(T* ptr) { set_ref(ptr); }
		//	void	operator=(const weak_ptr<T>& w);
		T*	operator->() const { return m_ptr; }
		T*	get() const { return m_ptr; }
		T& operator*() const { return *(T*)m_ptr; }
		operator T*() const { return m_ptr; }
		bool	operator==(const smart_ptr<T>& p) const { return m_ptr == p.m_ptr; }
		bool	operator!=(const smart_ptr<T>& p) const { return m_ptr != p.m_ptr; }
		bool	operator==(T* p) const { return m_ptr == p; }
		bool	operator!=(T* p) const { return m_ptr != p; }

		// Provide work-alikes for static_cast, dynamic_cast, implicit up-cast?  ("gentle_cast" a la ajb?)

	private:
		void	set_ref(T* ptr)
		{
			if (ptr != m_ptr)
			{
				if (m_ptr)
				{
					m_ptr->drop_ref();
				}
				m_ptr = ptr;

				if (m_ptr)
				{
					m_ptr->add_ref();
				}
			}
		}

		//	friend weak_ptr;

		T*	m_ptr;
	};


	// Helper for making objects that can have weak_ptr's.
	class weak_proxy
	{
	public:
		weak_proxy() :	m_ref_count(0),
			m_alive(true)
		{
		}

		// weak_ptr's call this to determine if their pointer is valid or not.
		bool	is_alive() const { return m_alive; }

		// Only the actual object should call this.
		void	notify_object_died() { m_alive = false; }

		void	add_ref()
		{
			m_ref_count++;
		}
		void	drop_ref()
		{
			m_ref_count--;
			if (m_ref_count == 0)
			{
				// Now we die.
				delete this;
			}
		}

	private:
		// Don't use these.
		weak_proxy(const weak_proxy& w) {}
		void	operator=(const weak_proxy& w) {}

		int	m_ref_count;
		bool	m_alive;
	};


	// A weak pointer points at an object, but the object may be deleted
	// at any time, in which case the weak pointer automatically becomes
	// NULL.  The only way to use a weak pointer is by converting it to a
	// strong pointer (i.e. for temporary use).
	//
	// The class pointed to must have a "weak_proxy* get_weak_proxy()" method.
	//
	// Usage idiom:
	//
	// if (smart_ptr<my_type> ptr = m_weak_ptr_to_my_type) { ... use ptr->whatever() safely in here ... }

	template<class T>
	class weak_ptr
	{
	public:
		weak_ptr() :	m_ptr(0)
		{
		}

		weak_ptr(T* ptr) :	m_ptr(0)
		{
			operator=(ptr);
		}

		weak_ptr(const smart_ptr<T>& ptr)
		{
			operator=(ptr.get());
		}

		// Default constructor and assignment from weak_ptr<T> are OK.

		void	operator=(T* ptr)
		{
			m_ptr = ptr;
			if (m_ptr)
			{
				m_proxy = m_ptr->get_weak_proxy();
				//			assert(m_proxy != NULL);
				//			assert(m_proxy->is_alive());
			}
			else
			{
				m_proxy = NULL;
			}
		}

		void	operator=(const smart_ptr<T>& ptr) { operator=(ptr.get()); }

		bool	operator==(const smart_ptr<T>& ptr) const
		{
			check_proxy();
			return m_ptr == ptr.get();
		}

		bool	operator!=(const smart_ptr<T>& ptr) const
		{
			check_proxy();
			return m_ptr != ptr.get();
		}

		bool	operator==(T* ptr) const
		{
			check_proxy();
			return m_ptr == ptr;
		}

		bool	operator!=(T* ptr) const
		{
			check_proxy();
			return m_ptr != ptr;
		}

		T*	operator->() const
		{
			check_proxy();
			assert(m_ptr);
			return m_ptr;
		}

		T*	get() const
		{
			check_proxy();
			return m_ptr;
		}

		// Conversion to smart_ptr.
		operator smart_ptr<T>()
		{
			check_proxy();
			return smart_ptr<T>(m_ptr);
		}

		bool	operator==(T* ptr) { check_proxy(); return m_ptr == ptr; }
		bool	operator==(const smart_ptr<T>& ptr) { check_proxy(); return m_ptr == ptr.get(); }

		// for hash< weak_ptr<...>, ...>
		bool	operator==(const weak_ptr<T>& ptr) const
		{
			check_proxy();
			ptr.check_proxy();
			return m_ptr == ptr.m_ptr;
		}

	private:

		void check_proxy() const
			// Set m_ptr to NULL if the object died.
		{
			if (m_ptr)
			{
				if (m_proxy->is_alive() == false)
				{
					// Underlying object went away.
					m_proxy = NULL;
					m_ptr = NULL;
				}
			}
		}

		mutable smart_ptr<weak_proxy>	m_proxy;
		mutable T*	m_ptr;
	};

	// For stuff that's tricky to keep track of w/r/t ownership & cleanup.
	struct ref_counted
	{
		ref_counted() :
			m_ref_count(0),
			m_weak_proxy(0)
		{
		}

		virtual ~ref_counted()
		{
			if (m_weak_proxy)
			{
				m_weak_proxy->notify_object_died();
				m_weak_proxy->drop_ref();
			}
		}

		void	add_ref() const
		{
			m_ref_count++;
		}

		void	drop_ref()
		{
			m_ref_count--;
			if (m_ref_count == 0)
			{
				// Delete me!
				delete this;
			}
		}

		int	get_ref_count() const { return m_ref_count; }
		weak_proxy* get_weak_proxy() const
		{
			if (m_weak_proxy == NULL)
			{
				m_weak_proxy = new weak_proxy;
				m_weak_proxy->add_ref();
			}
			return m_weak_proxy;
		}

	private:
		mutable int	m_ref_count;
		mutable weak_proxy*	m_weak_proxy;
	};

	struct mymutex
	{
		mymutex()
		{
			pthread_mutex_init(&m_mutex, NULL);
		}

		~mymutex()
		{
			pthread_mutex_destroy(&m_mutex);
		}

		void lock()
		{
			pthread_mutex_lock(&m_mutex);
		}

		void unlock()
		{
			pthread_mutex_unlock(&m_mutex);
		}

		pthread_mutex_t m_mutex;
	};

	struct mycondition
	{
		mycondition(): m_retcode(0)
		{
			pthread_cond_init(&m_cond, NULL);
		}

		~mycondition()
		{
			pthread_cond_destroy(&m_cond);
		}

		// Wait on the condition variable cond and unlock the provided mutex.
		// The mutex must the locked before entering this function.
		void wait()
		{
			m_retcode = 0;
			m_cond_mutex.lock();
			int rc = pthread_cond_wait(&m_cond, &m_cond_mutex.m_mutex);
			m_cond_mutex.unlock();

			if (rc != 0)
			{
				// time out or invalid args
				setRetcode(-1);
			}
		}

		// Wait for a while on the condition variable cond and unlock the provided mutex.
		// The mutex must the locked before entering this function.
		void wait(int timeInMs)		// milli seconds
		{
			struct timeval tv;
			struct timespec ts;
			gettimeofday(&tv, NULL);

			ts.tv_sec = time(NULL) + timeInMs / 1000;
			ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000);		// nano
			ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
			ts.tv_nsec %= (1000 * 1000 * 1000);

			m_retcode = 0;
			m_cond_mutex.lock();
			int rc = pthread_cond_timedwait(&m_cond, &m_cond_mutex.m_mutex, &ts);
			m_cond_mutex.unlock();

			if (rc != 0)
			{
				// time out or invalid args
				setRetcode(-1);
			}
		}

		void setRetcode(int val) { m_retcode = val; }
		inline int getRetcode() const { return m_retcode; }

		void signal()
		{
			pthread_cond_signal(&m_cond);
		}

	private:
		pthread_cond_t m_cond;
		mymutex m_cond_mutex;
		int m_retcode;
	};

	// like to autoptr
	struct autolock
	{
		mymutex& m_mutex;
		autolock(mymutex& mutex) : m_mutex(mutex)
		{
			m_mutex.lock();
		}

		~autolock()
		{
			m_mutex.unlock();
		}
	};

	//
	//
	//
	struct aPlayer;
	struct audioData : public ref_counted
	{
		audioData(const char* filename, bool isStream);
		~audioData();

		int getDuration() const { return fDuration; }
		void setDuration(SLmillisecond val) { fDuration = val; }

		bool fIsStream;
		SLDataFormat_PCM fFormat;
		std::vector<U8*> fPCM;
		std::string fPath;
		SLmillisecond fDuration;
		bool fIsReady;
	};

	//
	// aPlayer
	//

	struct aPlayer : public ref_counted
	{
		aPlayer(int channel);
		~aPlayer();

		struct fadeParams
		{
			long long fStartTime;
			int fTime;
			float fFrom;
			float fTo;
		};

		static void onPlayerStreamCallback(SLPlayItf player, void *pContext, SLuint32 event);
		static void onPlayerQueueCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx);

		int clear();
		bool play(SLEngineItf fEngine, SLObjectItf fOutputMixObject, audioData* adata, int number_of_loops, int number_of_milliseconds, PlatformNotifier* callback);
		float getVolume() const;
		bool setVolume(float val, float master);
		int pause();
		int resume();
		bool isPaused();
		bool isPlaying();
		bool seek(int ms);
		bool setMaxVolume(float val);
		bool setMinVolume(float val);
		float getMaxVolume() const { return fMaxVolume; }
		float getMinVolume() const { return fMinVolume; }
		PlatformNotifier* getNotifier() const { return fNotifier; }
		void setNotifier(PlatformNotifier* val)  {	Rtt_DELETE(fNotifier); fNotifier = NULL; }
		void fade();
		int startFading(int fade_ticks, float from, float to);
		long long getTicks() const;
		bool isFree() const { return fData == NULL; }
		bool isUsed() const { return fData != NULL; }
		const char* getPath() const { return fData == NULL ? "" : fData->fPath.c_str(); }
		void playBuffer(SLAndroidSimpleBufferQueueItf queueItf);
		audioData* getData() const { return fData.get(); }
		bool isFinished() const { return fIsFinished; }
		bool startDecoder(SLEngineItf fEngine, audioData* adata);
		bool determineDecodedFormat(SLDataFormat_PCM& ret);
		static void onDecoderBufferDecoded(SLAndroidSimpleBufferQueueItf bq, void* ctx);
		static void onDecoderCompleted(SLPlayItf pitf, void* ctx, SLuint32 event);
		void bufferDecoded(SLAndroidSimpleBufferQueueItf queueItf);
		bool setPitch(float val);
		bool prefetch(audioData* adata);
		int getPosition();

		smart_ptr<audioData> fData;
		bool fPausedBySystem;

		// for stream sound looping
		int fPlayTimer;		
		int fOldPosition;

	private:

		fadeParams fFade;

		SLObjectItf fPlayerObject;
		float fVolume;
		PlatformNotifier* fNotifier;
		float fMaxVolume;
		float fMinVolume;
		float fMaster;
		int fLoops;
		int fChannel;		// for debugging

		volatile int fCurrentBuffer;
		volatile bool fIsFinished;
		float fPitch;
		mymutex fMutex;
		mycondition fPrefetch;
	};

	//
	// PlatformOpenSLESPlayer
	//

	class PlatformOpenSLESPlayer
	{
	public:
		PlatformOpenSLESPlayer();
		virtual ~PlatformOpenSLESPlayer();

		void init(lua_State *L);
		bool createEngine();
		void clearEngine();

		bool LoadAll(const char* file_path);
		bool LoadStream(const char* file_path, unsigned int buffer_size, unsigned int max_queue_buffers, unsigned int number_of_startup_buffers, unsigned int number_of_buffers_to_queue_per_update_pass);
		void FreeData(const char* file_path);

		int FadeInChannelTimed(int which_channel, const char* almixer_data, int number_of_loops, unsigned int fade_ticks, unsigned int expire_ticks, PlatformNotifier* callback);
		unsigned int	CountAllFreeChannels() const;
		unsigned int	CountUnreservedFreeChannels() const;
		unsigned int	CountTotalChannels() const;
		unsigned int	CountAllUsedChannels() const;
		unsigned int	CountUnreservedUsedChannels() const;
		unsigned int	CountReservedChannels() const;
		bool	SetVolumeChannel(int which_channel, float new_volume);
		float	GetVolumeChannel(int which_channel) const;
		bool	SetMasterVolume(float new_volume);
		float	GetMasterVolume() const;
		int	FindFreeChannel(int start_channel) const;
		int FindChannelForDecoder() const;
		int StopChannel(int channel);
		int PauseChannel(int channel);
		int ResumeChannel(int channel);
		void	advance();
		int IsActiveChannel(int which_channel) const;
		int IsPlayingChannel(int which_channel) const;
		int IsPausedChannel(int which_channel) const;
		bool SeekData(const char* almixer_data, unsigned int number_of_milliseconds);
		int	SeekChannel(int which_channel, unsigned int number_of_milliseconds);
		int	RewindChannel(int which_channel);
		int	GetTotalTime(const char* almixer_data);
		bool	SetMaxVolumeChannel(int which_channel, float new_volume);
		float	GetMaxVolumeChannel(int which_channel) const;
		bool	SetMinVolumeChannel(int which_channel, float new_volume);
		float GetMinVolumeChannel(int which_channel) const;
		int	FadeOutChannel(int which_channel, unsigned int fade_ticks);
		int	FadeChannel(int which_channel, unsigned int fade_ticks, float to_volume);
		int ExpireChannel(int which_channel, int number_of_milliseconds);
		int	ReserveChannels(int number_of_reserve_channels);
		void setProperty(int ch, const char* key, float val);

		void SuspendPlayer();
		void ResumePlayer();

		bool createEngineInternal();
		int	getDuration(SLObjectItf fPlayerObject);

		SLObjectItf fOutputMixObject;
		SLEngineItf fEngine;

	private:

		std::map<std::string, smart_ptr<audioData> > fSources;	// decoded sounds
		std::vector<smart_ptr<aPlayer> > fPlayers;	// players

		SLObjectItf fEngineObject;
		float fMasterVolume;
		int fReservedChannels;
		mycondition fPrefetch;
	};

	class AudioTask : public Task
	{
	public:
		AudioTask()	: Task(true) {}

		virtual void operator()(Scheduler& sender);
	};

	PlatformOpenSLESPlayer* getOpenSLES_player();

} // namespace Rtt

#endif // Rtt_USE_OPENSLES
#endif // _Rtt_PlatformOpenSLESPlayer_H__
