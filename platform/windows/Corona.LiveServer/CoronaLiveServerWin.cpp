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

#include "stdafx.h"
#include "CoronaLiveServerWin.h"
#include <dns_sd.h>
#include "..\..\shared\CoronaLiveServer\CoronaLiveServerCore.h"

#pragma comment(lib, "dnssd.lib")

#define WM_UPDATE_NOTIFICATION (WM_USER+31)

class FSWatcher
{
public:
	static LRESULT CALLBACK OnProcessMessage(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam)
	{
		if (messageId == WM_UPDATE_NOTIFICATION)
		{
			return ((FSWatcher*)lParam)->SendUpdate();
		}
		else
		{
			return ::DefWindowProc(windowHandle, messageId, wParam, lParam);
		}
	}
private:
	static DWORD WINAPI Watcher(_In_ LPVOID lpParameter)
	{
		FSWatcher * self = (FSWatcher*)lpParameter;
		HWND hWnd = self->m_hWnd;
		HANDLE changeHandle = self->mHandle;
		HANDLE handles[2] = { self->mTerminator, changeHandle };
		DWORD status;
		while (TRUE)
		{
			status = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			switch (status) 
			{
			case WAIT_OBJECT_0:
				return 0;
			case WAIT_OBJECT_0 + 1:
				do {
					FindNextChangeNotification(changeHandle);
				} while (WaitForSingleObject(changeHandle, CoronaLiveServerWin::sLiveServerUpdateDelay) != WAIT_TIMEOUT);
				PostMessage(hWnd, WM_UPDATE_NOTIFICATION, 0, (LPARAM)self);
				break;
			default:
				ASSERT(0);
			}
		};
		
		return 0;
	}
	HANDLE mHandle, mThread, mTerminator;
	HWND m_hWnd;
	Rtt_LiveServer *mServer;

	LONG SendUpdate()
	{
		mServer->Update(mPath);
		return 0;
	}

public:
	std::wstring mPath;

	FSWatcher(const TCHAR* path, HWND hWnd, Rtt_LiveServer *server)
		: mTerminator(INVALID_HANDLE_VALUE)
		, m_hWnd(hWnd)
		, mServer(server)
		, mPath(path)
		, mThread(INVALID_HANDLE_VALUE)
		, mHandle(INVALID_HANDLE_VALUE)
	{
		mHandle = FindFirstChangeNotification(path, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
		if (mHandle != INVALID_HANDLE_VALUE)
		{
			mTerminator = CreateEvent(0, TRUE, FALSE, NULL);
			mThread = CreateThread(NULL, 0, Watcher, this, 0, 0);
		}
	}
	~FSWatcher()
	{
		SetEvent(mTerminator);
		if (mThread != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(mThread, INFINITE);
			CloseHandle(mThread);
			CloseHandle(mTerminator);
		}
		if (mHandle != INVALID_HANDLE_VALUE)
		{	
			FindCloseChangeNotification(mHandle);
		}
	}
};

CoronaLiveServerWin::CoronaLiveServerWin()
: mAnnounceHandle(nullptr)
, mServer(nullptr)
, m_hWnd(NULL)
{
}


CoronaLiveServerWin::~CoronaLiveServerWin()
{
	mFSWatchers.clear();
	if (mAnnounceHandle)
	{
		DNSServiceRefDeallocate((DNSServiceRef)mAnnounceHandle);
		mAnnounceHandle = nullptr;
	}
	delete mServer;
	mServer = nullptr;

	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		::UnregisterClassW(L"LiveServerWatcher", 0);
	}
}


int CoronaLiveServerWin::Add(const TCHAR* path)
{
	int err = 0;
	if (mServer == nullptr)
	{
		mServer = new Rtt_LiveServer();
	}
	int port = mServer->Add(path);
	if (port > 0)
	{
		if (mAnnounceHandle == nullptr)
		{
			DNSServiceRef ref = 0;
			DNSServiceRegister(&ref, 0, 0, "", "_corona_live._tcp", NULL, NULL, htons(port), 0, NULL, NULL, NULL);
			if (ref)
			{
				DNSServiceProcessResult(ref);
				mAnnounceHandle = ref;
			}
			else
			{
				mServer->Remove(path);
				err = 10;
				return err;
			}
		}

		if (m_hWnd == NULL)
		{
			static const wchar_t *sClassName = nullptr;
			if (!sClassName)
			{
				HMODULE moduleHandle = nullptr;
				DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
				::GetModuleHandleExW(flags, (LPCWSTR)sClassName, &moduleHandle);

				WNDCLASSEXW settings{};
				settings.cbSize = sizeof(settings);
				settings.lpszClassName = L"LiveServerWatcher";
				settings.hInstance = moduleHandle;
				settings.lpfnWndProc = &FSWatcher::OnProcessMessage;
				if (::RegisterClassExW(&settings))
				{
					sClassName = settings.lpszClassName;
					m_hWnd = ::CreateWindowEx(0, sClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, this);
					if (m_hWnd)
					{
						SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
					}
				}
			}
		}

		if (m_hWnd != NULL)
		{
			auto ptr = std::make_shared<FSWatcher>(path, m_hWnd, mServer);
			mFSWatchers[ptr.get()] = ptr;
		}
	}
	else
	{
		err = -port;
	}
	return err;
}

void CoronaLiveServerWin::Stop(const TCHAR* path)
{
	if (mServer != nullptr)
	{
		std::wstring path(path);
		mServer->Remove(path);
		for (auto it = mFSWatchers.begin(); it != mFSWatchers.end();) {
			if (it->first->mPath == path) {
				it = mFSWatchers.erase(it);
			}
			else
				it++;
		}
	}
}
