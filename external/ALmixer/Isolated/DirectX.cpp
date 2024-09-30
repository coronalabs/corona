// ----------------------------------------------------------------------------
// 
// DirectX.h
// Copyright (c) 2015 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

// Only use DirectX decoders if not using SDL and if compiling for Windows.
#if defined(ALMIXER_COMPILE_WITHOUT_SDL) && (defined(_WIN32) || defined(WINAPI_FAMILY))


// For Win32 desktop apps, we need the ACM APIs for Windows XP.
// Note: Microsoft's ACM APIs are not available to Windows Phone or Windows Universal apps.
#if (defined(_WIN32) && !defined(WINAPI_FAMILY)) || (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
#	define WINDOWS_ACM_SUPPORTED
#endif


// Enable memory leak tracking in Visual Studio if running under debug mode.
#if defined( Rtt_DEBUG ) || defined( _DEBUG )
#	ifndef _CRTDBG_MAP_ALLOC
#		define _CRTDBG_MAP_ALLOC
#	endif
#	include <stdlib.h>
#	include <crtdbg.h>
#endif


#if HAVE_CONFIG_H
#	include <config.h>
#endif
#include "SoundDecoder.h"
#include "SoundDecoder_Internal.h"
#include <sys\stat.h>
#include <atomic>
#include <exception>
#include <io.h>
#include <mutex>
#include <Objidl.h>
#include <string>
#include <thread>
#include <unordered_set>
#include <Windows.h>

// Include headers needed by Windows ACM APIs, if enabled.
// For Win32 desktop apps, we fallback to these APIs if the DirectX Media Foundation is not available/installed.
#ifdef WINDOWS_ACM_SUPPORTED
#	include <Mmreg.h>
#	include <MSAcm.h>
#	include <Wmsdkidl.h>
#endif

// Include headers needed by DirectX Media Foundation.
// Note: This module was designed to link to the Media Foundation libraries dynamically since not all
//       Windows operating systems provide them, such as Windows XP, Vista, 7N, 7NK, 8N, 8NK, etc.
//       It's okay to include these headers, but do not use the exported members so that the compiler
//       won't link attempt to link them in. We'll dynamically fetch the library's exported functions
//       via LoadLibrary() and GetProcAddress() at runtime instead.
#ifndef INITGUID
//	This needs to be defined and included before including the "mfapi.h" header file.
//  It allows us to use the Media Foundation GUIDs without having to link in its lib file.
#	define INITGUID
#	include <Guiddef.h>
#endif
#include <mfapi.h>
#include <Mfreadwrite.h>
#include <Propvarutil.h>


// Microsoft Visual C++ macro which allows us to easily do bitwise operations on the given enum like in C.
DEFINE_ENUM_FLAG_OPERATORS(SoundDecoder_SampleFlags)


#pragma region Private Member Variables
/// <summary>Mutex used to synchronize access to this module's static variables between threads.</summary>
static std::recursive_mutex sMutex;

/// <summary>Stores a set of each thread ID this module has initialized Microsoft COM on.</summary>
static std::unordered_set<std::thread::id> sComInitializedThreadIdSet; // This is a false positive (several, rather) for memory leaks:
																		// the report occurs before static deinitialization / DLL detach

/// <summary>
///  <para>Set true if the DirectX Media Foundation's MFStartup() function was called.</para>
///  <para>Expected to be initialized in this module's init() function, if the DirectX library is available.</para>
/// </summary>
static bool sWasMediaFoundationInitialized = false;

/// <summary>
///  <para>Incremented everytime this module's init() function is called.</para>
///  <para>Decremented when this module's quit() function is called.</para>
/// </summary>
static int sModuleInitializationCount = 0;

/// <summary>
///  <para>Set true if the DirectX Media Foundation is available on the system. False if not available.</para>
///  <para>
///   Set false if not available which can happen on Windows XP, Vista, 7N, 7NK, 8N, and 8NK systems.
///   Althought end-users can install it on Vista and newer systems, but not XP.
///  </para>
///  <para>This module's private MFDecoder class should only be used if the Media Foundation is available.</para>
/// </summary>
static bool sIsMediaFoundationAvailable = false;

/// <summary>Defines a callback type for DirectX Media Foundation function: MFStartup()</summary>
typedef HRESULT(STDAPICALLTYPE *MFStartupCallback)(ULONG, DWORD);
static MFStartupCallback sMFStartupCallback = nullptr;

/// <summary>Defines a callback type for DirectX Media Foundation function: MFShutdown()</summary>
typedef HRESULT(STDAPICALLTYPE *MFShutdownCallback)();
static MFShutdownCallback sMFShutdownCallback = nullptr;

/// <summary>Defines a callback type for DirectX Media Foundation function: MFCreateAsyncResult()</summary>
typedef HRESULT(STDAPICALLTYPE *MFCreateAsyncResultCallback)(IUnknown*, IMFAsyncCallback*, IUnknown*, IMFAsyncResult**);
static MFCreateAsyncResultCallback sMFCreateAsyncResultCallback = nullptr;

/// <summary>Defines a callback type for DirectX Media Foundation function: MFCreateSourceReaderFromByteStream()</summary>
typedef HRESULT(STDAPICALLTYPE *MFCreateSourceReaderFromByteStreamCallback)(_In_ IMFByteStream*, _In_ IMFAttributes*, _Out_ IMFSourceReader**);
static MFCreateSourceReaderFromByteStreamCallback sMFCreateSourceReaderFromByteStreamCallback = nullptr;

/// <summary>Defines a callback type for DirectX Media Foundation function: MFCreateMediaType()</summary>
typedef HRESULT(STDAPICALLTYPE *MFCreateMediaTypeCallback)(IMFMediaType**);
static MFCreateMediaTypeCallback sMFCreateMediaTypeCallback = nullptr;

/// <summary>Defines a callback type for "propsys.dll" function: PropVariantToInt64()</summary>
typedef HRESULT(STDAPICALLTYPE *PropVariantToInt64Callback)(_In_ REFPROPVARIANT, _Out_ LONGLONG*);
static PropVariantToInt64Callback sPropVariantToInt64Callback = nullptr;

#ifdef WINDOWS_ACM_SUPPORTED
/// <summary>
///  <para>Set true if Microsoft's ACM related libraries are available on the system. False if not.</para>
///  <para>Will likely be false on the N and NK editions of Windows.</para>
///  <para>This module's private AcmDecoder class should only be used if ACM is available.</para>
/// </summary>
static bool sIsAcmAvailable = false;

/// <summary>Defines a callback type for "wmvcore.dll" function: WMCreateSyncReader()</summary>
typedef HRESULT(STDMETHODCALLTYPE *WMCreateSyncReaderCallback)(IUnknown*, DWORD, IWMSyncReader**);
static WMCreateSyncReaderCallback sWMCreateSyncReaderCallback = nullptr;

#endif //WINDOWS_ACM_SUPPORTED

#pragma endregion


#pragma region Private RWopsStream Struct
namespace
{
	/// <summary>Win32 IStream interface to an ALmixer RWops file interface.</summary>
	/// <remarks>
	///  WARNING! The declaration order of public and protected members matters when deriving from a COM interface.
	///  The COM interface members must be defined at the top of this class, in the same order that they are defined
	///  in their respective COM interfaces, and ordered from base class to child class.
	///  This class' public member must be defined after the COM interface members.
	/// </remarks>
	struct RWopsStream : public IStream
	{
		public:
			#pragma region IUnknown Methods
			virtual HRESULT STDMETHODCALLTYPE QueryInterface(
						REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
			virtual ULONG STDMETHODCALLTYPE AddRef(void);
			virtual ULONG STDMETHODCALLTYPE Release(void);

			#pragma endregion


			#pragma region ISequentialStream Methods
			virtual HRESULT STDMETHODCALLTYPE Read(
						__out_bcount_part(cb, *pcbRead) void *pv, ULONG cb, __out_opt ULONG *pcbRead);
			virtual HRESULT STDMETHODCALLTYPE Write(__in_bcount(cb) const void *pv, ULONG cb, __out_opt  ULONG *pcbWritten);

			#pragma endregion


			#pragma region IStream Methods
			virtual HRESULT STDMETHODCALLTYPE Seek(
						LARGE_INTEGER dlibMove, DWORD dwOrigin, __out_opt ULARGE_INTEGER *plibNewPosition);
			virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize);
			virtual HRESULT STDMETHODCALLTYPE CopyTo(
						IStream *pstm, ULARGE_INTEGER cb, __out_opt  ULARGE_INTEGER *pcbRead,
						__out_opt ULARGE_INTEGER *pcbWritten);
			virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags);
			virtual HRESULT STDMETHODCALLTYPE Revert(void);
			virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
			virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
			virtual HRESULT STDMETHODCALLTYPE Stat(__RPC__out STATSTG *pstatstg, DWORD grfStatFlag);
			virtual HRESULT STDMETHODCALLTYPE Clone(__RPC__deref_out_opt IStream **ppstm);

			#pragma endregion


			#pragma region Public Static Functions
			static RWopsStream* CreateWith(ALmixer_RWops* rwOpsPointer);

			#pragma endregion

		private:
			#pragma region Private Constructors/Destructors
			RWopsStream(ALmixer_RWops* rwOpsPointer);
			RWopsStream(const RWopsStream& stream);
			virtual ~RWopsStream();

			#pragma endregion


			#pragma region Private Methods
			void operator=(const RWopsStream& stream);

			#pragma endregion


			#pragma region Private Member Variables
			ALmixer_RWops* fRWopsPointer;
			bool fDoesOwnRWopsObject;
			ULONG fReferenceCount;

			#pragma endregion
	};

	RWopsStream::RWopsStream(ALmixer_RWops* rwOpsPointer)
	:	fRWopsPointer(rwOpsPointer),
		fDoesOwnRWopsObject(false),
		fReferenceCount(1)
	{
	}

	RWopsStream::RWopsStream(const RWopsStream& stream)
	{
		// Copy constructor is not supported.
		throw std::exception();
	}

	RWopsStream::~RWopsStream()
	{
		if (fDoesOwnRWopsObject && fRWopsPointer)
		{
			fRWopsPointer->close(fRWopsPointer);
		}
	}

	RWopsStream* RWopsStream::CreateWith(ALmixer_RWops* rwOpsPointer)
	{
		if (!rwOpsPointer)
		{
			return nullptr;
		}
		return new RWopsStream(rwOpsPointer);
	}

	void RWopsStream::operator=(const RWopsStream& stream)
	{
		// Copy operator is not supported.
		throw std::exception();
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::QueryInterface(
		REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		// Validate argument.
		if (!ppvObject)
		{
			return E_POINTER;
		}

		// If the given interface ID matches this object's interface, then return this object.
		if ((__uuidof(IUnknown) == riid) || (__uuidof(ISequentialStream) == riid) || (__uuidof(IStream) == riid))
		{
			*ppvObject = this;
			this->AddRef();
			return S_OK;
		}

		// Given interface ID was not recognized.
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE RWopsStream::AddRef(void)
	{
		fReferenceCount++;
		return fReferenceCount;
	}

	ULONG STDMETHODCALLTYPE RWopsStream::Release(void)
	{
		// Decrement the reference count.
		fReferenceCount--;

		// Delete this object if this reference count is now zero.
		ULONG currentCount = fReferenceCount;
		if (currentCount <= 0)
		{
			delete this;
		}

		// Return the current reference count.
		return currentCount;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::Read(
		__out_bcount_part(cb, *pcbRead) void *pv, ULONG cb, __out_opt ULONG *pcbRead)
	{
		// Validate arguments.
		if (!pv || !pcbRead)
		{
			return STG_E_INVALIDPOINTER;
		}

		// Read the requested number of bytes to the given buffer.
		auto bytesRead = ALmixer_RWread(fRWopsPointer, pv, 1, cb);
		*pcbRead = bytesRead;

		// Return S_OK if we've read the exact number of requested bytes requested.
		// Return S_FALSE if we've read a different number of bytes, which can happen when reaching the end of the file.
		return (bytesRead == cb) ? S_OK : S_FALSE;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::Write(
		__in_bcount(cb) const void *pv, ULONG cb, __out_opt  ULONG *pcbWritten)
	{
		// Validate.
		if (!pv)
		{
			return STG_E_INVALIDPOINTER;
		}

		// Do not continue if given zero bytes to write.
		if (cb <= 0)
		{
			return S_OK;
		}

		// Write the given bytes.
		auto bytesWritten = fRWopsPointer->write(fRWopsPointer, pv, 1, cb);
		if (pcbWritten)
		{
			*pcbWritten = bytesWritten;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::Seek(
		LARGE_INTEGER dlibMove, DWORD dwOrigin, __out_opt ULARGE_INTEGER *plibNewPosition)
	{
		long offset = (long)(long long)dlibMove.QuadPart;
		auto currentPosition = ALmixer_RWseek(fRWopsPointer, offset, dwOrigin);
		return (currentPosition >= 0) ? S_OK : STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::SetSize(ULARGE_INTEGER libNewSize)
	{
		// Re-allocating the size of the RWops' source is not supported.
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::CopyTo(
		IStream *pstm, ULARGE_INTEGER cb, __out_opt  ULARGE_INTEGER *pcbRead, __out_opt ULARGE_INTEGER *pcbWritten)
	{
		// Not currently supported.
		return E_FAIL;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::Commit(DWORD grfCommitFlags)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::Revert(void)
	{
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return STG_E_INVALIDFUNCTION;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::Stat(__RPC__out STATSTG *pstatstg, DWORD grfStatFlag)
	{
		// Validate arguments.
		if (!pstatstg)
		{
			return STG_E_INVALIDPOINTER;
		}

		// Initialize the given "stat" object.
		memset(pstatstg, 0, sizeof(STATSTG));
		pstatstg->type = STGTY_STREAM;
		pstatstg->clsid = CLSID_NULL;

		// If we're streaming from a file, then copy its "stat" information to the given one.
		FILE* filePointer = fRWopsPointer->hidden.stdio.fp;
		if (filePointer)
		{
			struct _stat64 fileStatus;
			int result = _fstat64(_fileno(filePointer), &fileStatus);
			if (0 == result)
			{
				LARGE_INTEGER int64Converter;
				pstatstg->cbSize.QuadPart = fileStatus.st_size;
				int64Converter.QuadPart = fileStatus.st_mtime;
				pstatstg->mtime.dwHighDateTime = int64Converter.HighPart;
				pstatstg->mtime.dwLowDateTime = int64Converter.LowPart;
				int64Converter.QuadPart = fileStatus.st_ctime;
				pstatstg->ctime.dwHighDateTime = int64Converter.HighPart;
				pstatstg->ctime.dwLowDateTime = int64Converter.LowPart;
				int64Converter.QuadPart = fileStatus.st_atime;
				pstatstg->atime.dwHighDateTime = int64Converter.HighPart;
				pstatstg->atime.dwLowDateTime = int64Converter.LowPart;
				pstatstg->grfMode = fileStatus.st_mode;
			}
		}

		// Returns a success result.
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsStream::Clone(__RPC__deref_out_opt IStream **ppstm)
	{
		// Validate.
		if (!ppstm)
		{
			return STG_E_INVALIDPOINTER;
		}

		// Fetch the file pointer belonging to the RWops object.
		// Note: We're not supposed to assume RWops always references a file, even though it always does at the moment.
		FILE* filePointer = fRWopsPointer->hidden.stdio.fp;
		if (!filePointer)
		{
			return STG_E_INVALIDPOINTER;
		}

		// Open a new file stream to this object's file stream.
		int fileDescriptor = _fileno(filePointer);
		int clonedFileDescriptor = _dup(fileDescriptor);
		if (-1 == clonedFileDescriptor)
		{
			return STG_E_INSUFFICIENTMEMORY;
		}
		FILE* clonedFilePointer = _fdopen(clonedFileDescriptor, "rb");
		if (!clonedFilePointer)
		{
			return STG_E_INSUFFICIENTMEMORY;
		}
		auto clonedRWopsPointer = ALmixer_RWFromFP(clonedFilePointer, 1);
		if (!clonedRWopsPointer)
		{
			fclose(clonedFilePointer);
		}

		// Clone this IStream object and return it via the given argument.
		auto clonedStream = new RWopsStream(clonedRWopsPointer);
		clonedStream->fDoesOwnRWopsObject = true;
		*ppstm = clonedStream;
		return S_OK;
	}
}

#pragma endregion


#pragma region Private RWopsMFByteStream Struct
namespace
{
	/// <summary>DirectX Media Foundation "IMFByteStream" interface to an ALmixer RWops file interface.</summary>
	/// <remarks>
	///  WARNING! The declaration order of public and protected members matters when deriving from a COM interface.
	///  The COM interface members must be defined at the top of this class, in the same order that they are defined
	///  in their respective COM interfaces, and ordered from base class to child class.
	///  This class' public member must be defined after the COM interface members.
	/// </remarks>
	struct RWopsMFByteStream : public IMFByteStream
	{
		public:
			#pragma region IUnknown Methods
			virtual HRESULT STDMETHODCALLTYPE QueryInterface(
						REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
			virtual ULONG STDMETHODCALLTYPE AddRef(void);
			virtual ULONG STDMETHODCALLTYPE Release(void);

			#pragma endregion


			#pragma region IMFByteStream Methods
			virtual HRESULT STDMETHODCALLTYPE GetCapabilities(__RPC__out DWORD *pdwCapabilities);
			virtual HRESULT STDMETHODCALLTYPE GetLength(__RPC__out QWORD *pqwLength);
			virtual HRESULT STDMETHODCALLTYPE SetLength(QWORD qwLength);
			virtual HRESULT STDMETHODCALLTYPE GetCurrentPosition(__RPC__out QWORD *pqwPosition);
			virtual HRESULT STDMETHODCALLTYPE SetCurrentPosition(QWORD qwPosition);
			virtual HRESULT STDMETHODCALLTYPE IsEndOfStream(__RPC__out BOOL *pfEndOfStream);
			virtual HRESULT STDMETHODCALLTYPE Read(
						__out_bcount_part(cb, *pcbRead)  BYTE *pb, ULONG cb, __out  ULONG *pcbRead);
			virtual HRESULT STDMETHODCALLTYPE BeginRead(
						__out_bcount(cb)  BYTE *pb, ULONG cb, IMFAsyncCallback *pCallback, IUnknown *punkState);
			virtual HRESULT STDMETHODCALLTYPE EndRead(IMFAsyncResult *pResult, __out  ULONG *pcbRead);
			virtual HRESULT STDMETHODCALLTYPE Write(
						__in_bcount(cb)  const BYTE *pb, ULONG cb, __out __deref_out_range(<= , cb) ULONG *pcbWritten);
			virtual HRESULT STDMETHODCALLTYPE BeginWrite(
						__in_bcount(cb)  const BYTE *pb, ULONG cb, IMFAsyncCallback *pCallback, IUnknown *punkState);
			virtual HRESULT STDMETHODCALLTYPE EndWrite(IMFAsyncResult *pResult, __out  ULONG *pcbWritten);
			virtual HRESULT STDMETHODCALLTYPE Seek(
						MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset,
						DWORD dwSeekFlags, __out_opt  QWORD *pqwCurrentPosition);
			virtual HRESULT STDMETHODCALLTYPE Flush(void);
			virtual HRESULT STDMETHODCALLTYPE Close(void);

			#pragma endregion


			#pragma region Public Static Functions
			static RWopsMFByteStream* CreateWith(ALmixer_RWops* rwOpsPointer);

			#pragma endregion

		private:
			#pragma region Private Constructors/Destructors
			RWopsMFByteStream(ALmixer_RWops* rwOpsPointer);
			RWopsMFByteStream(const RWopsMFByteStream& stream);
			virtual ~RWopsMFByteStream();

			#pragma endregion


			#pragma region Private Methods
			void operator=(const RWopsMFByteStream& stream);

			#pragma endregion


			#pragma region Private Member Variables
			ALmixer_RWops* fRWopsPointer;
			bool fDoesOwnRWopsObject;
			ULONG fReferenceCount;
			long long fByteLength;
			long fAsyncBytesRead;
			std::atomic_bool fIsAsyncTaskExecuting;

			#pragma endregion
	};

	RWopsMFByteStream::RWopsMFByteStream(ALmixer_RWops* rwOpsPointer)
	:	fRWopsPointer(rwOpsPointer),
		fDoesOwnRWopsObject(false),
		fReferenceCount(1),
		fByteLength(-1),
		fAsyncBytesRead(-1)
	{
		fIsAsyncTaskExecuting = false;
	}

	RWopsMFByteStream::RWopsMFByteStream(const RWopsMFByteStream& stream)
	{
		// Copy constructor is not supported.
		throw std::exception();
	}

	RWopsMFByteStream::~RWopsMFByteStream()
	{
		if (fDoesOwnRWopsObject && fRWopsPointer)
		{
			fRWopsPointer->close(fRWopsPointer);
		}
	}

	RWopsMFByteStream* RWopsMFByteStream::CreateWith(ALmixer_RWops* rwOpsPointer)
	{
		// Validate argument.
		if (!rwOpsPointer)
		{
			return nullptr;
		}

		// Do not continue if DirectX Media Foundation is not available on the system.
		if (!sIsMediaFoundationAvailable)
		{
			return nullptr;
		}

		// Return a new Media Foundation bye stream interface wrapping the given ALmixer RWops streaming interface.
		return new RWopsMFByteStream(rwOpsPointer);
	}

	void RWopsMFByteStream::operator=(const RWopsMFByteStream& stream)
	{
		// Copy operator is not supported.
		throw std::exception();
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::QueryInterface(
		REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		// Validate argument.
		if (!ppvObject)
		{
			return E_POINTER;
		}

		// If the given interface ID matches this object's interface, then return this object.
		if ((__uuidof(IUnknown) == riid) || (__uuidof(IMFByteStream) == riid))
		{
			*ppvObject = this;
			this->AddRef();
			return S_OK;
		}

		// Given interface ID was not recognized.
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE RWopsMFByteStream::AddRef(void)
	{
		fReferenceCount++;
		return fReferenceCount;
	}

	ULONG STDMETHODCALLTYPE RWopsMFByteStream::Release(void)
	{
		// Decrement the reference count.
		fReferenceCount--;

		// Delete this object if this reference count is now zero.
		ULONG currentCount = fReferenceCount;
		if (currentCount <= 0)
		{
			delete this;
		}

		// Return the current reference count.
		return currentCount;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::GetCapabilities(__RPC__out DWORD *pdwCapabilities)
	{
		// Validate.
		if (!pdwCapabilities)
		{
			return E_INVALIDARG;
		}

		// Set flags indicating what the ALmixer RWops streaming interface is capable of.
		const DWORD kDoesNotUseNetwork = 0x800; // = MFBYTESTREAM_DOES_NOT_USE_NETWORK on Windows 8.
		*pdwCapabilities =
				MFBYTESTREAM_IS_READABLE | MFBYTESTREAM_IS_WRITABLE | MFBYTESTREAM_IS_SEEKABLE | kDoesNotUseNetwork;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::GetLength(__RPC__out QWORD *pqwLength)
	{
		// Validate.
		if (!pqwLength)
		{
			return E_INVALIDARG;
		}

		// Fetch the RWops stream's byte length, if not done already.
		if (fByteLength < 0)
		{
			// For best performance, first attempt to fetch the stream size via file APIs.
			FILE* filePointer = fRWopsPointer->hidden.stdio.fp;
			if (filePointer)
			{
				struct _stat64 fileStatus;
				int result = _fstat64(_fileno(filePointer), &fileStatus);
				if (0 == result)
				{
					fByteLength = fileStatus.st_size;
				}
			}

			// If RWops does not reference a file, then attempt to find the byte length via the RWops streaming APIs.
			if (fByteLength < 0)
			{
				auto lastPosition = ALmixer_RWtell(fRWopsPointer);
				if (lastPosition >= 0)
				{
					// Seek to the end of the stream to determine the byte length.
					auto streamEndPosition = ALmixer_RWseek(fRWopsPointer, 0, SEEK_END);
					if (streamEndPosition >= 0)
					{
						fByteLength = streamEndPosition + 1;
					}

					// Seek back to the previous position.
					ALmixer_RWseek(fRWopsPointer, lastPosition, SEEK_SET);
				}
			}
		}

		// Return the stream byte length via the given mutable argument.
		// Set to -1 if the byte length is unknown.
		*pqwLength = (fByteLength >= 0) ? (QWORD)fByteLength : -1;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::SetLength(QWORD qwLength)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::GetCurrentPosition(__RPC__out QWORD *pqwPosition)
	{
		// Validate.
		if (!pqwPosition)
		{
			return E_INVALIDARG;
		}

		// Fetch the current stream's position in bytes.
		auto position = ALmixer_RWtell(fRWopsPointer);
		if (position < 0)
		{
			return E_FAIL;
		}
		*pqwPosition = position;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::SetCurrentPosition(QWORD qwPosition)
	{
		auto newPosition = ALmixer_RWseek(fRWopsPointer, (long)qwPosition, SEEK_SET);
		if ((newPosition < 0) || ((QWORD)newPosition != qwPosition))
		{
			return E_FAIL;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::IsEndOfStream(__RPC__out BOOL *pfEndOfStream)
	{
		// Validate.
		if (!pfEndOfStream)
		{
			return E_INVALIDARG;
		}

		// Determine if we've reached the end of the stream.
		QWORD streamLength = 0;
		this->GetLength(&streamLength);
		if (-1 == streamLength)
		{
			*pfEndOfStream = FALSE;
		}
		else
		{
			auto currentPosition = ALmixer_RWtell(fRWopsPointer);
			*pfEndOfStream = (((QWORD)currentPosition + 1) >= streamLength) ? TRUE : FALSE;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::Read(
		__out_bcount_part(cb, *pcbRead)  BYTE *pb, ULONG cb, __out  ULONG *pcbRead)
	{
		// Validate.
		if (!pcbRead)
		{
			return E_INVALIDARG;
		}

		// Do not continue if given a null or zero length buffer.
		if (!pb || (cb <= 0))
		{
			*pcbRead = 0;
			return S_OK;
		}

		// Read the requested number of bytes to the given buffer.
		auto bytesRead = ALmixer_RWread(fRWopsPointer, pb, 1, cb);
		*pcbRead = bytesRead;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::BeginRead(
		__out_bcount(cb) BYTE *pb, ULONG cb, IMFAsyncCallback *pCallback, IUnknown *punkState)
	{
		// Validate.
		if (!pb || !pCallback)
		{
			return E_INVALIDARG;
		}

		// Do not continue if we're already in the middle of an async operation.
		if (fIsAsyncTaskExecuting.load())
		{
			return E_UNEXPECTED;
		}

		// Create the result object to be used by the async operation below.
		// Note: This must be created on the calling thread or else an exception will occur.
		IMFAsyncResult* asyncResultPointer = nullptr;
		HRESULT result = sMFCreateAsyncResultCallback(this, pCallback, punkState, &asyncResultPointer);
		if (FAILED(result))
		{
			return E_FAIL;
		}

		// Read the requested number of bytes to the given buffer.
		// Note: This is cheating. This method call is supposed to do an async read, but it's really blocking.
		//       We do this to avoid handling the case where multiple threads want to operate on the same stream.
		//       However, for this module's purposes, this is okay since ALmixer already threads out streamed decoding.
		fAsyncBytesRead = -1;
		ULONG bytesRead = 0;
		result = this->Read(pb, cb, &bytesRead);
		if (SUCCEEDED(result))
		{
			fAsyncBytesRead = (long)bytesRead;
		}
		else
		{
			asyncResultPointer->SetStatus(result);
		}

		// Notify the caller about the read results via the given callback.
		// Note: This callback must be invoked on another thread because some of Microsoft's decoders will crash
		//       (such as MKV decoder "mfmkvsrcsnk.dll") if invoked from this method.
		fIsAsyncTaskExecuting = true;
		auto streamPointer = this;
		streamPointer->AddRef();
		std::thread asyncTask([streamPointer, pCallback, asyncResultPointer]()
		{
			try
			{
				pCallback->Invoke(asyncResultPointer);
			}
			catch (...) {}
			asyncResultPointer->Release();
			streamPointer->Release();
		});
		asyncTask.detach();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::EndRead(IMFAsyncResult *pResult, __out  ULONG *pcbRead)
	{
		// Do not continue if the given result object was not generated by this object's BeginRead() method.
		IUnknown* objectPointer = nullptr;
		pResult->GetObject(&objectPointer);
		bool isResultObjectValid = (objectPointer == this);
		if (objectPointer)
		{
			objectPointer->Release();
		}
		if (!isResultObjectValid)
		{
			return E_INVALIDARG;
		}

		// Fetch the number of bytes read by the BeginRead() method.
		auto bytesRead = fAsyncBytesRead;
		fAsyncBytesRead = -1;

		// Flag that the async operation has ended.
		// We must do this before returning because the caller might call BeginRead() immediately afterwards.
		fIsAsyncTaskExecuting = false;

		// Return the number of bytes read via the given mutable argument.
		if (pcbRead)
		{
			*pcbRead = (ULONG)bytesRead;
		}
		return (bytesRead >= 0) ? S_OK : E_FAIL;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::Write(
		__in_bcount(cb)  const BYTE *pb, ULONG cb, __out __deref_out_range(<= , cb) ULONG *pcbWritten)
	{
		// Validate.
		if (!pb)
		{
			return E_POINTER;
		}

		// Write the given bytes out to the RWops stream.
		size_t bytesWritten = 0;
		if (cb > 0)
		{
			bytesWritten = fRWopsPointer->write(fRWopsPointer, pb, 1, cb);
		}

		// Return the number of bytes written via the following mutable argument.
		if (pcbWritten)
		{
			*pcbWritten = bytesWritten;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::BeginWrite(
		__in_bcount(cb)  const BYTE *pb, ULONG cb, IMFAsyncCallback *pCallback, IUnknown *punkState)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::EndWrite(IMFAsyncResult *pResult, __out  ULONG *pcbWritten)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::Seek(
		MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset,
		DWORD dwSeekFlags, __out_opt  QWORD *pqwCurrentPosition)
	{
		// Seek to the given position.
		int origin = (msoBegin == SeekOrigin) ? SEEK_SET : SEEK_CUR;
		auto currentPosition = ALmixer_RWseek(fRWopsPointer, (long)llSeekOffset, origin);
		if (currentPosition < 0)
		{
			return E_FAIL;
		}

		// Return the current stream position via the following mutable argument.
		if (pqwCurrentPosition)
		{
			*pqwCurrentPosition = (QWORD)currentPosition;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::Flush(void)
	{
		FILE* filePointer = fRWopsPointer->hidden.stdio.fp;
		if (filePointer)
		{
			fflush(filePointer);
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE RWopsMFByteStream::Close(void)
	{
		// Do not close the file if this streaming interface doesn't own it.
		if (false == fDoesOwnRWopsObject)
		{
			return S_FALSE;
		}

		// Close the file.
		auto result = fRWopsPointer->close(fRWopsPointer);
		fDoesOwnRWopsObject = false;
		return (0 == result) ? S_OK : E_FAIL;
	}
}

#pragma endregion


#pragma region Private BaseDecoder Class
namespace
{
	/// <summary>Base class that this module's "AcmDecoder" and "MFDecoder" classes derive from.</summary>
	class BaseDecoder
	{
		public:
			/// <summary>Creates a new audio decoder.</summary>
			BaseDecoder();

			/// <summary>Destroys this decoder's allocated resources. Does not close the file it was given.</summary>
			virtual ~BaseDecoder();

			/// <summary>Determines if this can decoder can decode the given file stream.</summary>
			/// <returns>
			///  <para>Returns true if this decoder can decode audio data from the given file stream.</para>
			///  <para>
			///   Returns false if audio data cannot be decoded, in which case, the ReadUsing() and Seek()
			///   methods will do nothing.
			///  </para>
			/// </returns>
			virtual bool CanDecode() const = 0;

			/// <summary>Determines if the referenced audio file is seekable.</summary>
			/// <returns>
			///  <para>Returns true if the audio file is seekable. This means the Seek() method is supported.</para>
			///  <para>Returns false if the reference file is not seekable.</para>
			/// </returns>
			virtual bool CanSeek() const = 0;

			/// <summary>
			///  <para>Gets information about the PCM audio format this decoder will decode to.</para>
			///  <para>Should not be called if the CanDecode() method returns false.</para>
			/// </summary>
			/// <returns>
			///  <para>
			///   Returns the format of the raw PCM audio format this decoder will decode the reference audio file to.
			///  </para>
			///  <para>Returns a struct initialize to all zeros if the reference file cannot be decoded.</para>
			/// </returns>
			virtual SoundDecoder_AudioInfo GetOutputInfo() const = 0;

			/// <summary>Gets the duration of the referenced audio file in milliseconds.</summary>
			/// <returns>
			///  <para>Returns the duration of the reference audio file in milliseconds.</para>
			///  <para>Returns zero if the referenced audio file cannot be decoded.</para>
			/// </returns>
			virtual int GetDurationInMiliseconds() const = 0;

			/// <summary>
			///  Decodes the referenced audio file's data and copies the decoded raw PCM data to the given
			///  sample struct's buffer.
			/// </summary>
			/// <param name="samplePointer">Provides a reference to the RWops file stream. Cannot be null.</param>
			/// <returns>
			///  <para>Returns the number of decoded bytes copied into the given sample struct's buffer.</para>
			///  <para>
			///   Returns zero if there is no more data to decode, if the reference file cannot be decoded,
			///   or if given an invalid argument.
			///  </para>
			/// </returns>
			virtual int ReadUsing(Sound_Sample* samplePointer) = 0;

			/// <summary>Seeks to the given time in the audio file</summary>
			/// <param name="samplePointer">Provides a reference to the RWops file stream. Cannot be null.</param>
			/// <param name="timeInMilliseconds">
			///  <para>The time in milliseconds to seek to.</para>
			///  <para>Set to zero to rewind back to the beginning of the audio file.</para>
			/// </param>
			/// <returns>
			///  <para>Returns true if successfully seeked to the given time.</para>
			///  <para>Returns false if unable to seek the reference audio file or if given invalid arguments.</para>
			/// </returns>
			virtual bool Seek(Sound_Sample* samplePointer, size_t timeInMilliseconds) = 0;
	};

	BaseDecoder::BaseDecoder()
	{
	}

	BaseDecoder::~BaseDecoder()
	{
	}
}

#pragma endregion


#pragma region Private AcmDecoder Class
#ifdef WINDOWS_ACM_SUPPORTED
namespace
{
	/// <summary>Decodes one MP3 file via Microsoft's ACM Win32 APIs.</summary>
	/// <remarks>
	///  This is the most backward compatible audio decoder for Win32 desktop apps and should be used as a
	///  fallback mechanism in case the DirectX Media Foundation was not found on the system.
	/// </remarks>
	class AcmDecoder : public BaseDecoder
	{
		public:
			/// <summary>Creates a new audio decoder which uses Microsoft's ACM Win32 APIs.</summary>
			/// <param name="samplePointer">Provides the audio file to be decoded via the RWops stream.</param>
			AcmDecoder(Sound_Sample* samplePointer);

			/// <summary>Destroys this decoder's allocated resources. Does not close the file it was given.</summary>
			virtual ~AcmDecoder();

			/// <summary>Determines if this can decoder can decode the given file stream.</summary>
			/// <returns>
			///  <para>Returns true if this decoder can decode audio data from the given file stream.</para>
			///  <para>
			///   Returns false if audio data cannot be decoded, in which case, the ReadUsing() and Seek()
			///   methods will do nothing.
			///  </para>
			/// </returns>
			virtual bool CanDecode() const;

			/// <summary>Determines if the referenced audio file is seekable.</summary>
			/// <returns>
			///  <para>Returns true if the audio file is seekable. This means the Seek() method is supported.</para>
			///  <para>Returns false if the reference file is not seekable.</para>
			/// </returns>
			virtual bool CanSeek() const;

			/// <summary>
			///  <para>Gets information about the PCM audio format this decoder will decode to.</para>
			///  <para>Should not be called if the CanDecode() method returns false.</para>
			/// </summary>
			/// <returns>
			///  <para>
			///   Returns the format of the raw PCM audio format this decoder will decode the reference audio file to.
			///  </para>
			///  <para>Returns a struct initialize to all zeros if the reference file cannot be decoded.</para>
			/// </returns>
			virtual SoundDecoder_AudioInfo GetOutputInfo() const;

			/// <summary>Gets the duration of the referenced audio file in milliseconds.</summary>
			/// <returns>
			///  <para>Returns the duration of the reference audio file in milliseconds.</para>
			///  <para>Returns zero if the referenced audio file cannot be decoded.</para>
			/// </returns>
			virtual int GetDurationInMiliseconds() const;

			/// <summary>
			///  Decodes the referenced audio file's data and copies the decoded raw PCM data to the given
			///  sample struct's buffer.
			/// </summary>
			/// <param name="samplePointer">Provides a reference to the RWops file stream. Cannot be null.</param>
			/// <returns>
			///  <para>Returns the number of decoded bytes copied into the given sample struct's buffer.</para>
			///  <para>
			///   Returns zero if there is no more data to decode, if the reference file cannot be decoded,
			///   or if given an invalid argument.
			///  </para>
			/// </returns>
			virtual int ReadUsing(Sound_Sample* samplePointer);

			/// <summary>Seeks to the given time in the audio file</summary>
			/// <param name="samplePointer">Provides a reference to the RWops file stream. Cannot be null.</param>
			/// <param name="timeInMilliseconds">
			///  <para>The time in milliseconds to seek to.</para>
			///  <para>Set to zero to rewind back to the beginning of the audio file.</para>
			/// </param>
			/// <returns>
			///  <para>Returns true if successfully seeked to the given time.</para>
			///  <para>Returns false if unable to seek the reference audio file or if given invalid arguments.</para>
			/// </returns>
			virtual bool Seek(Sound_Sample* samplePointer, size_t timeInMilliseconds);

		private:
			/// <summary>Mutex used to synchronize access between threads.</summary>
			std::recursive_mutex fMutex;

			/// <summary>
			///  <para>Handle to the ACM stream used to decode the given audio file.</para>
			///  <para>Will be null if unable to decode the given audio file.</para>
			/// </summary>
			HACMSTREAM fAcmStreamHandle;

			/// <summary>ACM header file used in the audio decoding/conversion process.</summary>
			ACMSTREAMHEADER fAcmStreamHeader;

			/// <summary>
			///  Pointer to a struct created by this class defining the audio format of the encoded audio file.
			/// </summary>
			LPWAVEFORMATEX fInputFormatPointer;

			/// <summary>Defines the raw PCM audio format to decode the audio file to.</summary>
			WAVEFORMATEX fOutputFormat;

			/// <summary>Input buffer used to store the encoded bytes copied from audio file for conversion.</summary>
			LPBYTE fInputBufferPointer;

			/// <summary>
			///  Buffer used to store the decoded audio data in PCM form extracted from "fInputBufferPointer".
			/// </summary>
			LPBYTE fOutputBufferPointer;

			/// <summary>Number of bytes member variable "fInputBufferPointer" provides.</summary>
			size_t fInputBufferSize;

			/// <summary>Number of bytes member variable "fOutputBufferPointer" provides.</summary>
			size_t fOutputBufferSize;

			/// <summary>
			///  <para>Number of decoded bytes remaing in "fOutputBufferPointer" from the last call to ReadUsing().</para>
			///  <para>Needed if we've decoded more bytes than can fit in ALmixer's buffer.</para>
			///  <para>
			///   The remaining bytes are expected to be copied to ALmixer's buffer on the next call to ReadUsing().
			///  </para>
			/// </summary>
			size_t fLastDecodedBytesRemaining;

			/// <summary>How long the audio file is in 100 nanosecond units.</summary>
			LONGLONG fDurationIn100NanosecondUnits;
	};

	AcmDecoder::AcmDecoder(Sound_Sample* samplePointer)
	:	BaseDecoder(),
		fAcmStreamHandle(nullptr),
		fInputFormatPointer(nullptr),
		fInputBufferPointer(nullptr),
		fOutputBufferPointer(nullptr),
		fInputBufferSize(0),
		fOutputBufferSize(0),
		fLastDecodedBytesRemaining(0),
		fDurationIn100NanosecondUnits(0)
	{
		// Initialize member variables before doing the below.
		memset(&fOutputFormat, 0, sizeof(fOutputFormat));
		fOutputFormat.wFormatTag = WAVE_FORMAT_PCM;
		memset(&fAcmStreamHeader, 0, sizeof(fAcmStreamHeader));
		fAcmStreamHeader.cbStruct = sizeof(fAcmStreamHeader);

		// Validate argument.
		if (!samplePointer)
		{
			return;
		}

		// Do not continue if Windows ACM is not available on the system.
		if (!sIsAcmAvailable || !sWMCreateSyncReaderCallback)
		{
			return;
		}

		// Fetch the internal sample struct.
		auto sampleInternalPointer = (Sound_SampleInternal*)samplePointer->opaque;
		if (!sampleInternalPointer || !(sampleInternalPointer->rw))
		{
			return;
		}

		// Attempt to read the given audio file's format information.
		auto streamPointer = RWopsStream::CreateWith(sampleInternalPointer->rw);
		IWMSyncReader* syncReaderPointer = nullptr;
		sWMCreateSyncReaderCallback(nullptr, WMT_RIGHT_PLAYBACK, &syncReaderPointer);
		if (syncReaderPointer)
		{
			IWMProfile* profilePointer = nullptr;
			syncReaderPointer->OpenStream(streamPointer);
			syncReaderPointer->QueryInterface(&profilePointer);
			if (profilePointer)
			{
				// Fetch the audio file's WAVEFORMATEX info.
				IWMStreamConfig* streamConfigPointer = nullptr;
				profilePointer->GetStream(0, &streamConfigPointer);
				if (streamConfigPointer)
				{
					IWMMediaProps* mediaPropertiesPointer = nullptr;
					streamConfigPointer->QueryInterface(&mediaPropertiesPointer);
					if (mediaPropertiesPointer)
					{
						DWORD mediaTypeSize = 0;
						mediaPropertiesPointer->GetMediaType(nullptr, &mediaTypeSize);
						if (mediaTypeSize > 0)
						{
							auto mediaTypePointer = (WM_MEDIA_TYPE*)malloc(mediaTypeSize);
							mediaPropertiesPointer->GetMediaType(mediaTypePointer, &mediaTypeSize);
							if (WMMEDIATYPE_Audio == mediaTypePointer->majortype)
							{
								auto waveFormatPointer = (WAVEFORMATEX*)mediaTypePointer->pbFormat;
								if (waveFormatPointer)
								{
									// Create a copy of the audio file's format information.
									// Note: The received format might not actually be a WAVEFORMATEX struct, but more
									//       likely another type of struct such as MPEGLAYER3WAVEFORMAT which contains
									//       a WAVEFORMATEX as its first member. This is how Microsoft fakes inheritance
									//       in C. The "wFormatTag" indicates which struct this pointer should be casted
									//       and the "cbSize" indicates how many additional bytes more the actual struct
									//       consumes compared to a WAVEFORMATEX struct.
									const auto kFormatSize = sizeof(WAVEFORMATEX) + waveFormatPointer->cbSize;
									fInputFormatPointer = (WAVEFORMATEX*)malloc(kFormatSize);
									memcpy(fInputFormatPointer, waveFormatPointer, kFormatSize);
								}
							}
							free(mediaTypePointer);
						}
						mediaPropertiesPointer->Release();
					}
					streamConfigPointer->Release();
				}
				profilePointer->Release();
			}
			if (fInputFormatPointer)
			{
				// Fetch the audio duration.
				IWMHeaderInfo* headerInfoPointer = nullptr;
				syncReaderPointer->QueryInterface(&headerInfoPointer);
				if (headerInfoPointer)
				{
					WMT_ATTR_DATATYPE attributeDataType;
					WORD streamIndex = 0;
					WORD valueByteLength = 0;
					HRESULT result = headerInfoPointer->GetAttributeByName(
								&streamIndex, ::g_wszWMDuration, &attributeDataType, nullptr, &valueByteLength);
					if (SUCCEEDED(result) && (8 == valueByteLength))
					{
						LONGLONG duration = 0;
						result = headerInfoPointer->GetAttributeByName(
									&streamIndex, ::g_wszWMDuration, &attributeDataType,
									(BYTE*)&duration, &valueByteLength);
						if (SUCCEEDED(result) && (duration >= 0))
						{
							fDurationIn100NanosecondUnits = duration;
						}
					}
					headerInfoPointer->Release();
				}
			}
			syncReaderPointer->Release();
		}
		streamPointer->Release();
		sampleInternalPointer->rw->seek(sampleInternalPointer->rw, 0, SEEK_SET);

		// Give up if we could not obtain the given file's audio format information.
		if (!fInputFormatPointer || (fDurationIn100NanosecondUnits <= 0))
		{
			return;
		}

		// Finish setting up the PCM output format to decode the given audio to.
		fOutputFormat.nChannels = (fInputFormatPointer->nChannels > 0) ? fInputFormatPointer->nChannels : 1;
		fOutputFormat.nSamplesPerSec = fInputFormatPointer->nSamplesPerSec;
		fOutputFormat.nSamplesPerSec = (fInputFormatPointer->nSamplesPerSec > 0) ? fInputFormatPointer->nSamplesPerSec : 44100;
		if (fInputFormatPointer->wBitsPerSample > 8)
		{
			fOutputFormat.wBitsPerSample = 16;
		}
		else if (fInputFormatPointer->wBitsPerSample > 0)
		{
			fOutputFormat.wBitsPerSample = 8;
		}
		else
		{
			fOutputFormat.wBitsPerSample = 16;
		}
		{
			// Microsoft's documentation states this is the formula you use for PCM formats.
			fOutputFormat.nBlockAlign = (fOutputFormat.nChannels * fOutputFormat.wBitsPerSample) / 8;
		}
		{
			// Microsoft's documentation states this is the formula you use for PCM formats.
			fOutputFormat.nAvgBytesPerSec = fOutputFormat.nSamplesPerSec * fOutputFormat.nBlockAlign;
		}

		// Open the audio conversion stream.
		auto openResult = ::acmStreamOpen(
				&fAcmStreamHandle, nullptr, fInputFormatPointer, &fOutputFormat, nullptr, 0, 0, 0);
		if (!fAcmStreamHandle)
		{
			return;
		}

		// Create a buffer used to store a batch of encoded audio data.
		fInputBufferSize = 1024;
#if 0
		if (WAVE_FORMAT_MPEGLAYER3 == fInputFormatPointer->wFormatTag)
		{
			auto mp3FormatPointer = (MPEGLAYER3WAVEFORMAT*)fInputFormatPointer;
			if (mp3FormatPointer->nBlockSize > 0)
			{
				fInputBufferSize = mp3FormatPointer->nBlockSize;
			}
		}
#endif
		fInputBufferPointer = (BYTE*)malloc(fInputBufferSize);

		// Create a buffer used to store a batch of decoded audio data.
		DWORD outputBufferSize = 0;
		::acmStreamSize(fAcmStreamHandle, fInputBufferSize, &outputBufferSize, ACM_STREAMSIZEF_SOURCE);
		if (outputBufferSize > 0)
		{
			fOutputBufferSize = outputBufferSize;
			fOutputBufferPointer = (BYTE*)malloc(fOutputBufferSize);
		}

		// Set up an ACM header to use above encoded/decoded buffers for conversion later.
		if (fInputBufferPointer && fOutputBufferPointer)
		{
			fAcmStreamHeader.pbSrc = fInputBufferPointer;
			fAcmStreamHeader.cbSrcLength = fInputBufferSize;
			fAcmStreamHeader.pbDst = fOutputBufferPointer;
			fAcmStreamHeader.cbDstLength = fOutputBufferSize;
			auto prepareResult = ::acmStreamPrepareHeader(fAcmStreamHandle, &fAcmStreamHeader, 0);
			if (prepareResult != 0)
			{
				fAcmStreamHeader.pbSrc = nullptr;
				fAcmStreamHeader.pbDst = nullptr;
			}
		}

		// If we've failed to set up the ACM header's input and output buffers, then free up the memory they use.
		if (fInputBufferPointer && !fAcmStreamHeader.pbSrc)
		{
			free(fInputBufferPointer);
			fInputBufferPointer = nullptr;
			fInputBufferSize = 0;
		}
		if (fOutputBufferPointer && !fAcmStreamHeader.pbDst)
		{
			free(fOutputBufferPointer);
			fOutputBufferPointer = nullptr;
			fOutputBufferSize = 0;
		}
	}

	AcmDecoder::~AcmDecoder()
	{
		if (fAcmStreamHandle && fInputBufferPointer && fOutputBufferPointer)
		{
			::acmStreamUnprepareHeader(fAcmStreamHandle, &fAcmStreamHeader, 0);
		}
		if (fInputBufferPointer)
		{
			free(fInputBufferPointer);
			fInputBufferPointer = nullptr;
			fInputBufferSize = 0;
		}
		if (fOutputBufferPointer)
		{
			free(fOutputBufferPointer);
			fOutputBufferPointer = nullptr;
			fOutputBufferSize = 0;
		}
		if (fInputFormatPointer)
		{
			free(fInputFormatPointer);
			fInputFormatPointer = nullptr;
		}
		if (fAcmStreamHandle)
		{
			::acmStreamClose(fAcmStreamHandle, 0);
			fAcmStreamHandle = nullptr;
		}
	}

	bool AcmDecoder::CanDecode() const
	{
		return (fAcmStreamHandle && fInputBufferPointer && fOutputBufferPointer);
	}

	bool AcmDecoder::CanSeek() const
	{
		return CanDecode();
	}

	SoundDecoder_AudioInfo AcmDecoder::GetOutputInfo() const
	{
		SoundDecoder_AudioInfo info{};
		if (this->CanDecode())
		{
			info.channels = (ALubyte)fOutputFormat.nChannels;
			info.rate = (ALuint)fOutputFormat.nSamplesPerSec;
			info.format = (16 == fOutputFormat.wBitsPerSample) ? AUDIO_S16 : AUDIO_U8;
		}
		return info;
	}

	int AcmDecoder::GetDurationInMiliseconds() const
	{
		int durationInMilliseconds = 0;
		if (fDurationIn100NanosecondUnits > 0)
		{
			double fractionalMilliseconds = (double)fDurationIn100NanosecondUnits / 10000.0;
			durationInMilliseconds = (int)(fractionalMilliseconds + 0.5);
		}
		return durationInMilliseconds;
	}

	int AcmDecoder::ReadUsing(Sound_Sample* samplePointer)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

		// Validate argument.
		if (!samplePointer)
		{
			return 0;
		}

		// Do not continue if unable to decode the referenced audio file.
		if (!CanDecode())
		{
			samplePointer->flags |= SOUND_SAMPLEFLAG_ERROR;
			return 0;
		}

		// Fetch the internal sample struct and make sure it is valid.
		auto sampleInternalPointer = (Sound_SampleInternal*)samplePointer->opaque;
		if (!sampleInternalPointer || !(sampleInternalPointer->rw) ||
		    !(sampleInternalPointer->buffer) || (sampleInternalPointer->buffer_size <= 0))
		{
			samplePointer->flags |= SOUND_SAMPLEFLAG_ERROR;
			return 0;
		}

		// Decode the source audio buffer to raw PCM/Wave and copy it to the given ALmixer buffer.
		size_t totalBytesCopied = 0;
		if (fLastDecodedBytesRemaining > 0)
		{
			// We've decoded too many bytes in the last call to this function.
			// Copy this data to the ALmixer buffer now that it has room for more.
			int outputBufferOffset = (int)fAcmStreamHeader.cbDstLengthUsed - (int)fLastDecodedBytesRemaining;
			if (outputBufferOffset < 0)
			{
				outputBufferOffset = 0;
				fLastDecodedBytesRemaining = fAcmStreamHeader.cbDstLengthUsed;
			}
			size_t bytesToCopy = fLastDecodedBytesRemaining;
			if (bytesToCopy > sampleInternalPointer->buffer_size)
			{
				bytesToCopy = sampleInternalPointer->buffer_size;
			}
			auto alMixerBufferPointer = (BYTE*)sampleInternalPointer->buffer;
			memcpy(alMixerBufferPointer, fOutputBufferPointer + outputBufferOffset, bytesToCopy);
			totalBytesCopied += bytesToCopy;
			fLastDecodedBytesRemaining -= bytesToCopy;
		}
		while (totalBytesCopied < sampleInternalPointer->buffer_size)
		{
			// Extract the next batch of encoded audio data from the audio file.
			size_t bytesRead = ALmixer_RWread(sampleInternalPointer->rw, fInputBufferPointer, 1, fInputBufferSize);
			if (bytesRead > 0)
			{
				// Decode the next batch of audio data.
				auto conversionResult =
						::acmStreamConvert(fAcmStreamHandle, &fAcmStreamHeader, ACM_STREAMCONVERTF_BLOCKALIGN);
				if (conversionResult != 0)
				{
					samplePointer->flags |= SOUND_SAMPLEFLAG_ERROR;
					break;
				}

				// Copy the decoded raw PCM audio data to ALmixer's buffer.
				// Note: Will be zero here if we're streaming through the audio file's header/meta-data section.
				size_t bytesToCopy = fAcmStreamHeader.cbDstLengthUsed;
				if (bytesToCopy > 0)
				{
					if ((bytesToCopy + totalBytesCopied) > sampleInternalPointer->buffer_size)
					{
						// We've decoded more bytes than can fit into the given ALmixer buffer.
						// Store the remaining decoded bytes to be copied on the ReadUsing() call.
						bytesToCopy = sampleInternalPointer->buffer_size - totalBytesCopied;
						fLastDecodedBytesRemaining = fAcmStreamHeader.cbDstLengthUsed - bytesToCopy;
					}
					auto alMixerBufferPointer = (BYTE*)sampleInternalPointer->buffer;
					memcpy(alMixerBufferPointer + totalBytesCopied, fOutputBufferPointer, bytesToCopy);
					totalBytesCopied += bytesToCopy;
				}
			}

			// If the bytes read from file is less than expected, then we've reached the end of the file.
			if (bytesRead < fInputBufferSize)
			{
				// Only notify the caller that we've reached the end of the file if we have no more bytes remaining.
				// This way the caller will call this function again to fetch the remaining bytes.
				if (fLastDecodedBytesRemaining <= 0)
				{
					samplePointer->flags |= SOUND_SAMPLEFLAG_EOF;
				}
				break;
			}
		}

		// Return the number of decoded PCM bytes copied to ALmixer's buffer.
		return (int)totalBytesCopied;
	}

	bool AcmDecoder::Seek(Sound_Sample* samplePointer, size_t timeInMilliseconds)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

		// Validate.
		if (!CanDecode() || !samplePointer)
		{
			return false;
		}

		// Fetch the internal sample struct and validate it.
		auto sampleInternalPointer = (Sound_SampleInternal*)samplePointer->opaque;
		if (!sampleInternalPointer || !(sampleInternalPointer->rw))
		{
			return false;
		}

		// Seek to the given point in time.
		bool hasSucceeded = false;
		if (timeInMilliseconds <= 0)
		{
			// Rewind to the beginning of the audio file.
			ALmixer_RWseek(sampleInternalPointer->rw, 0, SEEK_SET);
			hasSucceeded = true;
		}
		else if (timeInMilliseconds >= (size_t)this->GetDurationInMiliseconds())
		{
			// Seek to the end of the audio file.
			hasSucceeded = (ALmixer_RWseek(sampleInternalPointer->rw, 0, SEEK_END) >= 0);
		}
		else
		{
			// Decode the audio file from the beginning to find the given time via its decoded PCM data.
			// Note: This is a very slow way to do this, but it will do for now.
			ALmixer_RWseek(sampleInternalPointer->rw, 0, SEEK_SET);
			size_t totalBytesRead = 0;
			size_t bytesPerSample = fOutputFormat.wBitsPerSample / 8;
			double totalTimeInFractionalMilliseconds = 0;
			double timeInFractionMilliseconds = (double)timeInMilliseconds;
			double samplesPerFractionalMillisecond = (double)fOutputFormat.nSamplesPerSec / 1000.0;
			if (bytesPerSample && (samplesPerFractionalMillisecond > 0))
			{
				while (true)
				{
					// Read the next batch of encoded bytes from the audio file.
					size_t bytesRead = ALmixer_RWread(sampleInternalPointer->rw, fInputBufferPointer, 1, fInputBufferSize);
					if (bytesRead > 0)
					{
						// Decode the read bytes.
						auto conversionResult =
								::acmStreamConvert(fAcmStreamHandle, &fAcmStreamHeader, ACM_STREAMCONVERTF_BLOCKALIGN);
						if (conversionResult != 0)
						{
							ALmixer_RWseek(sampleInternalPointer->rw, 0, SEEK_SET);
							break;
						}

						// Check if we've reached the part of the encoded file for the given decoded PCM time.
						size_t samplesRead = fAcmStreamHeader.cbDstLengthUsed / bytesPerSample;
						double timeInFractionalMillisecondsRead = (double)samplesRead / samplesPerFractionalMillisecond;
						totalTimeInFractionalMilliseconds += timeInFractionalMillisecondsRead;
						if (totalTimeInFractionalMilliseconds > timeInFractionMilliseconds)
						{
							// Found it! Now seek backwards in the encoded file that we think will closely match the time.
							double deltaTimeInFractionMilliseconds =
									totalTimeInFractionalMilliseconds - timeInFractionMilliseconds;
							double scale = deltaTimeInFractionMilliseconds / timeInFractionalMillisecondsRead;
							long seekOffset = -(long)((double)bytesRead * scale);
							ALmixer_RWseek(sampleInternalPointer->rw, seekOffset, SEEK_CUR);
							hasSucceeded = true;
							break;
						}
					}

					// If we've reached the end of the file, then assume the given time is at the end of the file.
					if (bytesRead < fInputBufferSize)
					{
						hasSucceeded = true;
						break;
					}
				}
			}
		}

		// Ignore any remaining decoded bytes from the last read so that it won't be copied on the next read.
		fLastDecodedBytesRemaining = 0;

		// Returns true if the seek operation successeeded. Returns false if failed.
		return hasSucceeded;
	}
}

#endif
#pragma endregion


#pragma region Private MFDecoder Class
namespace
{
	/// <summary>
	///  <para>Decodes one audio file via Microsoft's DirectX Media Foundation library.</para>
	///  <para>Supports 3GP, AAC, MP3, WAV, and whatever other codecs that might be installed for this library.</para>
	/// </summary>
	class MFDecoder : public BaseDecoder
	{
		public:
			/// <summary>Creates a new audio decoder which using Microsoft's DirecgX Media Foundation library.</summary>
			/// <param name="samplePointer">Provides the audio file to be decoded via the RWops stream.</param>
			MFDecoder(Sound_Sample* samplePointer);

			/// <summary>Destroys this decoder's allocated resources. Does not close the file it was given.</summary>
			virtual ~MFDecoder();

			/// <summary>Determines if this can decoder can decode the given file stream.</summary>
			/// <returns>
			///  <para>Returns true if this decoder can decode audio data from the given file stream.</para>
			///  <para>
			///   Returns false if audio data cannot be decoded, in which case, the ReadUsing() and Seek()
			///   methods will do nothing.
			///  </para>
			/// </returns>
			virtual bool CanDecode() const;

			/// <summary>Determines if the referenced audio file is seekable.</summary>
			/// <returns>
			///  <para>Returns true if the audio file is seekable. This means the Seek() method is supported.</para>
			///  <para>Returns false if the reference file is not seekable.</para>
			/// </returns>
			virtual bool CanSeek() const;

			/// <summary>
			///  <para>Gets information about the PCM audio format this decoder will decode to.</para>
			///  <para>Should not be called if the CanDecode() method returns false.</para>
			/// </summary>
			/// <returns>
			///  <para>
			///   Returns the format of the raw PCM audio format this decoder will decode the reference audio file to.
			///  </para>
			///  <para>Returns a struct initialize to all zeros if the reference file cannot be decoded.</para>
			/// </returns>
			virtual SoundDecoder_AudioInfo GetOutputInfo() const;

			/// <summary>Gets the duration of the referenced audio file in milliseconds.</summary>
			/// <returns>
			///  <para>Returns the duration of the reference audio file in milliseconds.</para>
			///  <para>Returns zero if the referenced audio file cannot be decoded.</para>
			/// </returns>
			virtual int GetDurationInMiliseconds() const;

			/// <summary>
			///  Decodes the referenced audio file's data and copies the decoded raw PCM data to the given
			///  sample struct's buffer.
			/// </summary>
			/// <param name="samplePointer">Provides a reference to the RWops file stream. Cannot be null.</param>
			/// <returns>
			///  <para>Returns the number of decoded bytes copied into the given sample struct's buffer.</para>
			///  <para>
			///   Returns zero if there is no more data to decode, if the reference file cannot be decoded,
			///   or if given an invalid argument.
			///  </para>
			/// </returns>
			virtual int ReadUsing(Sound_Sample* samplePointer);

			/// <summary>Seeks to the given time in the audio file</summary>
			/// <param name="samplePointer">Provides a reference to the RWops file stream. Cannot be null.</param>
			/// <param name="timeInMilliseconds">
			///  <para>The time in milliseconds to seek to.</para>
			///  <para>Set to zero to rewind back to the beginning of the audio file.</para>
			/// </param>
			/// <returns>
			///  <para>Returns true if successfully seeked to the given time.</para>
			///  <para>Returns false if unable to seek the reference audio file or if given invalid arguments.</para>
			/// </returns>
			virtual bool Seek(Sound_Sample* samplePointer, size_t timeInMilliseconds);

		private:
			/// <summary>
			///  <para>
			///   Copies raw decoded PCM data from member variable "fLastDecodedSamplePointer" to the given buffer.
			///  </para>
			///  <para>This method is only expected to be called by this class' ReadUsing() method.</para>
			///  <para>
			///   After calling this method, this method will set member variable "fLastDecodedBytesRemaining"
			///   if there are any remaining bytes in "fLastDecodedSamplePointer" that were not copied to the given buffer.
			///  </para>
			/// </summary>
			/// <param name="bufferPointer">Buffer to copy decoded audio data to.</param>
			/// <param name="bufferSize">The size of argument "bufferPointer" in bytes.</param>
			/// <returns>
			///  <para>Returns the number of bytes copied into argument "bufferPointer".</para>
			///  <para>Returns zero if no data was available or if given invalid arguments.</para>
			/// </returns>
			int CopyDecodedSamplesTo(BYTE* bufferPointer, size_t bufferSize);


			/// <summary>Mutex used to synchronize access between threads.</summary>
			std::recursive_mutex fMutex;

			/// <summary>DirectX Media Foundation stream interface which wraps an ALmixer RWops stream interface.</summary>
			RWopsMFByteStream* fByteStreamPointer;

			/// <summary>
			///  <para>
			///   DirectX Media Foundation audio reader needed to decode the audio data reference by "fByteStreamPointer".
			///  </para>
			///  <para>Set null if the reference file/stream cannot be decoded.</para>
			/// </summary>
			IMFSourceReader* fReaderPointer;

			/// <summary>
			///  <para>Defines the raw PCM output format that the DirectX Media Foundation will decode to.</para>
			///  <para>Set null if the reference file/stream cannot be decoded.</para>
			/// </summary>
			IMFMediaType* fOutputMediaTypePointer;

			/// <summary>Pointer to the last batch of decoded audio acquired via the ReadUsing() method.</summary>
			IMFSample* fLastDecodedSamplePointer;

			/// <summary>
			///  <para>
			///   Number of decoded bytes remaining in member variable "fLastDecodedSamplePointer" from the
			///   last call to the ReadUsing() method.
			///  </para>
			///  <para>Set to zero if there are no decoded bytes remaining or no bytes have been decoded.</para>
			/// </summary>
			size_t fLastDecodedBytesRemaining;
	};

	MFDecoder::MFDecoder(Sound_Sample* samplePointer)
	:	BaseDecoder(),
		fByteStreamPointer(nullptr),
		fReaderPointer(nullptr),
		fOutputMediaTypePointer(nullptr),
		fLastDecodedSamplePointer(nullptr),
		fLastDecodedBytesRemaining(0)
	{
		// Validate argument.
		if (!samplePointer)
		{
			return;
		}

		// Do not continue if DirecX Media Foundation is not available on the system.
		if (!sIsMediaFoundationAvailable)
		{
			return;
		}

		// Fetch the internal sample struct.
		auto sampleInternalPointer = (Sound_SampleInternal*)samplePointer->opaque;
		if (!sampleInternalPointer || !(sampleInternalPointer->rw))
		{
			return;
		}

		// Create an audio file stream reader that wraps the ALmixer RWops stream interface.
		fByteStreamPointer = RWopsMFByteStream::CreateWith(sampleInternalPointer->rw);
		if (!fByteStreamPointer)
		{
			return;
		}
		HRESULT result = sMFCreateSourceReaderFromByteStreamCallback(fByteStreamPointer, nullptr, &fReaderPointer);
		if (!fReaderPointer)
		{
			return;
		}

		// Select the first audio stream in the file. Ignore all other streams.
		result = fReaderPointer->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
		result = fReaderPointer->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
		if (FAILED(result))
		{
			return;
		}

		// Set up the reader to decode the source audio to raw PCM audio.
		IMFMediaType* pcmMediaTypePointer = nullptr;
		result = sMFCreateMediaTypeCallback(&pcmMediaTypePointer);
		if (pcmMediaTypePointer)
		{
			result = pcmMediaTypePointer->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			result = pcmMediaTypePointer->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
			result = fReaderPointer->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pcmMediaTypePointer);
			if (SUCCEEDED(result))
			{
				// Now fetch a fully configured PCM output media type based on the partial one that was set up above.
				result = fReaderPointer->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &fOutputMediaTypePointer);
			}
		}
		if (!fOutputMediaTypePointer)
		{
			// Failed to set up and acquire an output media type.
			if (pcmMediaTypePointer)
			{
				pcmMediaTypePointer->Release();
			}
			return;
		}

		// Ensure the first audio stream is still selected.
		// Microsoft's examples threaten that it can change after doing the above.
		result = fReaderPointer->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
	}

	MFDecoder::~MFDecoder()
	{
		if (fLastDecodedSamplePointer)
		{
			fLastDecodedSamplePointer->Release();
			fLastDecodedSamplePointer = nullptr;
			fLastDecodedBytesRemaining = 0;
		}
		if (fOutputMediaTypePointer)
		{
			fOutputMediaTypePointer->Release();
			fOutputMediaTypePointer = nullptr;
		}
		if (fReaderPointer)
		{
			fReaderPointer->Release();
			fReaderPointer = nullptr;
		}
		if (fByteStreamPointer)
		{
			fByteStreamPointer->Release();
			fByteStreamPointer = nullptr;
		}
	}

	bool MFDecoder::CanDecode() const
	{
		return (fReaderPointer && fOutputMediaTypePointer);
	}

	bool MFDecoder::CanSeek() const
	{
		return CanDecode();
	}

	SoundDecoder_AudioInfo MFDecoder::GetOutputInfo() const
	{
		SoundDecoder_AudioInfo info{};
		if (this->CanDecode())
		{
			info.channels = (ALubyte)::MFGetAttributeUINT32(fOutputMediaTypePointer, MF_MT_AUDIO_NUM_CHANNELS, 1);
			info.rate = (ALuint)::MFGetAttributeUINT32(fOutputMediaTypePointer, MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
			auto bitsPerSample = ::MFGetAttributeUINT32(fOutputMediaTypePointer, MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
			info.format = (16 == bitsPerSample) ? AUDIO_S16LSB : AUDIO_U8;
		}
		return info;
	}

	int MFDecoder::GetDurationInMiliseconds() const
	{
		int durationInMilliseconds = 0;
		if (this->CanDecode() && fReaderPointer)
		{
			PROPVARIANT variantValue;
			HRESULT result = fReaderPointer->GetPresentationAttribute(
						MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &variantValue);
			if (SUCCEEDED(result))
			{
				LONGLONG durationIn100NanosecondUnits = 0;
				result = sPropVariantToInt64Callback(variantValue, &durationIn100NanosecondUnits);
				if (SUCCEEDED(result))
				{
					double fractionalMilliseconds = (double)durationIn100NanosecondUnits / 10000.0;
					durationInMilliseconds = (int)(fractionalMilliseconds + 0.5);
					::PropVariantClear(&variantValue);
				}
			}
		}
		return durationInMilliseconds;
	}

	int MFDecoder::ReadUsing(Sound_Sample* samplePointer)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

		// Validate argument.
		if (!samplePointer)
		{
			return 0;
		}

		// Do not continue if unable to decode the referenced audio file.
		if (!CanDecode())
		{
			samplePointer->flags |= SOUND_SAMPLEFLAG_ERROR;
			return 0;
		}

		// Fetch the internal sample struct and make sure it is valid.
		auto sampleInternalPointer = (Sound_SampleInternal*)samplePointer->opaque;
		if (!sampleInternalPointer || !(sampleInternalPointer->rw) ||
		    !(sampleInternalPointer->buffer) || (sampleInternalPointer->buffer_size <= 0))
		{
			samplePointer->flags |= SOUND_SAMPLEFLAG_ERROR;
			return 0;
		}

		// Decode the source audio buffer to raw PCM/Wave and copy it to the given ALmixer buffer.
		int totalBytesCopied = 0;
		while ((size_t)totalBytesCopied < sampleInternalPointer->buffer_size)
		{
			// Decode the next batch of audio data if we do not have any remaining from the last call.
			DWORD statusFlags = 0;
			if (!fLastDecodedSamplePointer)
			{
				HRESULT result = fReaderPointer->ReadSample(
							MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &statusFlags,
							nullptr, &fLastDecodedSamplePointer);
				if (FAILED(result) || (statusFlags & MF_SOURCE_READERF_ERROR))
				{
					samplePointer->flags |= SOUND_SAMPLEFLAG_ERROR;
					break;
				}
			}

			// Copy the next batch of decoded audio data to ALmixer's buffer.
			if (fLastDecodedSamplePointer)
			{
				auto alMixerBufferPointer = (BYTE*)sampleInternalPointer->buffer;
				totalBytesCopied += CopyDecodedSamplesTo(
						alMixerBufferPointer + totalBytesCopied, sampleInternalPointer->buffer_size - totalBytesCopied);
				if (fLastDecodedBytesRemaining <= 0)
				{
					fLastDecodedSamplePointer->Release();
					fLastDecodedSamplePointer = nullptr;
				}
			}

			// Do not continue if we've reached the end of the audio file.
			if (statusFlags & MF_SOURCE_READERF_ENDOFSTREAM)
			{
				// Only notify the caller that we've reached the end of the file if we have no more bytes remaining.
				// This way the caller will call this method again to fetch the remaining bytes.
				if (fLastDecodedBytesRemaining <= 0)
				{
					samplePointer->flags |= SOUND_SAMPLEFLAG_EOF;
				}
				break;
			}
		}

		return totalBytesCopied;
	}

	bool MFDecoder::Seek(Sound_Sample* samplePointer, size_t timeInMilliseconds)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

		// Validate.
		if (!CanDecode() || !samplePointer)
		{
			return false;
		}

		// Convert the given time into 100 nanosecond units and store it to a variant type.
		LONGLONG timeIn100NanosecondUnits = (LONGLONG)timeInMilliseconds * 10000LL;
		PROPVARIANT variantValue;
		HRESULT result = ::InitPropVariantFromInt64(timeIn100NanosecondUnits, &variantValue);
		if (FAILED(result))
		{
			return false;
		}

		// Seek to the given point in time.
		result = fReaderPointer->SetCurrentPosition(GUID_NULL, variantValue);
		if (FAILED(result))
		{
			return false;
		}

		// Clear any remaining decoded audio data so that it won't be copied into the next ReadUsing() method call.
		if (fLastDecodedSamplePointer)
		{
			fLastDecodedSamplePointer->Release();
			fLastDecodedSamplePointer = nullptr;
			fLastDecodedBytesRemaining = 0;
		}

		// Return true to indicate that we've successfully seeked to the given position.
		return true;
	}

	int MFDecoder::CopyDecodedSamplesTo(BYTE* bufferPointer, size_t bufferSize)
	{
		// Validate.
		if (!fLastDecodedSamplePointer || !bufferPointer || (bufferSize <= 0))
		{
			fLastDecodedBytesRemaining = 0;
			return 0;
		}

		// Fetch how many bytes are in all of the sample's buffers.
		DWORD totalSourceBytes = 0;
		fLastDecodedSamplePointer->GetTotalLength(&totalSourceBytes);
		if (totalSourceBytes <= 0)
		{
			fLastDecodedBytesRemaining = 0;
			return 0;
		}

		// If we decoded too many bytes than we could copy in the last call to this function,
		// then calculate where we should begin copying the remaining decoded bytes.
		size_t sourceByteStartIndex = 0;
		if (fLastDecodedBytesRemaining > 0)
		{
			if (fLastDecodedBytesRemaining > totalSourceBytes)
			{
				fLastDecodedBytesRemaining = totalSourceBytes;
			}
			sourceByteStartIndex = totalSourceBytes - fLastDecodedBytesRemaining;
		}

		// Copy the sample's decoded bytes to the given buffer.
		int totalBytesCopied = 0;
		int totalBytesIterated = 0;
		DWORD sourceBufferCount = 0;
		fLastDecodedSamplePointer->GetBufferCount(&sourceBufferCount);
		for (DWORD sourceBufferIndex = 0; sourceBufferIndex < sourceBufferCount; sourceBufferIndex++)
		{
			// Fetch the next buffer interface from the decoded sample.
			IMFMediaBuffer* mediaBufferPointer = nullptr;
			fLastDecodedSamplePointer->GetBufferByIndex(sourceBufferIndex, &mediaBufferPointer);
			if (!mediaBufferPointer)
			{
				break;
			}

			// Fetch the buffer interface's raw byte buffer.
			BYTE* sourceBufferPointer = nullptr;
			DWORD sourceBufferSize = 0;
			mediaBufferPointer->Lock(&sourceBufferPointer, nullptr, &sourceBufferSize);
			if (sourceBufferPointer)
			{
				// Copy the decoded raw PCM audio data to the given buffer.
				if ((sourceBufferSize > 0) && (sourceByteStartIndex < (totalBytesIterated + sourceBufferSize)))
				{
					// If we have any remaining bytes to copy from the last copy, then figure out where we left off.
					int sourceByteOffset = (int)sourceByteStartIndex - totalBytesIterated;
					if (sourceByteOffset < 0)
					{
						sourceByteOffset = 0;
					}

					// Determine how many bytes we can copy into the given buffer.
					size_t bytesToCopy = sourceBufferSize - sourceByteOffset;
					if ((bytesToCopy + totalBytesCopied) > bufferSize)
					{
						// We've decoded more bytes than can fit in the given buffer.
						bytesToCopy -= (bytesToCopy + (size_t)totalBytesCopied) - bufferSize;
					}

					// Copy the bytes.
					memcpy(bufferPointer + totalBytesCopied, sourceBufferPointer + sourceByteOffset, bytesToCopy);
					totalBytesCopied += bytesToCopy;
				}
				totalBytesIterated += sourceBufferSize;

				// Release the buffer interface's raw byte buffer.
				mediaBufferPointer->Unlock();
			}

			// Release the decoded's sample buffer interface.
			mediaBufferPointer->Release();

			// Do not continue if the given buffer is full.
			if ((size_t)totalBytesCopied >= bufferSize)
			{
				break;
			}
		}

		// If we have not copied over all of the decoded bytes, then store the number of remaining bytes.
		// We'll use this to copy the remaining bytes the next time this method gets called.
		int bytesRemaining = (int)totalSourceBytes - (totalBytesCopied + (int)sourceByteStartIndex);
		fLastDecodedBytesRemaining = (bytesRemaining > 0) ? (size_t)bytesRemaining : 0;

		// Return the total number of decoded bytes in "fLastDecodedSamplePointer" that was copied to the given buffer.
		return totalBytesCopied;
	}
}
#pragma endregion


#pragma region Private Functions
/// <summary>
///  Private helper function which fetches a C function callback from the given library via
///  the Win32 GetProcAddress() function.
/// </summary>
/// <param name="moduleHandle">Handle to the module to load the C function from.</param>
/// <param name="functionName">Name of the C function to load the given library.</param>
/// <param name="callback">Pointer to the callback that the loaded C function pointer will be written to.</param>
/// <returns>
///  <para>Returns true if successfully loaded the specified callback.</para>
///  <para>Returns false if the given C function name was not found or if given invalid arguments.</para>
/// </returns>
static bool DirectXDecoder_LoadCallback(HMODULE moduleHandle, const char* functionName, void** callback)
{
	if (!moduleHandle || !functionName || !callback)
	{
		return false;
	}
	*callback = (void*)::GetProcAddress(moduleHandle, functionName);
	return (*callback) ? true : false;
}

/// <summary>
///  <para>
///   Attempts to dynamically load the DirectX Media Foundation libraries and C function callbacks needed by this
///   module's "MFDecoder" class.
///  </para>
///  <para>
///   Will set static member "sIsMediaFoundationAvailable" to true if the Media Foundation library
///   is available on the system.
///  </para>
///  <para>Expected to be called by this module's open() function.</para>
/// </summary>
static void DirectXDecoder_LoadMediaFoundationLibraries()
{
	static bool sWasLoadAttemptedBefore = false;
	
	// Synchronize thread access.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

	// Do not continue if this function was called once before.
	if (sWasLoadAttemptedBefore)
	{
		return;
	}
	sWasLoadAttemptedBefore = true;

	// Attempt to load all DirectX Media Foundation related libraries needed by this module.
	// If any of these return null, then the system we're running on does not have the DirectX version required.
	auto mfplatModuleHandle = ::LoadLibraryW(L"mfplat");
	auto mfreadwriteModuleHandle = ::LoadLibraryW(L"mfreadwrite");
	auto propsysModuleHandle = ::LoadLibraryW(L"propsys");
	if (!mfplatModuleHandle || !mfreadwriteModuleHandle || !propsysModuleHandle)
	{
		return;
	}

	// Attempt to load all of the C functions needed to use the Media Foundation decoders.
	if (!DirectXDecoder_LoadCallback(mfplatModuleHandle, "MFStartup", (void**)&sMFStartupCallback)) return;
	if (!DirectXDecoder_LoadCallback(mfplatModuleHandle, "MFShutdown", (void**)&sMFShutdownCallback)) return;
	if (!DirectXDecoder_LoadCallback(mfplatModuleHandle, "MFCreateAsyncResult", (void**)&sMFCreateAsyncResultCallback)) return;
	if (!DirectXDecoder_LoadCallback(mfplatModuleHandle, "MFCreateMediaType", (void**)&sMFCreateMediaTypeCallback)) return;
	if (!DirectXDecoder_LoadCallback(mfreadwriteModuleHandle, "MFCreateSourceReaderFromByteStream", (void**)&sMFCreateSourceReaderFromByteStreamCallback)) return;
	if (!DirectXDecoder_LoadCallback(propsysModuleHandle, "PropVariantToInt64", (void**)&sPropVariantToInt64Callback)) return;

	// Success! We have everything needed to use the DirectX Media Foundation for audio decoding.
	sIsMediaFoundationAvailable = true;
}

#ifdef WINDOWS_ACM_SUPPORTED
/// <summary>
///  <para>
///   Attempts to dynamically load Windows ACM related libraries ang C function callbacks needed by this
///   module's "AcmDecoder" class.
///  </para>
///  <para>Will set static member "sIsAcmAvailable" to true if ACM related libraries are available on the system.</para>
///  <para>Expected to be called by this module's open() function.</para>
/// </summary>
static void DirectXDecoder_LoadAcmLibraries()
{
	static bool sWasLoadAttemptedBefore = false;
	
	// Synchronize thread access.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

	// Do not continue if this function was called once before.
	if (sWasLoadAttemptedBefore)
	{
		return;
	}
	sWasLoadAttemptedBefore = true;

	// Attempt to load all ACM related libraries needed by this module.
	// If any of these return null, then we're likely running on a Windows N/NK edition.
	auto wmvcoreModuleHandle = ::LoadLibraryW(L"wmvcore");
	if (!wmvcoreModuleHandle)
	{
		return;
	}

	// Attempt to load all of the C functions needed to use the ACM decoders.
	if (!DirectXDecoder_LoadCallback(wmvcoreModuleHandle, "WMCreateSyncReader", (void**)&sWMCreateSyncReaderCallback)) return;

	// Success! We have everything needed to use Windows' ACM for audio decoding.
	sIsAcmAvailable = true;
}
#endif

#pragma endregion


#pragma region Public Functions
extern "C" static int DirectXDecoder_init()
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

	// Initialize COM for this thread, if not done already.
	auto currentThreadId = std::this_thread::get_id();
	if (sComInitializedThreadIdSet.find(currentThreadId) == sComInitializedThreadIdSet.end())
	{
		HRESULT result = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (SUCCEEDED(result))
		{
			sComInitializedThreadIdSet.insert(currentThreadId);
		}
	}

	// Initialize DirectX Media Foundation, if not done already.
	// This is global and does not need to be done per thread like COM initialization.
	// This also must be done after initializing COM up above.
	DirectXDecoder_LoadMediaFoundationLibraries();
	if (sIsMediaFoundationAvailable && (sModuleInitializationCount < 1))
	{
		HRESULT result = sMFStartupCallback(MF_VERSION, MFSTARTUP_FULL);
		if (SUCCEEDED(result))
		{
			sWasMediaFoundationInitialized = true;
		}
	}

	// Load ACM related libraries.
#ifdef WINDOWS_ACM_SUPPORTED
	DirectXDecoder_LoadAcmLibraries();
#endif

	// Track the number of times this initialization function was called.
	sModuleInitializationCount++;

	// Return a success result.
	return 1;
}

extern "C" static void DirectXDecoder_quit()
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

	// Decrement
	if (sModuleInitializationCount > 0)
	{
		sModuleInitializationCount--;
	}

	// Shutdown the DirectX Media Foundation library if this module no longer needs it.
	if (sWasMediaFoundationInitialized && (sModuleInitializationCount <= 0))
	{
		sMFShutdownCallback();
	}

	// Uninitialize COM for this thread, but only if this module has successfully initialized it before.
	auto currentThreadId = std::this_thread::get_id();
	auto iter = sComInitializedThreadIdSet.find(currentThreadId);
	if (iter != sComInitializedThreadIdSet.end())
	{
		::CoUninitialize();
		sComInitializedThreadIdSet.erase(iter);
	}
}

extern "C" static int DirectXDecoder_open(Sound_Sample *samplePointer, const char *fileExtension)
{
	// Validate arguments.
	if (!samplePointer || !fileExtension || ('\0' == fileExtension[0]))
	{
		return 0;
	}

	// Fetch the internal sample struct.
	auto sampleInternalPointer = (Sound_SampleInternal*)samplePointer->opaque;
	if (!sampleInternalPointer)
	{
		return 0;
	}

	// Fetch a Windows audio decoder that can decode the given file.
	BaseDecoder* decoderPointer = nullptr;
	if (sWasMediaFoundationInitialized)
	{
		// The DirectX Media Foundation is available on the system.
		// Try it first since it supports the most audio formats.
		decoderPointer = new MFDecoder(samplePointer);
		if (decoderPointer->CanDecode() == false)
		{
			delete decoderPointer;
			decoderPointer = nullptr;
		}
	}
#ifdef WINDOWS_ACM_SUPPORTED
	if (!decoderPointer && sIsAcmAvailable)
	{
		// Fallback to using Microsoft's ACM audio decoders.
		// Note: This API is not available for Windows Universal or Windows Phone apps.
		//       Also not available to Win32 apps on the N/NK editions of Windows without Media Player installed.
		decoderPointer = new AcmDecoder(samplePointer);
		if (decoderPointer->CanDecode() == false)
		{
			delete decoderPointer;
			decoderPointer = nullptr;
		}
	}
#endif

	// Do not continue if this module is unable to decode the given audio file.
	if (!decoderPointer)
	{
		return 0;
	}

	// Update the given ALmixer sample with the decoder's information.
	if (decoderPointer->CanSeek())
	{
		samplePointer->flags = SOUND_SAMPLEFLAG_CANSEEK;
	}
	samplePointer->actual = decoderPointer->GetOutputInfo();
	sampleInternalPointer->decoder_private = decoderPointer;
	sampleInternalPointer->total_time = decoderPointer->GetDurationInMiliseconds();

	// Return a success result.
	return 1;
}

extern "C" static void DirectXDecoder_close(Sound_Sample *samplePointer)
{
	// Validate argument.
	if (!samplePointer)
	{
		return;
	}

	// Fetch the internal sample struct.
	auto sampleInternalPointer = (Sound_SampleInternal*)samplePointer->opaque;
	if (!sampleInternalPointer)
	{
		return;
	}

	// Delete the decoder attached to the given sample object.
	auto decoderPointer = (BaseDecoder*)sampleInternalPointer->decoder_private;
	if (decoderPointer)
	{
		delete decoderPointer;
		sampleInternalPointer->decoder_private = nullptr;
	}
}

extern "C" static size_t DirectXDecoder_read(Sound_Sample *samplePointer)
{
	if (samplePointer && samplePointer->opaque)
	{
		auto decoderPointer = (BaseDecoder*)((Sound_SampleInternal*)samplePointer->opaque)->decoder_private;
		if (decoderPointer)
		{
			return decoderPointer->ReadUsing(samplePointer);
		}
	}
	return 0;
}

extern "C" static int DirectXDecoder_seek(Sound_Sample *samplePointer, size_t ms)
{
	// Validate.
	if (!samplePointer)
	{
		return 0;
	}

	// Fetch the internal sample struct and validate it.
	auto sampleInternalPointer = (Sound_SampleInternal*)samplePointer->opaque;
	if (!sampleInternalPointer || !(sampleInternalPointer->decoder_private) || !(sampleInternalPointer->rw))
	{
		return 0;
	}

	// Seek to the given point in time in milliseconds.
	auto decoderPointer = (BaseDecoder*)sampleInternalPointer->decoder_private;
	bool didSeek = decoderPointer->Seek(samplePointer, ms);
	return didSeek ? 1 : 0;
}

extern "C" static int DirectXDecoder_rewind(Sound_Sample *samplePointer)
{
	// Seek to the beginning of the audio file.
	return DirectXDecoder_seek(samplePointer, 0);
}

#pragma endregion


#pragma region Public Callbacks
extern "C" const char *DirectXDecoderExtensions[] =
{
	"3gp",		// Supported by: Media Foundation
	"aac",		// Supported by: Media Foundation
#ifndef SOUND_SUPPORTS_MP3
	"mp3",		// Supported by: Media Foundation and ACM
#endif
#ifndef SOUND_SUPPORTS_WAV
	"wav",		// Supported by: Media Foundation
#endif
	nullptr
};

extern "C" const Sound_DecoderFunctions __Sound_DecoderFunctions_DirectX =
{
	{
		DirectXDecoderExtensions,
		"Decode audio through DirectX or ACM",
		"Corona Labs Inc.<support@coronalabs.com>",
		"http://www.coronalabs.com"
	},
	DirectXDecoder_init,
	DirectXDecoder_quit,
	DirectXDecoder_open,
	DirectXDecoder_close,
	DirectXDecoder_read,
	DirectXDecoder_rewind,
	DirectXDecoder_seek
};

#pragma endregion

#endif
