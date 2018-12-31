// ----------------------------------------------------------------------------
// 
// CancelEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "CancelEventArgs.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
CancelEventArgs::CancelEventArgs()
:	fWasCanceled(false)
{
}

#pragma endregion


#pragma region Public Properties
bool CancelEventArgs::Cancel::get()
{
	return fWasCanceled;
}

void CancelEventArgs::Cancel::set(bool value)
{
	fWasCanceled = value;
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT
