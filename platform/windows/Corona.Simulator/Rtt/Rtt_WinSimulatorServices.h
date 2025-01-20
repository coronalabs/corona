//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_MSimulatorServices.h"
#include "Rtt_LuaResource.h"


#pragma region Forward Declarations
class CSimulatorView;

#pragma endregion


namespace Rtt
{

/// <summary>
///  <para>
///   Provides access to the simulator window's project services such as "New Project", "Open Project",
///   "Show Sample Code", etc.
///  </para>
///  <para>These services can be registered into Lua which is used by the "Home Screen" Corona project.</para>
/// </summary>
class WinSimulatorServices : public MSimulatorServices
{
	public:
		/// <summary>Creates a new Simulator Services object for the given view.</summary>
		/// <param name="simulatorViewPointer">
		///  Reference to the view that this object will use to create new projects, open projects, and do builds for.
		/// </param>
		WinSimulatorServices(CSimulatorView& simulatorView);

		/// <summary>Destroys this object and its owned objects.</summary>
		virtual ~WinSimulatorServices();

		/// <summary>Displays a "New Project" window for creating a new Corona project.</summary>
		virtual bool NewProject() const;

		/// <summary>Opens a Corona project for simulation.</summary>
		/// <param name="name">
		///  <para>Set to the absolute path to the Corona project directory to open and simulate.</para>
		///  <para>Set to null or empty string to display the "Open File" dialog for selecting a project.</para>
		/// </param>
		virtual bool OpenProject(const char *name) const;

		/// <summary>Display the "Build" window for the currently open Corona project.</summary>
		/// <param name="platformName">The name of the platform to build for such as iOS, Android, etc.</param>
		virtual bool BuildProject(const char *platformName) const;

		/// <summary>Displays an "Open File" dialog which default to the sample projects directory.</summary>
		virtual void ShowSampleCode() const;

		virtual void SelectOpenFilename(const char* currDirectory, const char* extn, LuaResource* resource) const;
		virtual void SelectSaveFilename(const char* newFilename, const char* currDirectory, const char* extn, LuaResource* resource) const;
		virtual const char* GetCurrProjectPath( ) const;
		virtual void RunExtension(const char *extName) const;
		virtual void SetProjectResourceDirectory(const char *projectResourceDirectory);
		virtual void SetWindowCloseListener(LuaResource* resource) const;
		virtual void CloseWindow( ) const;
		virtual void GetRecentDocs(LightPtrArray<RecentProjectInfo> *list) const;
		virtual const char* GetPreference(const char *prefName) const;
		virtual void SetPreference(const char *prefName, const char *prefValue) const;
        virtual void SetDocumentEdited(bool value) const;
        virtual bool GetDocumentEdited( ) const;
        virtual void SetCursorRect(const char *cursorName, int x, int y, int width, int height) const;
        virtual void SetWindowResizeListener(LuaResource* resource) const;
		virtual void SetWindowTitle(const char *title) const;
		virtual void OpenTextEditor(const char *filename) const;
        virtual void OpenColorPanel(double r, double g, double b, double a, LuaResource* callback) const;
		virtual void SetBuildMessage(const char *message) const;
		virtual void SendAnalytics(const char *eventName, const char *keyName, const char *value) const;
		virtual bool RelaunchProject() const;
		virtual bool EditProject(const char *name) const;
		virtual bool ShowProjectFiles(const char *name) const;
		virtual bool ShowProjectSandbox(const char *name) const;

	private:
		/// <summary>
		///  Reference to the simulator application's view that will will implement this class' services
		///  such as "New Project", "Open Project", etc.
		/// </summary>
		CSimulatorView& fSimulatorView;
};

} // namespace Rtt
