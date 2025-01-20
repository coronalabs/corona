//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


/// <summary>
///  <para>Custom document template used to handle Corona "main.lua" files in the Corona Simulator.</para>
///  <para>
///   This class is needed to load a Corona project by its root directory since MFC by default only supports opening files.
///  </para>
/// </summary>
class CSimulatorDocTemplate : public CSingleDocTemplate
{
	DECLARE_DYNAMIC(CSimulatorDocTemplate)

	public:
		CSimulatorDocTemplate();
		CSimulatorDocTemplate(
				UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
		virtual ~CSimulatorDocTemplate();

		virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE) override;
		virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bAddToMRU, BOOL bMakeVisible) override;

	private:
		bool UpdateFilePath(LPCTSTR filePath, CString& updatedFilePath) const;
};
