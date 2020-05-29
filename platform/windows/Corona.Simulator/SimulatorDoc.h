//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoronaProject.h"
#include <memory>

class CSimulatorDoc : public CDocument
{
protected: // create from serialization only
	CSimulatorDoc();
	DECLARE_DYNCREATE(CSimulatorDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CSimulatorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CString GetPath()
	{ 
		return mProjectPointer->GetPath();
	}

	std::shared_ptr<CCoronaProject> GetProject()
	{
		return mProjectPointer;
	}

protected:
    std::shared_ptr<CCoronaProject> mProjectPointer;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void SetTitle(LPCTSTR lpszTitle);
};
