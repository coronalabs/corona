//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_MSimulatorServices__
#define __Rtt_MSimulatorServices__

#define Rtt_DEMO_PROJECT_NAME "demo"

// ----------------------------------------------------------------------------
#include "Rtt_Array.h"
#include "Rtt_LuaResource.h"

namespace Rtt
{
// ----------------------------------------------------------------------------

struct RecentProjectInfo;

class MSimulatorServices
{
	public:
		virtual bool NewProject() const = 0;
		virtual bool OpenProject( const char *name ) const = 0;
		virtual bool BuildProject( const char *platform ) const = 0;
		virtual void ShowSampleCode() const = 0;
        virtual void SelectOpenFilename(const char* currDirectory, const char* extn, LuaResource* resource) const = 0;
        virtual void SelectSaveFilename(const char* newFilename, const char* currDirectory, const char* extn, LuaResource* resource) const = 0;
        virtual const char* GetCurrProjectPath( ) const = 0;
        virtual void RunExtension(const char *extName) const = 0;
        virtual void SetProjectResourceDirectory(const char *projectResourceDirectory) = 0;
		virtual void GetRecentDocs(LightPtrArray<RecentProjectInfo> *list) const = 0;
        virtual void SetWindowCloseListener(LuaResource* resource) const = 0;
        virtual void CloseWindow( ) const = 0;
        virtual const char* GetPreference(const char *prefName) const = 0;
        virtual void SetPreference(const char *prefName, const char *prefValue) const = 0;
        virtual void SetDocumentEdited(bool value) const = 0;
        virtual bool GetDocumentEdited( ) const = 0;
        virtual void SetCursorRect(const char *cursorName, int x, int y, int w, int h) const = 0;
        virtual void SetWindowResizeListener(LuaResource* resource) const = 0;
        virtual void SetWindowTitle(const char *windowTitle) const = 0;
        virtual void OpenTextEditor(const char *filename) const = 0;
        virtual void OpenColorPanel(double r, double g, double b, double a, LuaResource* callback) const = 0;
		virtual void SetBuildMessage(const char *buildMessage) const = 0;
		virtual void SendAnalytics(const char *eventName, const char *keyName, const char *value) const = 0;
		virtual bool RelaunchProject( ) const = 0;
		virtual bool EditProject( const char *name ) const = 0;
		virtual bool ShowProjectFiles( const char *name ) const = 0;
		virtual bool ShowProjectSandbox( const char *name ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_MSimulatorServices__
