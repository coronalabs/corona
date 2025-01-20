//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_MacSimulatorServices__
#define __Rtt_MacSimulatorServices__

#include "Rtt_MRuntimeDelegate.h"
#include "Rtt_MSimulatorServices.h"
#include "Rtt_LuaResource.h"

// ----------------------------------------------------------------------------

@class AppDelegate;
@class NewProjectController;
@class CoronaWindowController;
@class NSString;



namespace Rtt
{

struct RecentProjectInfo;
// ----------------------------------------------------------------------------

class MacSimulatorServices : public MSimulatorServices
{
	public:
		MacSimulatorServices( AppDelegate *owner, CoronaWindowController *viewController, NSString *resourcePath );
		~MacSimulatorServices();

	// MSimulatorServices
	public:
		virtual bool NewProject() const;
		virtual bool OpenProject( const char *name ) const;
		virtual bool BuildProject( const char *platform ) const;
        virtual void ShowSampleCode() const;
        virtual void SelectOpenFilename(const char* currDirectory, const char* extn, LuaResource* resource) const;
        virtual void SelectSaveFilename(const char* newFilename, const char* currDirectory, const char* extn, LuaResource* resource) const;
        virtual const char* GetCurrProjectPath( ) const;
        virtual void RunExtension(const char *extName) const;
        virtual void SetProjectResourceDirectory(const char *projectResourceDirectory);
        virtual void SetWindowCloseListener(LuaResource* resource) const;
        virtual void CloseWindow( ) const;
        virtual const char* GetPreference(const char *prefName) const;
        virtual void SetPreference(const char *prefName, const char *prefValue) const;
        virtual void SetDocumentEdited(bool value) const;
        virtual bool GetDocumentEdited( ) const;
		virtual void GetRecentDocs(LightPtrArray<RecentProjectInfo> *list) const;
        virtual void SetCursorRect(const char *cursorName, int x, int y, int w, int h) const;
        virtual void SetWindowResizeListener(LuaResource* resource) const;
        virtual void SetWindowTitle(const char *windowTitle) const;
        virtual void OpenTextEditor(const char *filename) const;
        virtual void OpenColorPanel(double r, double g, double b, double a, LuaResource* callback) const;
		virtual void SetBuildMessage(const char *message) const;
		virtual void SendAnalytics(const char *eventName, const char *keyName, const char *value) const;
		virtual bool RelaunchProject( ) const;
		virtual bool EditProject( const char *name ) const;
		virtual bool ShowProjectFiles( const char *name ) const;
		virtual bool ShowProjectSandbox( const char *name ) const;

	private:
		AppDelegate *fOwner;
        CoronaWindowController *fWindowController;
        NSString *fResourcePath;
#ifdef Rtt_INCLUDE_COLOR_PANEL // color panel is no longer used and is problematic on some macOS versions
        NSColorPanel *fColorPanel;
#endif
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_MacSimulatorServices__
