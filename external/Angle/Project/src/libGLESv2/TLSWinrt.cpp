//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TLSWinrt.cpp: Thread local storage implementation for WinRT and WP8.

#include "precompiled.h"
#include "TLSWinrt.h"

#if defined(WINAPI_FAMILY)
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

__declspec( thread ) DWORD currentTLS = TLS_OUT_OF_INDEXES;
__declspec( thread ) gl::Current glContext;

void* TlsGetValue(DWORD index) { return &glContext; };
void * LocalAlloc(UINT uFlags, size_t size) { return (void*) &glContext; };
void LocalFree(HLOCAL index) {glContext.context = NULL; glContext.display = NULL;};
DWORD TlsAlloc() { return 1; };
void TlsSetValue(DWORD currentTLS, gl::Current* current) {};
void TlsFree(DWORD index) {currentTLS = TLS_OUT_OF_INDEXES;};
#endif
#endif